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
  <256-byte char*, name> <1-byte char, data type specifier> <optional, 1-byte uint8, width>
>
<4-byte num> <4-byte size>
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
  uint8_t num; // max item count = 255
  uint16_t width; // total width of each data row
  struct metadata_type **types;
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
    fprintf(stderr, "Invalid data type %s.\n", type_string);
    exit(EXIT_FAILURE);
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
      fprintf(stderr, "Unknown data_type %d.\n", type);
      exit(EXIT_FAILURE);
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
      fprintf(stderr, "Unknown data_type %d.\n", type);
      exit(EXIT_FAILURE);
  }
  return type_char;
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
        fprintf(stderr, "fwrite failure for CHAR in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;

    case INT_8: 
      b = atoi(data);
      if (fwrite(&b, sizeof(int8_t), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for INT_8 in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case INT_16: 
      h = atoi(data);
      if (fwrite(&h, sizeof(int16_t), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for INT_16 in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case INT_32: 
      i = atol(data);
      if (fwrite(&i, sizeof(int32_t), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for INT_32 in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case INT_64: 
      l = atoll(data);
      if (fwrite(&l, sizeof(int64_t), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for INT_64 in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case FLOAT: 
      f = atof(data);
      if (fwrite(&f, sizeof(float), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for FLOAT in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case DOUBLE: 
      d = atof(data);
      if (fwrite(&d, sizeof(double), 1, metadata_index) != 1) {
        fprintf(stderr, "fwrite failure for DOUBLE in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      break;
      
    case STRING: 
      snprintf(s_format, sizeof(s_format), "%%%ds", str_width - 1);
      s = (char *)calloc(str_width, sizeof(char));
      if (s == NULL) {
        fprintf(stderr, "calloc failure for s in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      sscanf(data, s_format, s);

      if (fwrite(s, sizeof(char), str_width, metadata_index) != str_width) {
        fprintf(stderr, "fwrite failure for STRING in fwrite_data_type_item.\n");
        exit(EXIT_FAILURE);
      }
      free(s);
      break;
      
    default:
      fprintf(stderr, "Unknown data_type %d.\n", type);
      exit(EXIT_FAILURE);
  }
}

struct metadata_columns *read_metadata_conf(char *metadata_conf_filename) {
  FILE *metadata_conf = fopen(metadata_conf_filename, "r");
  if (metadata_conf == NULL) {
    fprintf(stderr, "%s not found.\n", metadata_conf_filename);
    exit(EXIT_FAILURE);
  }

  struct metadata_columns *metadata_columns = (struct metadata_columns *)malloc(sizeof(struct metadata_columns));
  if (metadata_columns == NULL) {
    fprintf(stderr, "malloc failure for metadata_columns in read_metadata_conf.\n");
    exit(EXIT_FAILURE);
  }
  metadata_columns->num = 0;
  metadata_columns->width = 0;

  metadata_columns->columns = (struct metadata_column **)malloc(255 * sizeof(struct metadata_column*));
  if (metadata_columns->columns == NULL) {
    fprintf(stderr, "malloc failure for metadata_columns->columns in read_metadata_conf.\n");
    exit(EXIT_FAILURE);
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
    sscanf(line, "%hhd %255s %7s %hhd", &column, name, type_string, &str_len);
    printf("%d %s %s %d\n", column, name, type_string, str_len);

    if (metadata_columns->num == 255) {
      fprintf(stderr, "Cannot store more than 255 columns.\n");
      exit(EXIT_FAILURE);
    }
    
    if (str_len == 255) {
      fprintf(stderr, "Column '%s': string length cannot be more than 254.\n", name);
      exit(EXIT_FAILURE);
    }

    if (khash_str2int_has_key(column_set, name)) {
      fprintf(stderr, "Cannot allow duplicate column '%s'.\n", name);
      exit(EXIT_FAILURE);
    } else {
      khash_str2int_set(column_set, strdup(name), 1);
    }

    struct metadata_type *metadata_type = (struct metadata_type *)calloc(1, sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      fprintf(stderr, "calloc failure for metadata_type in read_metadata_conf.\n");
      exit(EXIT_FAILURE);
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
      fprintf(stderr, "malloc failure for metadata_column in read_metadata_conf.\n");
      exit(EXIT_FAILURE);
    }

    metadata_column->type = metadata_type;
    metadata_column->column = column;
    
    metadata_columns->columns[metadata_columns->num++] = metadata_column;

  }

  if (metadata_columns->num < 255) {
    metadata_columns->columns = (struct metadata_column **)realloc(metadata_columns->columns, sizeof(struct metadata_column*) * metadata_columns->num);
    if (metadata_columns->columns == NULL) {
      fprintf(stderr, "realloc failure for metadata_columns->columns in read_metadata_conf.\n");
      exit(EXIT_FAILURE);
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
  // for (i = 0; i < metadata_types->num; ++i) {
  //   struct metadata_column *metadata_column = metadata_types->types[i];
  //   struct metadata_type *metadata_type = metadata_column->type;
  //   free(metadata_type);
  //   free(metadata_column);
  // }
  free(metadata_types->types);
  free(metadata_types);
}

void display_metadata(struct metadata_columns *metadata_columns) {
  int i;
  printf("%d %d\n", metadata_columns->num, metadata_columns->width);
  for (i = 0; i < metadata_columns->num; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    printf("%d %d %s %d\n", metadata_column->column, metadata_type->data_type, metadata_type->name, metadata_type->width);
  }
}

void init_metadata_dat(char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *metadata_index = fopen(metadata_index_filename, "wb");
  if (metadata_index == NULL) {
    fprintf(stderr, "%s not found.\n", metadata_index_filename);
    exit(EXIT_FAILURE);
  }

  int i;
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};

  if (fwrite(GIGGLE_METADATA_FILE_MARKER, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index) != GIGGLE_METADATA_FILE_MARKER_LENGTH) {
    fprintf(stderr, "fwrite failure for file marker in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(GIGGLE_METADATA_VERSION_MARKER, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index) != GIGGLE_METADATA_VERSION_MARKER_LENGTH) {
    fprintf(stderr, "fwrite failure for version marker in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index) != GIGGLE_METADATA_EXTRA_LENGTH) {
    fprintf(stderr, "fwrite failure for extra in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(&(metadata_columns->num), sizeof(uint8_t), 1, metadata_index) != 1) {
    fprintf(stderr, "fwrite failure for metadata_columns->num in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(&(metadata_columns->width), sizeof(uint16_t), 1, metadata_index) != 1) {
    fprintf(stderr, "fwrite failure for metadata_columns->width in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < metadata_columns->num; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    
    char type_char = data_type_to_char(metadata_type->data_type);
    if (fwrite(&type_char, sizeof(char), 1, metadata_index) != 1) {
      fprintf(stderr, "fwrite failure for type_char in init_metadata_dat.\n");
      exit(EXIT_FAILURE);
    }

    if (fwrite(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index) != 1) {
      fprintf(stderr, "fwrite failure for metadata_type->width in init_metadata_dat.\n");
      exit(EXIT_FAILURE);
    }

    if (fwrite(metadata_type->name, sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index) != COLUMN_NAME_MAX_LENGTH) {
      fprintf(stderr, "fwrite failure for metadata_type->name in init_metadata_dat.\n");
      exit(EXIT_FAILURE);
    }
  }

  fclose(metadata_index);
}

void append_metadata_dat(char *intervals_filename, char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *intervals = fopen(intervals_filename, "r");
  if (intervals == NULL) {
    fprintf(stderr, "%s not found.\n", intervals_filename);
    exit(EXIT_FAILURE);
  }
  FILE *metadata_index = fopen(metadata_index_filename, "a+b");
  if (metadata_index == NULL) {
    fprintf(stderr, "%s not found.\n", metadata_index_filename);
    exit(EXIT_FAILURE);
  }
  
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

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

    free(fields);
    free(kline.s);
  }

  if (line)
    free(line);

  fclose(intervals);
  fclose(metadata_index);
}

struct metadata_types *get_columns_from_metadata_dat(char *metadata_index_filename) {
  FILE *metadata_index = fopen(metadata_index_filename, "rb");
  if (metadata_index == NULL) {
    fprintf(stderr, "%s not found.\n", metadata_index_filename);
    exit(EXIT_FAILURE);
  }
  
  struct metadata_types *metadata_types = (struct metadata_types *)malloc(sizeof(struct metadata_types));
  if (metadata_types == NULL) {
    fprintf(stderr, "malloc failure for metadata_types in read_metadata_conf.\n");
    exit(EXIT_FAILURE);
  }

  int i;
  size_t fr;
  
  fr = fread(&(metadata_types->num), sizeof(uint8_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);
  
  fr = fread(&(metadata_types->width), sizeof(uint16_t), 1, metadata_index);
  check_file_read(metadata_index_filename, metadata_index, 1, fr);

  metadata_types->types = (struct metadata_type **)malloc(metadata_types->num * sizeof(struct metadata_type*));
  if (metadata_types->types == NULL) {
    fprintf(stderr, "calloc failure for metadata_types->types in read_metadata_conf.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < metadata_types->num; ++i) {
    
    // <256-byte char*, name>
    // <1-byte char, data type specifier>
    // <optional, 1-byte uint8, width>


  }

  fclose(metadata_index);
  
  return metadata_types;
}

struct metadata_types *read_metadata_dat() {
  // TODO
}

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index.dat";
  char *intervals_filename = "intervals.tsv";

  // 1. Read metadata.conf
  struct metadata_columns *metadata_columns = read_metadata_conf(metadata_conf_filename);

  display_metadata(metadata_columns);

  // 2. Write header in metadata_index.dat
  init_metadata_dat(metadata_index_filename, metadata_columns);

  // 3. Read intervals.tsv and append data to metadata_index.dat
  append_metadata_dat(intervals_filename, metadata_index_filename, metadata_columns);

  // 4. Read ith interval's metadata from metadata_index.dat
  struct metadata_types *metadata_types = get_columns_from_metadata_dat(metadata_index_filename);

  free_metadata_columns(metadata_columns);
  free_metadata_types(metadata_types);

  return 0;
}
