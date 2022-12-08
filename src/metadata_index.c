#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <htslib/khash_str2int.h>
#include "util.h"
#include "metadata_index.h"

char *METADATA_INDEX_FILE_NAME = "metadata_index.dat";

enum data_type data_type_string_to_enum(char type_string[8]) {
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

char data_type_enum_to_char(enum data_type type) {
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

enum data_type data_type_char_to_enum(char type_char) {
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

uint8_t get_width_of_data_type(enum data_type type) {
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

// this function is not responsible for
// freeing the allocated memory for the string
char *safe_sscanf(uint8_t str_width, char *data) {
  char s_format[6]; // '%' + max value for int8_t + 's' + NULL -> '%255sN'
  char *s;
  snprintf(s_format, sizeof(s_format), "%%%ds", str_width - 1);
  s = (char *)calloc(str_width, sizeof(char));
  if (s == NULL) {
    err(1, "calloc failure for s in safe_sscanf.\n");
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

struct metadata_index *metadata_index_new() {
  struct metadata_index *metadata_index = (struct metadata_index *)malloc(sizeof(struct metadata_index));
  if (metadata_index == NULL) {
    err(1, "malloc failure for metadata_index in metadata_index_new.\n");
  }

  metadata_index->columns = NULL;
  metadata_index->metadata_index_filename = NULL;
  metadata_index->metadata_index_fp = NULL;
  metadata_index->column_name_to_index = NULL;
  metadata_index->col_offsets = NULL;
  metadata_index->types = NULL;

  metadata_index->num_cols = -1;
  metadata_index->row_width = -1;
  metadata_index->num_rows = -1;
  metadata_index->header_offset = -1;

  return metadata_index;
}

void read_metadata_conf(struct metadata_index *metadata_index, char *metadata_conf_filename) {
  FILE *metadata_conf_fp = fopen(metadata_conf_filename, "r");
  if (metadata_conf_fp == NULL) {
    err(1, "%s not found.\n", metadata_conf_filename);
  }

  uint8_t num_cols = 0;
  uint16_t col_offset = 0;

  metadata_index->types = (struct metadata_type **)malloc(255 * sizeof(struct metadata_type*));
  if (metadata_index->types == NULL) {
    err(1, "malloc failure for metadata_index->types in read_metadata_conf.\n");
  }
  metadata_index->columns = (uint8_t *)malloc(255 * sizeof(uint8_t));
  if (metadata_index->columns == NULL) {
    err(1, "malloc failure for metadata_index->columns in read_metadata_conf.\n");
  }
  metadata_index->col_offsets = (uint16_t *)malloc(255 * sizeof(uint16_t));
  if (metadata_index->col_offsets == NULL) {
    err(1, "malloc failure for metadata_index->col_offsets in read_metadata_conf.\n");
  }

  metadata_index->column_name_to_index = khash_str2int_init();

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, metadata_conf_fp)) != -1) {
    // printf("Retrieved line of length %zu:\n%s\n", read, line);

    uint8_t column;
    char name[COLUMN_NAME_MAX_LENGTH] = {0};
    char type_string[8];
    uint8_t str_len = 0;

    // 31 in %31s = COLUMN_NAME_MAX_LENGTH - 1
    sscanf(line, "%hhu %31s %7s %hhu", &column, name, type_string, &str_len); 
    // printf("%d %s %s %d\n", column, name, type_string, str_len);

    if (num_cols == 255) {
      err(1, "Cannot store more than 255 columns.\n");
    }
    
    if (str_len == 255) {
      err(1, "Column '%s': string length cannot be more than 254.\n", name);
    }

    if (khash_str2int_has_key(metadata_index->column_name_to_index, name)) {
      err(1, "Cannot allow duplicate column '%s'.\n", name);
    } else {
      khash_str2int_set(metadata_index->column_name_to_index, strdup(name), num_cols);
    }

    struct metadata_type *metadata_type = (struct metadata_type *)calloc(1, sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      err(1, "calloc failure for metadata_type in read_metadata_conf.\n");
    }

    memcpy(metadata_type->name, name, COLUMN_NAME_MAX_LENGTH);
    metadata_type->data_type = data_type_string_to_enum(type_string);
    metadata_type->width += get_width_of_data_type(metadata_type->data_type);
    if (metadata_type->data_type == STRING) {
      metadata_type->width = str_len + 1;
    }

    metadata_index->col_offsets[num_cols] = col_offset;
    col_offset += metadata_type->width;

    metadata_index->types[num_cols] = metadata_type;
    metadata_index->columns[num_cols] = column;
    num_cols++;
  }

  if (num_cols < 255) {
    metadata_index->types = (struct metadata_type **)realloc(metadata_index->types, sizeof(struct metadata_type*) * num_cols);
    if (metadata_index->types == NULL) {
      err(1, "realloc failure for metadata_index->types in read_metadata_conf.\n");
    }
    metadata_index->columns = (uint8_t *)realloc(metadata_index->columns, sizeof(uint8_t) * num_cols);
    if (metadata_index->columns == NULL) {
      err(1, "realloc failure for metadata_index->columns in read_metadata_conf.\n");
    }
    metadata_index->col_offsets = (uint16_t *)realloc(metadata_index->col_offsets, sizeof(uint16_t) * num_cols);
    if (metadata_index->col_offsets == NULL) {
      err(1, "realloc failure for metadata_index->col_offsets in read_metadata_conf.\n");
    }
  }

  metadata_index->num_cols = num_cols;
  metadata_index->row_width = col_offset;

  if (line)
    free(line);

  fclose(metadata_conf_fp);
}

void write_metadata_index_header(struct metadata_index *metadata_index) {
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;
  
  int i;
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};

  if (fwrite(GIGGLE_METADATA_FILE_MARKER, sizeof(char), GIGGLE_METADATA_FILE_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_FILE_MARKER_LENGTH) {
    err(1, "fwrite failure for file marker in write_metadata_index_header.\n");
  }

  if (fwrite(GIGGLE_METADATA_VERSION_MARKER, sizeof(char), GIGGLE_METADATA_VERSION_MARKER_LENGTH, metadata_index_fp) != GIGGLE_METADATA_VERSION_MARKER_LENGTH) {
    err(1, "fwrite failure for version marker in write_metadata_index_header.\n");
  }

  if (fwrite(extra, sizeof(char), GIGGLE_METADATA_EXTRA_LENGTH, metadata_index_fp) != GIGGLE_METADATA_EXTRA_LENGTH) {
    err(1, "fwrite failure for extra in write_metadata_index_header.\n");
  }

  if (fwrite(&(metadata_index->num_cols), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_index->num_cols in write_metadata_index_header.\n");
  }

  if (fwrite(&(metadata_index->row_width), sizeof(uint16_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for metadata_index->row_width in write_metadata_index_header.\n");
  }

  for (i = 0; i < metadata_index->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_index->types[i];
    
    char type_char = data_type_enum_to_char(metadata_type->data_type);
    if (fwrite(&type_char, sizeof(char), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for type_char in write_metadata_index_header.\n");
    }

    if (fwrite(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp) != 1) {
      err(1, "fwrite failure for metadata_type->width in write_metadata_index_header.\n");
    }

    if (fwrite(metadata_type->name, sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp) != COLUMN_NAME_MAX_LENGTH) {
      err(1, "fwrite failure for metadata_type->name in write_metadata_index_header.\n");
    }
  }

  if (fwrite(&(metadata_index->num_rows), sizeof(uint64_t), 1, metadata_index_fp) != 1) {
    err(1, "fwrite failure for num_rows in write_metadata_index_header.\n");
  }

  metadata_index->header_offset = ftell(metadata_index_fp);
}

void read_metadata_index_header(struct metadata_index *metadata_index) {  
  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;

  int i;
  size_t fr;
  char file_marker[GIGGLE_METADATA_FILE_MARKER_LENGTH];
  char version_marker[GIGGLE_METADATA_VERSION_MARKER_LENGTH];
  char extra[GIGGLE_METADATA_EXTRA_LENGTH] = {0};
  uint16_t col_offset = 0;

  metadata_index->column_name_to_index = khash_str2int_init();

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

  fr = fread(&(metadata_index->num_cols), sizeof(uint8_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
  
  fr = fread(&(metadata_index->row_width), sizeof(uint16_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

  metadata_index->col_offsets = (uint16_t *)malloc(metadata_index->num_cols * sizeof(uint16_t));
  if (metadata_index->col_offsets == NULL) {
    err(1, "malloc failure for metadata_index->col_offsets in read_metadata_index_header.\n");
  }
  
  metadata_index->types = (struct metadata_type **)malloc(metadata_index->num_cols * sizeof(struct metadata_type*));
  if (metadata_index->types == NULL) {
    err(1, "malloc failure for metadata_index->types in read_metadata_index_header.\n");
  }

  for (i = 0; i < metadata_index->num_cols; ++i) {
    struct metadata_type *metadata_type = (struct metadata_type *)calloc(1, sizeof(struct metadata_type));
    if (metadata_type == NULL) {
      err(1, "calloc failure for metadata_type in read_metadata_index_header.\n");
    }

    char type_char;
    fr = fread(&type_char, sizeof(char), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);
    metadata_type->data_type = data_type_char_to_enum(type_char);

    fr = fread(&(metadata_type->width), sizeof(uint8_t), 1, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

    fr = fread(&(metadata_type->name), sizeof(char), COLUMN_NAME_MAX_LENGTH, metadata_index_fp);
    check_file_read(metadata_index_filename, metadata_index_fp, COLUMN_NAME_MAX_LENGTH, fr);

    khash_str2int_set(metadata_index->column_name_to_index, strdup(metadata_type->name), i);

    metadata_index->col_offsets[i] = col_offset;
    col_offset += metadata_type->width;

    metadata_index->types[i] = metadata_type;
  }
  
  fr = fread(&(metadata_index->num_rows), sizeof(uint64_t), 1, metadata_index_fp);
  check_file_read(metadata_index_filename, metadata_index_fp, 1, fr);

  metadata_index->header_offset = ftell(metadata_index_fp);
}

struct metadata_index *metadata_index_init(char *metadata_conf_filename, char *metadata_index_filename) {
  if (metadata_conf_filename == NULL) {
    err(1, "metadata_conf_filename cannot be NULL.\n");
  }
  if (metadata_index_filename == NULL) {
    err(1, "metadata_index_filename cannot be NULL.\n");
  }

  struct metadata_index *metadata_index = metadata_index_new();
  metadata_index->metadata_index_filename = strdup(metadata_index_filename);

  metadata_index->num_rows = 0;

  // Read metadata.conf
  read_metadata_conf(metadata_index, metadata_conf_filename);

  FILE *metadata_index_fp = fopen(metadata_index_filename, "wb+");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  metadata_index->metadata_index_fp = metadata_index_fp;

  // Write header in metadata_index.dat
  write_metadata_index_header(metadata_index);

  return metadata_index;
}

uint64_t metadata_index_add(struct metadata_index *metadata_index, uint32_t file_id, kstring_t *line) {
  uint8_t *columns = metadata_index->columns;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;
  int fields_length;
  int *fields;
  fields = ksplit(line, '\t', &fields_length);

  // TODO: Write file_id before metadata columns

  for (int i = 0; i < metadata_index->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_index->types[i];
    int column = columns[i];
    char *data = line->s + fields[column - 1];
    fwrite_data_type_item(metadata_index_fp, metadata_type, data);
  }

  free(fields);
  return metadata_index->num_rows++;
}

void metadata_index_store(struct metadata_index *metadata_index) {
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;

  uint64_t total_offset = metadata_index->header_offset - sizeof(uint64_t);

  if (fseek(metadata_index_fp, total_offset, SEEK_SET) != 0) {
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
  struct metadata_index *metadata_index = metadata_index_new();
  metadata_index->metadata_index_filename = strdup(metadata_index_filename);

  FILE *metadata_index_fp = fopen(metadata_index_filename, "rb");
  if (metadata_index_fp == NULL) {
    err(1, "%s not found.\n", metadata_index_filename);
  }
  metadata_index->metadata_index_fp = metadata_index_fp;
  
  // Read header from metadata_index.dat
  read_metadata_index_header(metadata_index);

  return metadata_index;
}

struct metadata_rows *read_metadata_rows(struct metadata_index *metadata_index) {
  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;
  
  if (fseek(metadata_index_fp, metadata_index->header_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  struct metadata_rows *metadata_rows = (struct metadata_rows *)malloc(sizeof(struct metadata_rows));
  if (metadata_rows == NULL) {
    err(1, "malloc failure for metadata_rows in read_metadata_rows.\n");
  }

  metadata_rows->num = metadata_index->num_rows;

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

    metadata_row->num = metadata_index->num_cols;
    
    metadata_row->items = (struct metadata_item **)malloc(metadata_row->num * sizeof(struct metadata_item*));
    if (metadata_row->items == NULL) {
      err(1, "malloc failure for metadata_row->items in read_metadata_rows.\n");
    }

    for (j = 0; j < metadata_row->num; ++j) {
      struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
      if (metadata_item == NULL) {
        err(1, "malloc failure for metadata_item in read_metadata_rows.\n");
      }
      metadata_item->type = metadata_index->types[j];

      fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);

      metadata_row->items[j] = metadata_item;
    }

    metadata_rows->rows[i] = metadata_row;
  }

  return metadata_rows;
}

struct metadata_row *read_metadata_row(struct metadata_index *metadata_index, uint64_t interval_id) {
  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;

  uint64_t total_offset = metadata_index->header_offset + metadata_index->row_width * interval_id;
  
  if (fseek(metadata_index_fp, total_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  int i;
  size_t fr, data_width;

  struct metadata_row *metadata_row = (struct metadata_row *)malloc(sizeof(struct metadata_row));
  if (metadata_row == NULL) {
    err(1, "malloc failure for metadata_row in read_metadata_row.\n");
  }

  metadata_row->num = metadata_index->num_cols;
  
  metadata_row->items = (struct metadata_item **)malloc(metadata_row->num * sizeof(struct metadata_item*));
  if (metadata_row->items == NULL) {
    err(1, "malloc failure for metadata_row->items in read_metadata_row.\n");
  }

  for (i = 0; i < metadata_row->num; ++i) {
    struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
    if (metadata_item == NULL) {
      err(1, "malloc failure for metadata_item in read_metadata_row.\n");
    }
    metadata_item->type = metadata_index->types[i];

    fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);

    metadata_row->items[i] = metadata_item;
  }

  return metadata_row;
}

struct metadata_item *read_metadata_item_by_column_id(struct metadata_index *metadata_index, uint64_t interval_id, uint8_t column_id) {
  char *metadata_index_filename = metadata_index->metadata_index_filename;
  FILE *metadata_index_fp = metadata_index->metadata_index_fp;

  uint64_t total_offset = metadata_index->header_offset + metadata_index->row_width * interval_id + metadata_index->col_offsets[column_id];
  
  if (fseek(metadata_index_fp, total_offset, SEEK_SET) != 0) {
    err(1, "Could not seek to metadata start in '%s'.", metadata_index_filename);
  }
  
  size_t fr;

  struct metadata_item *metadata_item = (struct metadata_item *)malloc(sizeof(struct metadata_item));
  if (metadata_item == NULL) {
    err(1, "malloc failure for metadata_item in read_metadata_item_by_column_id.\n");
  }
  metadata_item->type = metadata_index->types[column_id];

  fread_data_type_item(metadata_index_filename, metadata_index_fp, metadata_item);
  
  return metadata_item;
}

struct metadata_item *read_metadata_item_by_column_name(struct metadata_index *metadata_index, uint64_t interval_id, char *column_name) {
  int lookup_result, column_id;

  lookup_result = khash_str2int_get(metadata_index->column_name_to_index, column_name, &column_id);
  if (lookup_result == -1) {
    err(1, "Column %s not found in metadata.\n", column_name);
  }
  return read_metadata_item_by_column_id(metadata_index, interval_id, column_id);
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

void metadata_index_destroy(struct metadata_index **metadata_index_ptr) {
  struct metadata_index *metadata_index = *metadata_index_ptr;
  int i;

  free(metadata_index->columns);

  free(metadata_index->metadata_index_filename);

  for (i = 0; i < metadata_index->num_cols; ++i) {
    free(metadata_index->types[i]);
  }
  free(metadata_index->types);

  free(metadata_index->col_offsets);
  khash_str2int_destroy_free(metadata_index->column_name_to_index);

  fclose(metadata_index->metadata_index_fp);
  free(metadata_index);

  *metadata_index_ptr = NULL;
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

void print_metadata_index(struct metadata_index *metadata_index) {
  int i;
  uint8_t *columns = metadata_index->columns;
  printf("metadata_index => num_cols: %d, num_rows: %lu, row_width: %d, header_offset: %lu\n", metadata_index->num_cols, metadata_index->num_rows, metadata_index->row_width, metadata_index->header_offset);
  for (i = 0; i < metadata_index->num_cols; ++i) {
    struct metadata_type *metadata_type = metadata_index->types[i];
    if (columns) {
      printf("%d => column: %d, data_type: %d, type_char: %c, name: %s, width: %d\n", i, columns[i], metadata_type->data_type, data_type_enum_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
    } else {
      printf("%d => data_type: %d, type_char: %c, name: %s, width: %d\n", i, metadata_type->data_type, data_type_enum_to_char(metadata_type->data_type), metadata_type->name, metadata_type->width);
    }
  }
}
