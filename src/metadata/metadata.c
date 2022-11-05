#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <htslib/kstring.h>
#include <htslib/khash_str2int.h>
#include <err.h>
#include <sysexits.h>

#define COLUMN_NAME_MAX_LENGTH 32
#define GIGGLE_METADATA_FILE_MARKER_LENGTH 7
#define GIGGLE_METADATA_FILE_MARKER "GIGLMET"
#define GIGGLE_METADATA_VERSION_MARKER_LENGTH 3
#define GIGGLE_METADATA_VERSION_MARKER "000"
#define GIGGLE_METADATA_EXTRA_LENGTH 6

void check_file_read(char *file_name, FILE *fp, size_t exp, size_t obs)
{
    if (exp != obs) {
        if (feof(fp))
            errx(EX_IOERR,
                 "Error reading file \"%s\": End of file",
                 file_name);
        err(EX_IOERR, "Error reading file \"%s\"", file_name);
    }
}

/*
input
<column number> <column name> <data type> <optional, string length>

output
<7-byte GIGLMET> <3-byte 000> <6-byte extra>
<1-byte uint8 num_cols> <1-byte uint8 col_width>
< array of  
  <256-byte char*, name>
  <1-byte uint8, width>
  <1-byte char, data type specifier>
>
<8-byte uint64 num_rows>
< array of  
<data 1> <data 2> ... <data n>
>
*/

enum data_type {
  CHAR,   // c = char
  INT_8,  // b = byte
  INT_16, // h = short
  INT_32, // i = int
  INT_64, // l = long
  FLOAT,  // f = float
  DOUBLE, // d = double
  STRING  // s = string
};

struct metadata_type {
  enum data_type data_type;
  uint8_t width; // max data string length = 255
  // max name length = COLUMN_NAME_MAX_LENGTH - 1 (null character)
  char name[COLUMN_NAME_MAX_LENGTH]; 
};

struct metadata_column {
  uint8_t column; // column number
  struct metadata_type *type;
};

struct metadata_columns {
  uint8_t num_cols; // max item count = 255
  uint16_t row_width; // total width of each data row
  struct metadata_column **columns;
};

struct metadata_types {
  uint8_t num_cols;
  uint16_t row_width; // total width of each data row
  uint64_t num_rows;
  uint64_t header_offset; // total header offset, end of the header file position
  void *column_name_to_index; // khash_str2int hashmap to map column names to column indexes
  uint16_t *col_offsets; // offset of ith column in a data row
  struct metadata_type **types;
};

union metadata_data {
  char c;
  int8_t b;
  int16_t h;
  int32_t i;
  int64_t l;
  float f;
  double d;
  char *s;
};

struct metadata_item {
  struct metadata_type *type;
  union metadata_data data;
};

struct metadata_row {
  uint8_t num; // total number of columns
  struct metadata_item **items; // array of rows
};

struct metadata_rows {
  uint64_t num; // total number of rows
  struct metadata_row **rows; // data rows
};

enum comparison {
  EQUAL,
  NOT_EQUAL,
  LESS,
  GREATER,
  LESS_EQUAL,
  GREATER_EQUAL
};

struct query_filter {
  struct metadata_type *type;
  uint8_t column_id;
  enum comparison comparison;
  union metadata_data data;
};

enum data_type type_string_to_enum(char type_string[8]) {
  enum data_type type;
  for(int i = 0; type_string[i]; ++i){
    type_string[i] = toupper(type_string[i]);
  }
  if (strcmp(type_string, "CHAR") == 0) type = CHAR;
  else if (strcmp(type_string, "INT_8") == 0) type = INT_8;
  else if (strcmp(type_string, "INT_16") == 0) type = INT_16;
  else if (strcmp(type_string, "INT_32") == 0) type = INT_32;
  else if (strcmp(type_string, "INT_64") == 0) type = INT_64;
  else if (strcmp(type_string, "FLOAT") == 0) type = FLOAT;
  else if (strcmp(type_string, "DOUBLE") == 0) type = DOUBLE;
  else if (strcmp(type_string, "STRING") == 0) type = STRING;
  else {
    err(1, "Invalid data type %s.\n", type_string);
  }
  return type;
}

uint8_t data_type_to_width(enum data_type type) {
  uint8_t width = 0;
  switch (type) {
    case CHAR: 
      width = sizeof(char);
      break;
    case INT_8: 
      width = sizeof(int8_t);
      break;
    case INT_16: 
      width = sizeof(int16_t);
      break;
    case INT_32: 
      width = sizeof(int32_t);
      break;
    case INT_64: 
      width = sizeof(int64_t);
      break;
    case FLOAT: 
      width = sizeof(float);
      break;
    case DOUBLE: 
      width = sizeof(double);
      break;
    case STRING: 
      break;
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
  return width;
}

char data_type_to_char(enum data_type type) {
  char type_char = 0;
  switch (type) {
    case CHAR: 
      type_char = 'c';
      break;
    case INT_8: 
      type_char = 'b';
      break;
    case INT_16: 
      type_char = 'h';
      break;
    case INT_32: 
      type_char = 'i';
      break;
    case INT_64: 
      type_char = 'l';
      break;
    case FLOAT: 
      type_char = 'f';
      break;
    case DOUBLE: 
      type_char = 'd';
      break;
    case STRING: 
      type_char = 's';
      break;
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
  return type_char;
}

enum data_type type_char_to_enum(char type_char) {
  enum data_type type;
  switch (type_char) {
    case 'c': 
      type = CHAR;
      break;
    case 'b': 
      type = INT_8;
      break;
    case 'h': 
      type = INT_16;
      break;
    case 'i': 
      type = INT_32;
      break;
    case 'l': 
      type = INT_64;
      break;
    case 'f': 
      type = FLOAT;
      break;
    case 'd': 
      type = DOUBLE;
      break;
    case 's': 
      type = STRING;
      break;
    default:
      err(1, "Unknown type_char %c.\n", type_char);
  }
  return type;
}

// this function is not responsible for
// freeing the allocated memory for the string
char *safe_sscanf(uint8_t str_width, char *data) {
  char s_format[6]; // '%' + max value for int8_t + 's' + NULL -> '%255sN'
  char *s;
  snprintf(s_format, sizeof(s_format), "%%%ds", str_width - 1);
  s = (char *)calloc(str_width, sizeof(char));
  if (s == NULL) {
    err(1, "calloc failure for s in fwrite_data_type_item.\n");
  }
  sscanf(data, s_format, s);
  return s;
}

void fwrite_data_type_item(FILE *metadata_index, struct metadata_type *metadata_type, char *data) {
  enum data_type type = metadata_type->data_type;
  char c;
  int8_t b;
  int16_t h;
  int32_t i;
  int64_t l;
  float f;
  double d;
  char *s;
  uint8_t str_width;

  switch (type) {
    case CHAR: 
      c = *data;
      if (fwrite(&c, sizeof(char), 1, metadata_index) != 1) {
        err(1, "fwrite failure for CHAR in fwrite_data_type_item.\n");
      }
      break;

    case INT_8: 
      b = atoi(data);
      if (fwrite(&b, sizeof(int8_t), 1, metadata_index) != 1) {
        err(1, "fwrite failure for INT_8 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_16: 
      h = atoi(data);
      if (fwrite(&h, sizeof(int16_t), 1, metadata_index) != 1) {
        err(1, "fwrite failure for INT_16 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_32: 
      i = atol(data);
      if (fwrite(&i, sizeof(int32_t), 1, metadata_index) != 1) {
        err(1, "fwrite failure for INT_32 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_64: 
      l = atoll(data);
      if (fwrite(&l, sizeof(int64_t), 1, metadata_index) != 1) {
        err(1, "fwrite failure for INT_64 in fwrite_data_type_item.\n");
      }
      break;
      
    case FLOAT: 
      f = atof(data);
      if (fwrite(&f, sizeof(float), 1, metadata_index) != 1) {
        err(1, "fwrite failure for FLOAT in fwrite_data_type_item.\n");
      }
      break;
      
    case DOUBLE: 
      d = atof(data);
      if (fwrite(&d, sizeof(double), 1, metadata_index) != 1) {
        err(1, "fwrite failure for DOUBLE in fwrite_data_type_item.\n");
      }
      break;
      
    case STRING: 
      str_width = metadata_type->width;
      s = safe_sscanf(str_width, data);
      if (fwrite(s, sizeof(char), str_width, metadata_index) != str_width) {
        err(1, "fwrite failure for STRING in fwrite_data_type_item.\n");
      }
      free(s);
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
}

void fread_data_type_item(char *metadata_index_filename, FILE *metadata_index, struct metadata_item *metadata_item) {
  struct metadata_type *metadata_type = metadata_item->type;
  enum data_type type = metadata_type->data_type;
  uint8_t str_width;
  char *s;
  size_t fr;

  switch (type) {
    case CHAR: 
      fr = fread(&(metadata_item->data.c), sizeof(char), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;

    case INT_8: 
      fr = fread(&(metadata_item->data.b), sizeof(int8_t), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case INT_16: 
      fr = fread(&(metadata_item->data.h), sizeof(int16_t), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case INT_32: 
      fr = fread(&(metadata_item->data.i), sizeof(int32_t), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case INT_64: 
      fr = fread(&(metadata_item->data.l), sizeof(int64_t), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case FLOAT: 
      fr = fread(&(metadata_item->data.f), sizeof(float), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case DOUBLE: 
      fr = fread(&(metadata_item->data.d), sizeof(double), 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);
      break;
      
    case STRING: 
      str_width = metadata_type->width;
      s = (char *)malloc(str_width * sizeof(char));
      if (s == NULL) {
        err(1, "malloc failure for s in fwrite_data_type_item.\n");
      }
      fr = fread(s, sizeof(char), str_width, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, str_width, fr);
      metadata_item->data.s = s;
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
}

union metadata_data parse_query_filter_data_string(struct metadata_type *metadata_type, char *data) {
  union metadata_data metadata_data;
  enum data_type type = metadata_type->data_type;
  char *s;

  switch (type) {
    case CHAR: 
      metadata_data.c = *data;
      break;

    case INT_8: 
      metadata_data.b = atoi(data);
      break;
      
    case INT_16: 
      metadata_data.h = atoi(data);
      break;
      
    case INT_32: 
      metadata_data.i = atol(data);
      break;
      
    case INT_64: 
      metadata_data.l = atoll(data);
      break;
      
    case FLOAT: 
      metadata_data.f = atof(data);
      break;
      
    case DOUBLE: 
      metadata_data.d = atof(data);
      break;
      
    case STRING: 
      metadata_data.s = safe_sscanf(metadata_type->width, data);
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }

  return metadata_data;
}

enum comparison comparison_string_to_enum(char *query_filter_string, int *start_comparison, int *end_comparison) {
  int i, start = -1, end;
  for(i = 0; query_filter_string[i]; ++i) {
    if (!(isalpha(query_filter_string[i]) || query_filter_string[i] == '_')) { // column name ended
      start = end = i;
      // TODO: if the first character of the string data is 
      // allowed to be =, escape logic needs to be implemented 
      if (query_filter_string[i+1] == '=') {
        end = i+1;
      }
      break;
    }
  }

  if (start == -1) {
    err(1, "Comparison operator not found.\n");
  }

  enum comparison comparison;
  if (start == end) {
    switch (query_filter_string[start]) {
      case '<': 
        comparison = LESS;
        break;
      case '>': 
        comparison = GREATER;
        break;
      default:
        err(1, "Unknown comparison operator %c.\n", query_filter_string[start]);
    }
  } else {
    switch (query_filter_string[start]) {
      case '=': 
        comparison = EQUAL;
        break;
      case '!': 
        comparison = NOT_EQUAL;
        break;
      case '<': 
        comparison = LESS_EQUAL;
        break;
      case '>': 
        comparison = GREATER_EQUAL;
        break;
      default:
        err(1, "Unknown comparison operator %c=.\n", query_filter_string[start]);
    }
  }

  *start_comparison = start;
  *end_comparison = end;
  return comparison;
}

struct metadata_columns *read_metadata_conf(char *metadata_conf_filename) {
  FILE *metadata_conf = fopen(metadata_conf_filename, "r");
  if (metadata_conf == NULL) {
    err(1, "%s not found.\n", metadata_conf_filename);
  }

  struct metadata_columns *metadata_columns = (struct metadata_columns *)malloc(sizeof(struct metadata_columns));
  if (metadata_columns == NULL) {
    err(1, "malloc failure for metadata_columns in read_metadata_conf.\n");
  }
  metadata_columns->num_cols = 0;
  metadata_columns->row_width = 0;

  metadata_columns->columns = (struct metadata_column **)malloc(255 * sizeof(struct metadata_column*));
  if (metadata_columns->columns == NULL) {
    err(1, "malloc failure for metadata_columns->columns in read_metadata_conf.\n");
  }

  void *column_set = khash_str2int_init();

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, metadata_conf)) != -1) {
    // printf("Retrieved line of length %zu:\n%s\n", read, line);

    uint8_t column;
    char name[256];
    char type_string[8];
    uint8_t str_len = 0;
    sscanf(line, "%hhu %255s %7s %hhu", &column, name, type_string, &str_len);
    // printf("%d %s %s %d\n", column, name, type_string, str_len);

    if (metadata_columns->num_cols == 255) {
      err(1, "Cannot store more than 255 columns.\n");
    }
    
    if (str_len == 255) {
      err(1, "Column '%s': string length cannot be more than 254.\n", name);
    }

    if (khash_str2int_has_key(column_set, name)) {
      err(1, "Cannot allow duplicate column '%s'.\n", name);
    } else {
      khash_str2int_set(column_set, strdup(name), 1);
    }

    struct metadata_type *metadata_type = (struct metadata_type *)calloc(1, sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      err(1, "calloc failure for metadata_type in read_metadata_conf.\n");
    }

    strncpy(metadata_type->name, name, strlen(name) + 1);
    metadata_type->data_type = type_string_to_enum(type_string);
    metadata_type->width += data_type_to_width(metadata_type->data_type);
    if (metadata_type->data_type == STRING) {
      metadata_type->width = str_len + 1;
    }
    metadata_columns->row_width += metadata_type->width;

    struct metadata_column *metadata_column = (struct metadata_column *)malloc(sizeof(struct metadata_column));
    if (metadata_column == NULL) {
      err(1, "malloc failure for metadata_column in read_metadata_conf.\n");
    }

    metadata_column->type = metadata_type;
    metadata_column->column = column;
    
    metadata_columns->columns[metadata_columns->num_cols++] = metadata_column;

  }

  if (metadata_columns->num_cols < 255) {
    metadata_columns->columns = (struct metadata_column **)realloc(metadata_columns->columns, sizeof(struct metadata_column*) * metadata_columns->num_cols);
    if (metadata_columns->columns == NULL) {
      err(1, "realloc failure for metadata_columns->columns in read_metadata_conf.\n");
    }
  }

  khash_str2int_destroy_free(column_set);

  if (line)
    free(line);

  fclose(metadata_conf);
  return metadata_columns;
}

void free_metadata_columns(struct metadata_columns *metadata_columns) {
  int i;
  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    free(metadata_type);
    free(metadata_column);
  }
  free(metadata_columns->columns);
  free(metadata_columns);
}

void free_metadata_types(struct metadata_types *metadata_types) {
  int i;
  for (i = 0; i < metadata_types->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_types->types[i];
    free(metadata_type);
  }
  free(metadata_types->col_offsets);
  free(metadata_types->types);
  khash_str2int_destroy_free(metadata_types->column_name_to_index);
  free(metadata_types);
}

void free_metadata_item(struct metadata_item *metadata_item) {
  if (metadata_item->type->data_type == STRING) {
    free(metadata_item->data.s);
  }
  free(metadata_item);
}

void free_metadata_row(struct metadata_row *metadata_row) {
  int i;
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    free_metadata_item(metadata_item);
  }
  free(metadata_row->items);
  free(metadata_row);
}

void free_metadata_rows(struct metadata_rows *metadata_rows) {
  int i;
  for (i = 0; i < metadata_rows->num; ++i) {
    free_metadata_row(metadata_rows->rows[i]);
  }
  free(metadata_rows->rows);
  free(metadata_rows);
}

void free_query_filter(struct query_filter *query_filter) {
  if (query_filter->type->data_type == STRING) {
    free(query_filter->data.s);
  }
  free(query_filter);
}

void display_metadata_columns(struct metadata_columns *metadata_columns) {
  int i;
  printf("metadata_columns => num_cols: %d, row_width: %d\n", metadata_columns->num_cols, metadata_columns->row_width);
  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    printf("%d => column: %d, data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_column->column, metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void display_metadata_types(struct metadata_types *metadata_types) {
  int i;
  printf("metadata_types => num_cols: %d, num_rows: %lu, row_width: %d, header_offset: %lu\n", metadata_types->num_cols, metadata_types->num_rows, metadata_types->row_width, metadata_types->header_offset);
  for (i = 0; i < metadata_types->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_types->types[i];
    printf("%d => data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void display_metadata_data(struct metadata_type *type, union metadata_data data) {
  printf("%s: ", type->name);
  switch (type->data_type) {
    case CHAR: 
      printf("%c", data.c);
      break;
    case INT_8: 
      printf("%hhu", data.b);
      break;
    case INT_16: 
      printf("%hu", data.h);
      break;
    case INT_32: 
      printf("%u", data.i);
      break;
    case INT_64: 
      printf("%lu", data.l);
      break;
    case FLOAT: 
      printf("%f", data.f);
      break;
    case DOUBLE: 
      printf("%lf", data.d);
      break;
    case STRING: 
      printf("%s", data.s);
      break;
    default:
      err(1, "Unknown data_type %d.\n", type->data_type);
  }
}

void display_comparison(enum comparison comparison) {
  switch (comparison) {
    case EQUAL: 
      printf("==");
      break;
    case NOT_EQUAL: 
      printf("!=");
      break;
    case LESS: 
      printf("<");
      break;
    case GREATER: 
      printf(">");
      break;
    case LESS_EQUAL: 
      printf("<=");
      break;
    case GREATER_EQUAL: 
      printf(">=");
      break;
    default:
      err(1, "Unknown comparison %d.\n", comparison);
  }
}

void display_metadata_item(struct metadata_item *metadata_item, int row_id, int column_id) {
  printf("metadata_row_%d_col_%d => ", row_id, column_id);
  display_metadata_data(metadata_item->type, metadata_item->data);
  printf("\n");
}

void display_metadata_row(struct metadata_row *metadata_row, int row_id) {
  int i;
  printf("metadata_row %d => ", row_id);
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    display_metadata_data(metadata_item->type, metadata_item->data);
    printf(", ");
  }
  printf("\n");
}

void display_metadata_rows(struct metadata_rows *metadata_rows) {
  int i, j;
  printf("metadata_rows => num_rows: %lu\n", metadata_rows->num);
  for (i = 0; i < metadata_rows->num; ++i) {
    struct metadata_row *metadata_row = metadata_rows->rows[i];
    display_metadata_row(metadata_row, i);
  }
}

void display_query_filter(struct query_filter *query_filter) {
  int i;
  display_metadata_data(query_filter->type, query_filter->data);
  printf(", comparison: ");
  display_comparison(query_filter->comparison);
  printf("\n");
}

void init_metadata_dat(char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *metadata_index = fopen(metadata_index_filename, "wb");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  int i;
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};

  if (fwrite(GIGGLE_METADATA_FILE_MARKER, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index) != GIGGLE_METADATA_FILE_MARKER_LENGTH) {
    err(1, "fwrite failure for file marker in init_metadata_dat.\n");
  }

  if (fwrite(GIGGLE_METADATA_VERSION_MARKER, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index) != GIGGLE_METADATA_VERSION_MARKER_LENGTH) {
    err(1, "fwrite failure for version marker in init_metadata_dat.\n");
  }

  if (fwrite(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index) != GIGGLE_METADATA_EXTRA_LENGTH) {
    err(1, "fwrite failure for extra in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->num_cols), sizeof(uint8_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for metadata_columns->num_cols in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->row_width), sizeof(uint16_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for metadata_columns->row_width in init_metadata_dat.\n");
  }

  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    
    char type_char = data_type_to_char(metadata_type->data_type);
    if (fwrite(&type_char, sizeof(char), 1, metadata_index) != 1) {
      err(1, "fwrite failure for type_char in init_metadata_dat.\n");
    }

    if (fwrite(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index) != 1) {
      err(1, "fwrite failure for metadata_type->width in init_metadata_dat.\n");
    }

    if (fwrite(metadata_type->name, sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index) != COLUMN_NAME_MAX_LENGTH) {
      err(1, "fwrite failure for metadata_type->name in init_metadata_dat.\n");
    }
  }

  fclose(metadata_index);
}

void append_metadata_dat(char *intervals_filename, char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *intervals = fopen(intervals_filename, "r");
  if (intervals == NULL) {
    err(1, "%s not found.\n", intervals_filename);
  }
  FILE *metadata_index = fopen(metadata_index_filename, "r+b");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  uint64_t num_rows = 0;

  if (fseek(metadata_index, 0, SEEK_END) != 0) {
    err(1, "Could not seek to the end to append in '%s'.", metadata_index_filename);
  }
  uint64_t curr_offset = ftell(metadata_index);

  if (fwrite(&num_rows, sizeof(uint64_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for num_rows in append_metadata_dat.\n");
  }

  while ((read = getline(&line, &len, intervals)) != -1) {
    // printf("Retrieved line of length %zu:\n%s\n", read, line);
    
    kstring_t kline = {0, 0, NULL};
    kputs(line, &kline);

    int fields_length;
    int *fields;
    fields = ksplit(&kline, '\t', &fields_length);

    for (int i = 0; i < metadata_columns->num_cols; ++i) {
      struct metadata_column *metadata_column = metadata_columns->columns[i];
      int column = metadata_column->column;
      struct metadata_type *metadata_type = metadata_column->type;
      char *data = kline.s + fields[column - 1];
      fwrite_data_type_item(metadata_index, metadata_type, data);
    }

    ++num_rows;

    free(fields);
    free(kline.s);
  }

  if (line)
    free(line);

  if (fseek(metadata_index, curr_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }

  if (fwrite(&num_rows, sizeof(uint64_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for num_rows in append_metadata_dat.\n");
  }

  fclose(intervals);
  fclose(metadata_index);
}

struct metadata_types *read_metadata_types_from_metadata_dat(char *metadata_index_filename) {
  FILE *metadata_index = fopen(metadata_index_filename, "rb");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  
  struct metadata_types *metadata_types = (struct metadata_types *)malloc(sizeof(struct metadata_types));
  if (metadata_types == NULL) {
    err(1, "malloc failure for metadata_types in read_metadata_types_from_metadata_dat.\n");
  }

  int i;
  size_t fr;
  char file_marker[7];
  char version_marker[3];
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};
  uint16_t col_offset = 0;

  metadata_types->column_name_to_index = khash_str2int_init();

  fr = fread(file_marker, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, GIGGLE_METADATA_FILE_MARKER_LENGTH, fr);
  if (strcmp(file_marker, GIGGLE_METADATA_FILE_MARKER) != 0) {
    err(1, "Not a GIGGLE Metadata Index file.\n");
  }

  fr = fread(version_marker, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, GIGGLE_METADATA_VERSION_MARKER_LENGTH, fr);
  if (strcmp(version_marker, GIGGLE_METADATA_VERSION_MARKER) != 0) {
    err(1, "Incompatible GIGGLE Metadata Index version.\n");
  }
  
  fr = fread(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, GIGGLE_METADATA_EXTRA_LENGTH, fr);

  fr = fread(&(metadata_types->num_cols), sizeof(uint8_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);
  
  fr = fread(&(metadata_types->row_width), sizeof(uint16_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);

  metadata_types->col_offsets = (uint16_t *)malloc(metadata_types->num_cols * sizeof(uint16_t));
  if (metadata_types->col_offsets == NULL) {
    err(1, "malloc failure for metadata_types->col_offsets in read_metadata_types_from_metadata_dat.\n");
  }
  
  metadata_types->types = (struct metadata_type **)malloc(metadata_types->num_cols * sizeof(struct metadata_type*));
  if (metadata_types->types == NULL) {
    err(1, "malloc failure for metadata_types->types in read_metadata_types_from_metadata_dat.\n");
  }

  for (i = 0; i < metadata_types->num_cols; ++i) {
    struct metadata_type *metadata_type = (struct metadata_type *)calloc(1, sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      err(1, "calloc failure for metadata_type in read_metadata_types_from_metadata_dat.\n");
    }

    char type_char;
    fr = fread(&type_char, sizeof(char), 1, metadata_index);
    check_file_read(metadata_index_filename, metadata_index, 1, fr);
    metadata_type->data_type = type_char_to_enum(type_char);

    fr = fread(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index);
    check_file_read(metadata_index_filename, metadata_index, 1, fr);

    fr = fread(&(metadata_type->name), sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index);
    check_file_read(metadata_index_filename, metadata_index, COLUMN_NAME_MAX_LENGTH, fr);

    khash_str2int_set(metadata_types->column_name_to_index, strdup(metadata_type->name), i);

    metadata_types->col_offsets[i] = col_offset;
    col_offset += metadata_type->width;

    metadata_types->types[i] = metadata_type;
  }
  
  fr = fread(&(metadata_types->num_rows), sizeof(uint64_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);

  metadata_types->header_offset = ftell(metadata_index);

  fclose(metadata_index);

  return metadata_types;
}

struct metadata_rows *read_metadata_rows(char *metadata_index_filename, struct metadata_types *metadata_types) {
  FILE *metadata_index = fopen(metadata_index_filename, "rb");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  
  if (fseek(metadata_index, metadata_types->header_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  struct metadata_rows *metadata_rows = (struct metadata_rows *)malloc(sizeof(struct metadata_rows));
  if (metadata_rows == NULL) {
    err(1, "malloc failure for metadata_rows in read_metadata_rows.\n");
  }

  metadata_rows->num = metadata_types->num_rows;

  int i, j;

  metadata_rows->rows = (struct metadata_row **)malloc(metadata_rows->num * sizeof(struct metadata_row*));
  if (metadata_rows->rows == NULL) {
    err(1, "malloc failure for metadata_rows->rows in read_metadata_rows.\n");
  }

  for (i = 0; i < metadata_rows->num; ++i) {
    struct metadata_row *metadata_row = (struct metadata_row *)malloc(sizeof(struct metadata_row));
    if (metadata_row == NULL) {
      err(1, "malloc failure for metadata_row in read_metadata_rows.\n");
    }

    metadata_row->num = metadata_types->num_cols;
    
    metadata_row->items = (struct metadata_item **)malloc(metadata_row->num * sizeof(struct metadata_item*));
    if (metadata_row->items == NULL) {
      err(1, "malloc failure for metadata_row->items in read_metadata_rows.\n");
    }

    for (j = 0; j < metadata_row->num; ++j) {
      struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
      if (metadata_item == NULL) {
        err(1, "malloc failure for metadata_item in read_metadata_rows.\n");
      }
      metadata_item->type = metadata_types->types[j];

      fread_data_type_item(metadata_index_filename, metadata_index, metadata_item);

      metadata_row->items[j] = metadata_item;
    }

    metadata_rows->rows[i] = metadata_row;
  }
  
  fclose(metadata_index);

  return metadata_rows;
}

struct metadata_row *read_metadata_row(char *metadata_index_filename, struct metadata_types *metadata_types, uint64_t interval_id) {
  FILE *metadata_index = fopen(metadata_index_filename, "rb");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  uint64_t total_offset = metadata_types->header_offset + metadata_types->row_width * interval_id;
  
  if (fseek(metadata_index, total_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  int i;
  size_t fr, data_width;

  struct metadata_row *metadata_row = (struct metadata_row *)malloc(sizeof(struct metadata_row));
  if (metadata_row == NULL) {
    err(1, "malloc failure for metadata_row in read_metadata_row.\n");
  }

  metadata_row->num = metadata_types->num_cols;
  
  metadata_row->items = (struct metadata_item **)malloc(metadata_row->num * sizeof(struct metadata_item*));
  if (metadata_row->items == NULL) {
    err(1, "malloc failure for metadata_row->items in read_metadata_row.\n");
  }

  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
    if (metadata_item == NULL) {
      err(1, "malloc failure for metadata_item in read_metadata_row.\n");
    }
    metadata_item->type = metadata_types->types[i];

    fread_data_type_item(metadata_index_filename, metadata_index, metadata_item);

    metadata_row->items[i] = metadata_item;
  }
  
  fclose(metadata_index);

  return metadata_row;
}

struct metadata_item *read_metadata_item(char *metadata_index_filename, struct metadata_types *metadata_types, uint64_t interval_id, uint8_t column_id) {
  FILE *metadata_index = fopen(metadata_index_filename, "rb");
  if (metadata_index == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  uint64_t total_offset = metadata_types->header_offset + metadata_types->row_width * interval_id + metadata_types->col_offsets[column_id];
  
  if (fseek(metadata_index, total_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  size_t fr;

  struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
  if (metadata_item == NULL) {
    err(1, "malloc failure for metadata_item in read_metadata_item.\n");
  }
  metadata_item->type = metadata_types->types[column_id];

  fread_data_type_item(metadata_index_filename, metadata_index, metadata_item);
  
  fclose(metadata_index);

  return metadata_item;
}

struct query_filter *parse_query_filter_string(struct metadata_types *metadata_types, char *query_filter_string) {
  struct query_filter *query_filter = (struct query_filter *)malloc(sizeof(struct query_filter));
  if (query_filter == NULL) {
    err(1, "malloc failure for query_filter in parse_query_filter_string.\n");
  }

  int start_comparison, end_comparison, lookup_result;
  char *data_string;
  char null_tmp;
  int column_id;

  query_filter->comparison = comparison_string_to_enum(query_filter_string, &start_comparison, &end_comparison);
  
  null_tmp = query_filter_string[start_comparison];
  query_filter_string[start_comparison] = 0; // null-terminate the column name after parsing comparison operator

  lookup_result = khash_str2int_get(metadata_types->column_name_to_index, query_filter_string, &column_id);
  query_filter_string[start_comparison] = null_tmp;

  if (lookup_result == -1) {
    err(1, "Column %s not found in metadata.\n", query_filter_string);
  }

  query_filter->type = metadata_types->types[column_id];
  query_filter->column_id = column_id;

  data_string = query_filter_string + end_comparison + 1;
  query_filter->data = parse_query_filter_data_string(query_filter->type, data_string);

  return query_filter;
}

int perform_metadata_comparison(struct metadata_type *metadata_type, enum comparison comparison, union metadata_data source, union metadata_data target) {
  enum data_type type = metadata_type->data_type;
  int less, equal, greater;
  int result;
  char c;
  int8_t b;
  int16_t h;
  int32_t i;
  int64_t l;
  float f;
  double d;
  int s;

  switch (type) {
    case CHAR: 
      c = source.c - target.c;
      less = (c < 0); equal = (c == 0); greater = (c > 0);
      break;
    case INT_8: 
      b = source.b - target.b;
      less = (b < 0); equal = (b == 0); greater = (b > 0);
      break;
    case INT_16: 
      h = source.h - target.h;
      less = (h < 0); equal = (h == 0); greater = (h > 0);
      break;
    case INT_32: 
      i = source.i - target.i;
      less = (i < 0); equal = (i == 0); greater = (i > 0);
      break;
    case INT_64: 
      l = source.l - target.l;
      less = (l < 0); equal = (l == 0); greater = (l > 0);
      break;
    case FLOAT: 
      f = source.f - target.f;
      less = (f < 0); equal = (f == 0); greater = (f > 0);
      break;
    case DOUBLE: 
      d = source.d - target.d;
      less = (d < 0); equal = (d == 0); greater = (d > 0);
      break;
    case STRING: 
      s = strcmp(source.s, target.s);
      less = (s < 0); equal = (s == 0); greater = (s > 0);
      break;
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
  
  switch (comparison) {
    case EQUAL: 
      result = equal;
      break;
    case NOT_EQUAL: 
      result = !equal;
      break;
    case LESS: 
      result = less;
      break;
    case GREATER: 
      result = greater;
      break;
    case LESS_EQUAL: 
      result = less || equal;
      break;
    case GREATER_EQUAL: 
      result = greater || equal;
      break;
    default:
      err(1, "Unknown comparison %d.\n", comparison);
  }

  return result;
}

int filter_metadata_row_by_item(struct metadata_item *metadata_item, struct query_filter *query_filter) {  
  struct metadata_type *metadata_type = query_filter->type;
  enum comparison comparison = query_filter->comparison;
  uint8_t column_id = query_filter->column_id;
  union metadata_data target = query_filter->data;

  union metadata_data source = metadata_item->data;

  return perform_metadata_comparison(metadata_type, comparison, source, target);
}

int filter_metadata_row_by_row(struct metadata_row *metadata_row, struct query_filter *query_filter) {  
  uint8_t column_id = query_filter->column_id;
  struct metadata_item *metadata_item = metadata_row->items[column_id];
  return filter_metadata_row_by_item(metadata_item, query_filter);
}

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index.dat";
  char *intervals_filename = "intervals.tsv";

  // 1. Read metadata.conf
  struct metadata_columns *metadata_columns = read_metadata_conf(metadata_conf_filename);
  printf("\nCreated metadata_columns from %s\n", metadata_conf_filename);
  display_metadata_columns(metadata_columns);

  // 2. Write header in metadata_index.dat
  init_metadata_dat(metadata_index_filename, metadata_columns);
  printf("\nInitialized Metadata Index in %s\n", metadata_index_filename);

  // 3. Read intervals.tsv and append data to metadata_index.dat
  append_metadata_dat(intervals_filename, metadata_index_filename, metadata_columns);
  printf("\nAppended Metadata from %s to %s\n", intervals_filename, metadata_index_filename);

  // 4. Read metadata_types from metadata_index.dat
  struct metadata_types *metadata_types = read_metadata_types_from_metadata_dat(metadata_index_filename);
  printf("\nRead metadata_types from %s\n", metadata_index_filename);
  display_metadata_types(metadata_types);

  // 5. Read metadata rows from metadata_index.dat
  struct metadata_rows *metadata_rows = read_metadata_rows(metadata_index_filename, metadata_types);
  printf("\nRead metadata_rows from %s\n", metadata_index_filename);
  display_metadata_rows(metadata_rows);
  
  // 6. Read ith interval's metadata from metadata_index.dat
  struct metadata_row *metadata_row_1 = read_metadata_row(metadata_index_filename, metadata_types, 1);
  struct metadata_row *metadata_row_3 = read_metadata_row(metadata_index_filename, metadata_types, 3);
  printf("\nRead metadata_rows 1 and 3 from %s\n", metadata_index_filename);
  display_metadata_row(metadata_row_1, 1);  
  display_metadata_row(metadata_row_3, 3);

  // 7. Read ith interval's jth metadata column from metadata_index.dat
  struct metadata_item *metadata_row_0_col_2 = read_metadata_item(metadata_index_filename, metadata_types, 0, 2);
  struct metadata_item *metadata_row_2_col_3 = read_metadata_item(metadata_index_filename, metadata_types, 2, 3);
  printf("\nRead metadata_items row_0_col_0 and row_2_col_4 from %s\n", metadata_index_filename);
  display_metadata_item(metadata_row_0_col_2, 0, 2);  
  display_metadata_item(metadata_row_2_col_3, 2, 3);

  // 8. Read query filter
  char query_filter_string_1[] = "feature<my_feature";
  char query_filter_string_2[] = "score>=456.5";
  struct query_filter *query_filter_1 = parse_query_filter_string(metadata_types, query_filter_string_1);
  struct query_filter *query_filter_2 = parse_query_filter_string(metadata_types, query_filter_string_2);
  printf("\nParsed query filter string: %s\n", query_filter_string_1);
  display_query_filter(query_filter_1);
  printf("\nParsed query filter string: %s\n", query_filter_string_2);
  display_query_filter(query_filter_2);

  // 9. Filter rows using query filters
  int row_1_filter_1 = filter_metadata_row_by_row(metadata_row_1, query_filter_1);
  int row_1_filter_2 = filter_metadata_row_by_row(metadata_row_1, query_filter_2);
  int row_3_filter_1 = filter_metadata_row_by_row(metadata_row_3, query_filter_1);
  int row_3_filter_2 = filter_metadata_row_by_row(metadata_row_3, query_filter_2);
  printf("\nFiltered rows using query filters\n");
  printf("row_1_filter_1: %d, row_1_filter_2: %d\n", row_1_filter_1, row_1_filter_2);
  printf("row_3_filter_1: %d, row_3_filter_2: %d\n", row_3_filter_1, row_3_filter_2);

  // 10. Filter items using query filters
  int row_0_col_2 = filter_metadata_row_by_item(metadata_row_0_col_2, query_filter_1);
  int row_2_col_3 = filter_metadata_row_by_item(metadata_row_2_col_3, query_filter_2);
  printf("\nFiltered items using query filters\n");
  printf("row_0_col_2: %d\n", row_0_col_2);
  printf("row_2_col_3: %d\n", row_2_col_3);

  // free memory
  free_query_filter(query_filter_1);
  free_query_filter(query_filter_2);
  free_metadata_item(metadata_row_0_col_2);
  free_metadata_item(metadata_row_2_col_3);
  free_metadata_row(metadata_row_1);
  free_metadata_row(metadata_row_3);
  free_metadata_rows(metadata_rows);
  free_metadata_types(metadata_types);
  free_metadata_columns(metadata_columns);

  return 0;
}
