#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <htslib/khash_str2int.h>
#include "metadata_index.h"

// TODO: remove this after integrating with main codebase
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

void fwrite_data_type_item(FILE *metadata_index_fp, struct metadata_type *metadata_type, char *data) {
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
      if (fwrite(&c, sizeof(char), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for CHAR in fwrite_data_type_item.\n");
      }
      break;

    case INT_8: 
      b = atoi(data);
      if (fwrite(&b, sizeof(int8_t), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for INT_8 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_16: 
      h = atoi(data);
      if (fwrite(&h, sizeof(int16_t), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for INT_16 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_32: 
      i = atol(data);
      if (fwrite(&i, sizeof(int32_t), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for INT_32 in fwrite_data_type_item.\n");
      }
      break;
      
    case INT_64: 
      l = atoll(data);
      if (fwrite(&l, sizeof(int64_t), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for INT_64 in fwrite_data_type_item.\n");
      }
      break;
      
    case FLOAT: 
      f = atof(data);
      if (fwrite(&f, sizeof(float), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for FLOAT in fwrite_data_type_item.\n");
      }
      break;
      
    case DOUBLE: 
      d = atof(data);
      if (fwrite(&d, sizeof(double), 1, metadata_index_fp) != 1) {
        err(1, "fwrite failure for DOUBLE in fwrite_data_type_item.\n");
      }
      break;
      
    case STRING: 
      str_width = metadata_type->width;
      s = safe_sscanf(str_width, data);
      if (fwrite(s, sizeof(char), str_width, metadata_index_fp) != str_width) {
        err(1, "fwrite failure for STRING in fwrite_data_type_item.\n");
      }
      free(s);
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
}

void fread_data_type_item(char *metadata_index_filename, FILE *metadata_index_fp, struct metadata_item *metadata_item) {
  struct metadata_type *metadata_type = metadata_item->type;
  enum data_type type = metadata_type->data_type;
  uint8_t str_width;
  char *s;
  size_t fr;

  switch (type) {
    case CHAR: 
      fr = fread(&(metadata_item->data.c), sizeof(char), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;

    case INT_8: 
      fr = fread(&(metadata_item->data.b), sizeof(int8_t), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case INT_16: 
      fr = fread(&(metadata_item->data.h), sizeof(int16_t), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case INT_32: 
      fr = fread(&(metadata_item->data.i), sizeof(int32_t), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case INT_64: 
      fr = fread(&(metadata_item->data.l), sizeof(int64_t), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case FLOAT: 
      fr = fread(&(metadata_item->data.f), sizeof(float), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case DOUBLE: 
      fr = fread(&(metadata_item->data.d), sizeof(double), 1, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
      break;
      
    case STRING: 
      str_width = metadata_type->width;
      s = (char *)malloc(str_width * sizeof(char));
      if (s == NULL) {
        err(1, "malloc failure for s in fwrite_data_type_item.\n");
      }
      fr = fread(s, sizeof(char), str_width, metadata_index_fp);
      check_file_read(metadata_index_filename, metadata_index_fp, str_width, fr);
      metadata_item->data.s = s;
      break;
      
    default:
      err(1, "Unknown data_type %d.\n", type);
  }
}


struct metadata_columns *read_metadata_conf(char *metadata_conf_filename) {
  FILE *metadata_conf_fp = fopen(metadata_conf_filename, "r");
  if (metadata_conf_fp == NULL) {
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

  while ((read = getline(&line, &len, metadata_conf_fp)) != -1) {
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

  fclose(metadata_conf_fp);
  return metadata_columns;
}

void init_metadata_dat(char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *metadata_index_fp = fopen(metadata_index_filename, "wb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  int i;
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};

  if (fwrite(GIGGLE_METADATA_FILE_MARKER, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_FILE_MARKER_LENGTH) {
    err(1, "fwrite failure for file marker in init_metadata_dat.\n");
  }

  if (fwrite(GIGGLE_METADATA_VERSION_MARKER, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_VERSION_MARKER_LENGTH) {
    err(1, "fwrite failure for version marker in init_metadata_dat.\n");
  }

  if (fwrite(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index_fp) != GIGGLE_METADATA_EXTRA_LENGTH) {
    err(1, "fwrite failure for extra in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->num_cols), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_columns->num_cols in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->row_width), sizeof(uint16_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_columns->row_width in init_metadata_dat.\n");
  }

  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    
    char type_char = data_type_to_char(metadata_type->data_type);
    if (fwrite(&type_char, sizeof(char), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for type_char in init_metadata_dat.\n");
    }

    if (fwrite(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for metadata_type->width in init_metadata_dat.\n");
    }

    if (fwrite(metadata_type->name, sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp) != COLUMN_NAME_MAX_LENGTH) {
      err(1, "fwrite failure for metadata_type->name in init_metadata_dat.\n");
    }
  }

  fclose(metadata_index_fp);
}

void init_metadata_index_dat(struct metadata_index *metadata_index) {
  struct metadata_columns *metadata_columns = metadata_index->metadata_columns;
  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = fopen(metadata_index_filename, "wb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  metadata_index->metadata_index_fp = metadata_index_fp;

  int i;
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};
  metadata_index->num_rows = 0;

  if (fwrite(GIGGLE_METADATA_FILE_MARKER, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_FILE_MARKER_LENGTH) {
    err(1, "fwrite failure for file marker in init_metadata_dat.\n");
  }

  if (fwrite(GIGGLE_METADATA_VERSION_MARKER, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_VERSION_MARKER_LENGTH) {
    err(1, "fwrite failure for version marker in init_metadata_dat.\n");
  }

  if (fwrite(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index_fp) != GIGGLE_METADATA_EXTRA_LENGTH) {
    err(1, "fwrite failure for extra in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->num_cols), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_columns->num_cols in init_metadata_dat.\n");
  }

  if (fwrite(&(metadata_columns->row_width), sizeof(uint16_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_columns->row_width in init_metadata_dat.\n");
  }

  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    
    char type_char = data_type_to_char(metadata_type->data_type);
    if (fwrite(&type_char, sizeof(char), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for type_char in init_metadata_dat.\n");
    }

    if (fwrite(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for metadata_type->width in init_metadata_dat.\n");
    }

    if (fwrite(metadata_type->name, sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp) != COLUMN_NAME_MAX_LENGTH) {
      err(1, "fwrite failure for metadata_type->name in init_metadata_dat.\n");
    }
  }

  metadata_index->header_offset = ftell(metadata_index_fp);

  if (fwrite(&(metadata_index->num_rows), sizeof(uint64_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for num_rows in append_metadata_dat.\n");
  }
}

void append_metadata_dat(char *intervals_filename, char *metadata_index_filename, struct metadata_columns *metadata_columns) {
  FILE *intervals_fp = fopen(intervals_filename, "r");
  if (intervals_fp == NULL) {
    err(1, "%s not found.\n", intervals_filename);
  }
  FILE *metadata_index_fp = fopen(metadata_index_filename, "r+b");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  uint64_t num_rows = 0;

  if (fseek(metadata_index_fp, 0, SEEK_END) != 0) {
    err(1, "Could not seek to the end to append in '%s'.", metadata_index_filename);
  }
  uint64_t curr_offset = ftell(metadata_index_fp);

  if (fwrite(&num_rows, sizeof(uint64_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for num_rows in append_metadata_dat.\n");
  }

  while ((read = getline(&line, &len, intervals_fp)) != -1) {
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
      fwrite_data_type_item(metadata_index_fp, metadata_type, data);
    }

    ++num_rows;

    free(fields);
    free(kline.s);
  }

  if (line)
    free(line);

  if (fseek(metadata_index_fp, curr_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }

  if (fwrite(&num_rows, sizeof(uint64_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for num_rows in append_metadata_dat.\n");
  }

  fclose(intervals_fp);
  fclose(metadata_index_fp);
}

struct metadata_types *read_metadata_types_from_metadata_dat(char *metadata_index_filename) {
  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
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

  fr = fread(file_marker, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_FILE_MARKER_LENGTH, fr);
  if (strcmp(file_marker, GIGGLE_METADATA_FILE_MARKER) != 0) {
    err(1, "Not a GIGGLE Metadata Index file.\n");
  }

  fr = fread(version_marker, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_VERSION_MARKER_LENGTH, fr);
  if (strcmp(version_marker, GIGGLE_METADATA_VERSION_MARKER) != 0) {
    err(1, "Incompatible GIGGLE Metadata Index version.\n");
  }
  
  fr = fread(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_EXTRA_LENGTH, fr);

  fr = fread(&(metadata_types->num_cols), sizeof(uint8_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
  
  fr = fread(&(metadata_types->row_width), sizeof(uint16_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

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
    fr = fread(&type_char, sizeof(char), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
    metadata_type->data_type = type_char_to_enum(type_char);

    fr = fread(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

    fr = fread(&(metadata_type->name), sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, COLUMN_NAME_MAX_LENGTH, fr);

    khash_str2int_set(metadata_types->column_name_to_index, strdup(metadata_type->name), i);

    metadata_types->col_offsets[i] = col_offset;
    col_offset += metadata_type->width;

    metadata_types->types[i] = metadata_type;
  }
  
  fr = fread(&(metadata_types->num_rows), sizeof(uint64_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

  metadata_types->header_offset = ftell(metadata_index_fp);

  fclose(metadata_index_fp);

  return metadata_types;
}

void read_metadata_types_from_metadata_index_dat(struct metadata_index *metadata_index) {
  metadata_index->metadata_conf_filename = NULL;
  metadata_index->metadata_columns = NULL;

  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  metadata_index->metadata_index_fp = metadata_index_fp;
  
  struct metadata_types *metadata_types = (struct metadata_types *)malloc(sizeof(struct metadata_types));
  if (metadata_types == NULL) {
    err(1, "malloc failure for metadata_types in read_metadata_types_from_metadata_dat.\n");
  }
  metadata_index->metadata_types = metadata_types;

  int i;
  size_t fr;
  char file_marker[7];
  char version_marker[3];
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};
  uint16_t col_offset = 0;

  metadata_types->column_name_to_index = khash_str2int_init();

  fr = fread(file_marker, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_FILE_MARKER_LENGTH, fr);
  if (strcmp(file_marker, GIGGLE_METADATA_FILE_MARKER) != 0) {
    err(1, "Not a GIGGLE Metadata Index file.\n");
  }

  fr = fread(version_marker, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_VERSION_MARKER_LENGTH, fr);
  if (strcmp(version_marker, GIGGLE_METADATA_VERSION_MARKER) != 0) {
    err(1, "Incompatible GIGGLE Metadata Index version.\n");
  }
  
  fr = fread(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, GIGGLE_METADATA_EXTRA_LENGTH, fr);

  fr = fread(&(metadata_types->num_cols), sizeof(uint8_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
  
  fr = fread(&(metadata_types->row_width), sizeof(uint16_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

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
    fr = fread(&type_char, sizeof(char), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
    metadata_type->data_type = type_char_to_enum(type_char);

    fr = fread(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

    fr = fread(&(metadata_type->name), sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, COLUMN_NAME_MAX_LENGTH, fr);

    khash_str2int_set(metadata_types->column_name_to_index, strdup(metadata_type->name), i);

    metadata_types->col_offsets[i] = col_offset;
    col_offset += metadata_type->width;

    metadata_types->types[i] = metadata_type;
  }
  
  fr = fread(&(metadata_types->num_rows), sizeof(uint64_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

  metadata_types->header_offset = ftell(metadata_index_fp);
}

struct metadata_rows *read_metadata_rows(char *metadata_index_filename, struct metadata_types *metadata_types) {
  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  
  if (fseek(metadata_index_fp, metadata_types->header_offset, SEEK_SET) != 0) {
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

      fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);

      metadata_row->items[j] = metadata_item;
    }

    metadata_rows->rows[i] = metadata_row;
  }
  
  fclose(metadata_index_fp);

  return metadata_rows;
}

struct metadata_row *read_metadata_row(char *metadata_index_filename, struct metadata_types *metadata_types, uint64_t interval_id) {
  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  uint64_t total_offset = metadata_types->header_offset + metadata_types->row_width * interval_id;
  
  if (fseek(metadata_index_fp, total_offset, SEEK_SET) != 0) {
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

    fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);

    metadata_row->items[i] = metadata_item;
  }
  
  fclose(metadata_index_fp);

  return metadata_row;
}

struct metadata_item *read_metadata_item_by_column_id(char *metadata_index_filename, struct metadata_types *metadata_types, uint64_t interval_id, uint8_t column_id) {
  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }

  uint64_t total_offset = metadata_types->header_offset + metadata_types->row_width * interval_id + metadata_types->col_offsets[column_id];
  
  if (fseek(metadata_index_fp, total_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  size_t fr;

  struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
  if (metadata_item == NULL) {
    err(1, "malloc failure for metadata_item in read_metadata_item_by_column_id.\n");
  }
  metadata_item->type = metadata_types->types[column_id];

  fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);
  
  fclose(metadata_index_fp);

  return metadata_item;
}

struct metadata_item *read_metadata_item_by_column_name(char *metadata_index_filename, struct metadata_types *metadata_types, uint64_t interval_id, char *column_name) {
  int lookup_result, column_id;

  lookup_result = khash_str2int_get(metadata_types->column_name_to_index, column_name, &column_id);
  if (lookup_result == -1) {
    err(1, "Column %s not found in metadata.\n", column_name);
  }
  return read_metadata_item_by_column_id(metadata_index_filename, metadata_types, interval_id, column_id);
}

struct metadata_index *metadata_index_init(char *metadata_conf_filename, char *metadata_index_filename) {
  if (metadata_conf_filename == NULL) {
    err(1, "metadata_conf_filename cannot be NULL.\n");
  }
  if (metadata_index_filename == NULL) {
    err(1, "metadata_index_filename cannot be NULL.\n");
  }
  struct metadata_index *metadata_index = (struct metadata_index *)malloc(sizeof(struct metadata_index));
  if (metadata_index == NULL) {
    err(1, "malloc failure for metadata_index in metadata_index_init.\n");
  }
  metadata_index->metadata_conf_filename = strdup(metadata_conf_filename);
  metadata_index->metadata_index_filename = strdup(metadata_index_filename);

  metadata_index->metadata_types = NULL;

  // Read metadata.conf
  metadata_index->metadata_columns = read_metadata_conf(metadata_conf_filename);

  // Write header in metadata_index.dat
  init_metadata_index_dat(metadata_index);

  return metadata_index;
}

uint64_t metadata_index_add(struct metadata_index *metadata_index, uint32_t file_id, kstring_t *line) {
  struct metadata_columns *metadata_columns = metadata_index->metadata_columns;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;
  int fields_length;
  int *fields;
  fields = ksplit(line, '\t', &fields_length);

  // TODO: Write file_id before metadata columns

  for (int i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    int column = metadata_column->column;
    struct metadata_type *metadata_type = metadata_column->type;
    char *data = line->s + fields[column - 1];
    fwrite_data_type_item(metadata_index_fp, metadata_type, data);
  }

  free(fields);
  metadata_index->num_rows++;
}

void metadata_index_store(struct metadata_index *metadata_index) {
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;

  if (fseek(metadata_index_fp, metadata_index->header_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index->metadata_index_filename);
  }

  if (fwrite(&(metadata_index->num_rows), sizeof(uint64_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for num_rows in metadata_index_store.\n");
  }
}

struct metadata_index *metadata_index_load(char *metadata_index_filename) {
  if (metadata_index_filename == NULL) {
    err(1, "metadata_index_filename cannot be NULL.\n");
  }
  struct metadata_index *metadata_index = (struct metadata_index *)malloc(sizeof(struct metadata_index));
  if (metadata_index == NULL) {
    err(1, "malloc failure for metadata_index in metadata_index_init.\n");
  }
  metadata_index->metadata_index_filename = strdup(metadata_index_filename);

  // Read metadata_types from metadata_index.dat
  read_metadata_types_from_metadata_index_dat(metadata_index);

  return metadata_index;
}

void metadata_columns_destroy(struct metadata_columns *metadata_columns) {
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

void metadata_types_destroy(struct metadata_types *metadata_types) {
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

void metadata_item_destroy(struct metadata_item *metadata_item) {
  if (metadata_item->type->data_type == STRING) {
    free(metadata_item->data.s);
  }
  free(metadata_item);
}

void metadata_row_destroy(struct metadata_row *metadata_row) {
  int i;
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    metadata_item_destroy(metadata_item);
  }
  free(metadata_row->items);
  free(metadata_row);
}

void metadata_rows_destroy(struct metadata_rows *metadata_rows) {
  int i;
  for (i = 0; i < metadata_rows->num; ++i) {
    metadata_row_destroy(metadata_rows->rows[i]);
  }
  free(metadata_rows->rows);
  free(metadata_rows);
}

void metadata_index_destroy(struct metadata_index **metadata_index) {
  if ((*metadata_index)->metadata_conf_filename) {
    free((*metadata_index)->metadata_conf_filename);
  }
  free((*metadata_index)->metadata_index_filename);
  if ((*metadata_index)->metadata_columns) {
    metadata_columns_destroy((*metadata_index)->metadata_columns);
  }
  if ((*metadata_index)->metadata_types) {
    metadata_types_destroy((*metadata_index)->metadata_types);
  }
  fclose((*metadata_index)->metadata_index_fp);
  free(*metadata_index);
  *metadata_index = NULL;
}

void print_metadata_columns(struct metadata_columns *metadata_columns) {
  int i;
  printf("metadata_columns => num_cols: %d, row_width: %d\n", metadata_columns->num_cols, metadata_columns->row_width);
  for (i = 0; i < metadata_columns->num_cols; ++i) {
    struct metadata_column *metadata_column = metadata_columns->columns[i];
    struct metadata_type *metadata_type = metadata_column->type;
    printf("%d => column: %d, data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_column->column, metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void print_metadata_types(struct metadata_types *metadata_types) {
  int i;
  printf("metadata_types => num_cols: %d, num_rows: %lu, row_width: %d, header_offset: %lu\n", metadata_types->num_cols, metadata_types->num_rows, metadata_types->row_width, metadata_types->header_offset);
  for (i = 0; i < metadata_types->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_types->types[i];
    printf("%d => data_type: %d, type_char: %c, name: %s, width: %d\n", i,metadata_type->data_type, data_type_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
  }
}

void print_metadata_data(struct metadata_type *type, union metadata_data data) {
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

void print_metadata_item(struct metadata_item *metadata_item) {
  print_metadata_data(metadata_item->type, metadata_item->data);
  printf("\n");
}

void print_metadata_row(struct metadata_row *metadata_row) {
  int i;
  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = metadata_row->items[i];
    print_metadata_data(metadata_item->type, metadata_item->data);
    printf(", ");
  }
  printf("\n");
}

void print_metadata_rows(struct metadata_rows *metadata_rows) {
  int i, j;
  printf("metadata_rows => num_rows: %lu\n", metadata_rows->num);
  for (i = 0; i < metadata_rows->num; ++i) {
    struct metadata_row *metadata_row = metadata_rows->rows[i];
    printf("metadata_row %d => ", i);
    print_metadata_row(metadata_row);
  }
}