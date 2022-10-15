#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define COLUMN_NAME_MAX_LENGTH 32

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
  struct metadata_type **types;
};

enum data_type type_string_to_enum(char type_string[8]) {
  enum data_type type;
  for(int i = 0; type_string[i]; i++){
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
      break;
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
      break;
  }
  return type_char;
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

  metadata_columns->columns = (struct metadata_column **)malloc(sizeof(struct metadata_column*) * 255);
  if (metadata_columns->columns == NULL) {
    fprintf(stderr, "malloc failure for metadata_columns->columns in read_metadata_conf.\n");
    exit(EXIT_FAILURE);
  }

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

    struct metadata_type *metadata_type = (struct metadata_type *)malloc(sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      fprintf(stderr, "malloc failure for metadata_type in read_metadata_conf.\n");
      exit(EXIT_FAILURE);
    }

    strncpy(metadata_type->name, name, strlen(name) + 1);
    metadata_type->data_type = type_string_to_enum(type_string);
    metadata_type->width += data_type_to_width(metadata_type->data_type);
    if (metadata_type->data_type == STRING) {
      metadata_type->width = str_len;
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

  if (line)
    free(line);

  if (metadata_columns->num < 255) {
    metadata_columns->columns = (struct metadata_column **)realloc(metadata_columns->columns, sizeof(struct metadata_column*) * metadata_columns->num);
  }

  fclose(metadata_conf);
  return metadata_columns;
}

void free_metadata(struct metadata_columns *metadata_columns) {
  int i;
  for (i = 0; i < metadata_columns->num; i++) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    free(metadata_type);
    free(metadata_column);
  }
  free(metadata_columns->columns);
  free(metadata_columns);
}

void display_metadata(struct metadata_columns *metadata_columns) {
  int i;
  printf("%d %d\n", metadata_columns->num, metadata_columns->width);
  for (i = 0; i < metadata_columns->num; i++) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    printf("%d %d %s %d\n", metadata_column->column, metadata_type->data_type, metadata_type->name, metadata_type->width);
  }
}

void init_metadata_dat(struct metadata_columns *metadata_columns, char *metadata_index_filename) {
  FILE *metadata_index = fopen(metadata_index_filename, "wb");
  int i;

  if (fwrite(&(metadata_columns->num), sizeof(uint8_t), 1, metadata_index) != 1) {
    fprintf(stderr, "fwrite failure for metadata_columns->num in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(&(metadata_columns->width), sizeof(uint16_t), 1, metadata_index) != 1) {
    fprintf(stderr, "fwrite failure for metadata_columns->width in init_metadata_dat.\n");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < metadata_columns->num; i++) {
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

struct metadata_types *read_metadata_dat() {
  // TODO
}

int main(void) {
  
  // 1. Read metadata.conf
  struct metadata_columns *metadata_columns = read_metadata_conf("metadata.conf");

  display_metadata(metadata_columns);

  // 2. Write header in metadata_index.dat
  init_metadata_dat(metadata_columns, "metadata_index.dat");

  // 3. Read intervals.tsv and append data to metadata_index.dat
  
  // 4. Read ith interval's metadata from metadata_index.dat
  

  free_metadata(metadata_columns);

  return 0;
}
