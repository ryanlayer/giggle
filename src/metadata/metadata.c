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
<1-byte uint8 num> <2-byte uint16 width>
< array of  
  <256-byte char*, name>
  <1-byte uint8, width>
  <1-byte char, data type specifier>
>
<8-byte num>
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
  uint8_t num; // max item count = 255
  uint16_t width; // total width of each data row
  struct metadata_column **columns;
};

struct metadata_types {
  uint8_t num_cols;
  uint64_t num_rows;
  uint16_t width; // total width of each data row
  uint64_t header_offset; // total header offset, end of the header file position
  struct metadata_type **types;
};

struct metadata_item {
  struct metadata_type *type;
  void *data;
};

struct metadata_row {
  uint8_t num; // total number of columns
  struct metadata_item **items; // array of rows
};

struct metadata_rows {
  uint64_t num; // total number of rows
  struct metadata_row **rows; // data rows
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

void fwrite_data_type_item(FILE *metadata_index, struct metadata_type *metadata_type, char *data) {
  enum data_type type = metadata_type->data_type;
  char c;
  int8_t b;
  int16_t h;
  int32_t i;
  int64_t l;
  float f;
  double d;
  uint8_t str_width = metadata_type->width;
  char s_format[6]; // '%' + max value for int8_t + 's' + NULL -> '%255sN'
  char *s;

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
      snprintf(s_format, sizeof(s_format), "%%%ds", str_width - 1);
      s = (char *)calloc(str_width, sizeof(char));
      if (s == NULL) {
        err(1, "calloc failure for s in fwrite_data_type_item.\n");
      }
      sscanf(data, s_format, s);

      if (fwrite(s, sizeof(char), str_width, metadata_index) != str_width) {
        err(1, "fwrite failure for STRING in fwrite_data_type_item.\n");
      }
      free(s);
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
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
  metadata_columns->num = 0;
  metadata_columns->width = 0;

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
    uint8_t str_len;
    sscanf(line, "%hhu %255s %7s %hhu", &column, name, type_string, &str_len);
    // printf("%d %s %s %d\n", column, name, type_string, str_len);

    if (metadata_columns->num == 255) {
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
    metadata_columns->width += metadata_type->width;

    struct metadata_column *metadata_column = (struct metadata_column *)malloc(sizeof(struct metadata_column));
    if (metadata_column == NULL) {
      err(1, "malloc failure for metadata_column in read_metadata_conf.\n");
    }

    metadata_column->type = metadata_type;
    metadata_column->column = column;
    
    metadata_columns->columns[metadata_columns->num++] = metadata_column;

  }

  if (metadata_columns->num < 255) {
    metadata_columns->columns = (struct metadata_column **)realloc(metadata_columns->columns, sizeof(struct metadata_column*) * metadata_columns->num);
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
  for (i = 0; i < metadata_columns->num; ++i) {
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
  free(metadata_types->types);
  free(metadata_types);
}

void free_metadata_rows(struct metadata_rows *metadata_rows) {
  int i, j;
  for (i = 0; i < metadata_rows->num; ++i) {
    struct metadata_row *metadata_row = metadata_rows->rows[i];

    for (j = 0; j < metadata_row->num; ++j) {
      struct metadata_item *metadata_item = metadata_row->items[j];
      free(metadata_item);
    }

    free(metadata_row->items);
    free(metadata_row);
  }
  free(metadata_rows->rows);
  free(metadata_rows);
}

void free_metadata_row(struct metadata_row *metadata_row) {
  int i;
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    free(metadata_item);
  }
  free(metadata_row->items);
  free(metadata_row);
}

void display_metadata_columns(struct metadata_columns *metadata_columns) {
  int i;
  printf("metadata_columns => num: %d, width: %d\n", metadata_columns->num, metadata_columns->width);
  for (i = 0; i < metadata_columns->num; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    printf("%d => column: %d, data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_column->column, metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void display_metadata_types(struct metadata_types *metadata_types) {
  int i;
  printf("metadata_types => num_cols: %d, num_rows: %lu, width: %d, header_offset: %lu\n", metadata_types->num_cols, metadata_types->num_rows, metadata_types->width, metadata_types->header_offset);
  for (i = 0; i < metadata_types->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_types->types[i];
    printf("%d => data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void display_metadata_item(struct metadata_item *metadata_item) {
  printf("%s: ", metadata_item->type->name);
  switch (metadata_item->type->data_type) {
    case CHAR: 
      printf("%c", *(char *)(&metadata_item->data));
      break;
    case INT_8: 
      printf("%hhu", *(int8_t *)(&metadata_item->data));
      break;
    case INT_16: 
      printf("%hu", *(int16_t *)(&metadata_item->data));
      break;
    case INT_32: 
      printf("%u", *(int32_t *)(&metadata_item->data));
      break;
    case INT_64: 
      printf("%lu", *(int64_t *)(&metadata_item->data));
      break;
    case FLOAT: 
      printf("%f", *(float *)(&metadata_item->data));
      break;
    case DOUBLE: 
      printf("%lf", *(double *)(&metadata_item->data));
      break;
    case STRING: 
      printf("%s", (char *)(&metadata_item->data));
      break;
    default:
      err(1, "Unknown data_type %d.\n", metadata_item->type->data_type);
  }
  printf(", ");
}

void display_metadata_rows(struct metadata_rows *metadata_rows) {
  int i, j;
  printf("metadata_rows => num_rows: %lu\n", metadata_rows->num);
  for (i = 0; i < metadata_rows->num; ++i) {
    printf("%d => ", i);
    struct metadata_row *metadata_row = metadata_rows->rows[i];
    for (j = 0; j < metadata_row->num; ++j) {
      struct metadata_item *metadata_item = metadata_row->items[j];
      display_metadata_item(metadata_item);
    }
    printf("\n");
  }
}

void display_metadata_row(struct metadata_row *metadata_row) {
  int i;
  printf("metadata_row => ");
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    display_metadata_item(metadata_item);
  }
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

  if (fwrite(&(metadata_columns->num), sizeof(uint8_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for metadata_columns->num in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->width), sizeof(uint16_t), 1, metadata_index) != 1) {
    err(1, "fwrite failure for metadata_columns->width in init_metadata_dat.\n");
  }

  for (i = 0; i < metadata_columns->num; ++i) {
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

    for (int i = 0; i < metadata_columns->num; ++i) {
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
  
  fr = fread(&(metadata_types->width), sizeof(uint16_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);

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
  size_t fr, data_width;

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
      data_width = metadata_item->type->width;

      fr = fread(&(metadata_item->data), data_width, 1, metadata_index);
      check_file_read(metadata_index_filename, metadata_index, 1, fr);

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

  uint64_t total_offset = metadata_types->header_offset + metadata_types->width * interval_id;
  
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
    data_width = metadata_item->type->width;

    fr = fread(&(metadata_item->data), data_width, 1, metadata_index);
    check_file_read(metadata_index_filename, metadata_index, 1, fr);

    metadata_row->items[i] = metadata_item;
  }
  
  fclose(metadata_index);

  return metadata_row;
}

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index.dat";
  char *intervals_filename = "intervals.tsv";

  // 1. Read metadata.conf
  struct metadata_columns *metadata_columns = read_metadata_conf(metadata_conf_filename);
  printf("Created metadata_columns from %s.\n", metadata_conf_filename);
  display_metadata_columns(metadata_columns);

  // 2. Write header in metadata_index.dat
  init_metadata_dat(metadata_index_filename, metadata_columns);
  printf("Initialized Metadata Index in %s.\n", metadata_index_filename);

  // 3. Read intervals.tsv and append data to metadata_index.dat
  append_metadata_dat(intervals_filename, metadata_index_filename, metadata_columns);
  printf("Appended Metadata from %s to %s.\n", intervals_filename, metadata_index_filename);

  // 4. Read metadata_types from metadata_index.dat
  struct metadata_types *metadata_types = read_metadata_types_from_metadata_dat(metadata_index_filename);
  printf("Read metadata_types from %s.\n", metadata_index_filename);
  display_metadata_types(metadata_types);

  // 5. Read metadata rows from metadata_index.dat
  struct metadata_rows *metadata_rows = read_metadata_rows(metadata_index_filename, metadata_types);
  printf("Read metadata_rows from %s.\n", metadata_index_filename);
  display_metadata_rows(metadata_rows);
  
  // 6. Read ith interval's metadata from metadata_index.dat
  struct metadata_row *metadata_row_1 = read_metadata_row(metadata_index_filename, metadata_types, 1);
  display_metadata_row(metadata_row_1);
  
  struct metadata_row *metadata_row_3 = read_metadata_row(metadata_index_filename, metadata_types, 3);
  display_metadata_row(metadata_row_3);

  free_metadata_columns(metadata_columns);
  free_metadata_types(metadata_types);
  free_metadata_rows(metadata_rows);
  free_metadata_row(metadata_row_1);
  free_metadata_row(metadata_row_3);

  return 0;
}
