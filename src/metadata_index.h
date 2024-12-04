#ifndef __METADATA_INDEX_H__
#define __METADATA_INDEX_H__

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <err.h>
#include <sysexits.h>
#include <htslib/kstring.h>

#define COLUMN_NAME_MAX_LENGTH 32
#define GIGGLE_METADATA_FILE_MARKER_LENGTH 7
#define GIGGLE_METADATA_FILE_MARKER "GIGLMET"
#define GIGGLE_METADATA_VERSION_MARKER_LENGTH 3
#define GIGGLE_METADATA_VERSION_MARKER "000"
#define GIGGLE_METADATA_EXTRA_LENGTH 6

extern char *METADATA_INDEX_FILE_NAME;

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

struct metadata_index {
  // for storing the column numbers in the conf file, 
  // only used by the operations used in indexing- init, add 
  uint8_t *columns; 

  char *metadata_index_filename;
  FILE *metadata_index_fp;
  
  uint8_t num_cols;
  uint16_t row_width; // total width of each data row
  uint64_t num_rows;

  void *column_name_to_index; // khash_str2int hashmap to map column names to column indexes
  uint16_t *col_offsets; // offset of ith column in a data row

  struct metadata_type **types;
  uint64_t header_offset; // total header offset, end of the header file position
};

enum data_type data_type_string_to_enum(char type_string[8]);
char data_type_enum_to_char(enum data_type type);
enum data_type data_type_char_to_enum(char type_char);
uint8_t get_width_of_data_type(enum data_type type);

char *safe_sscanf(uint8_t str_width, char *data);

void fwrite_data_type_item(FILE *metadata_index_fp, struct metadata_type *metadata_type, char *data);
void fread_data_type_item(char *metadata_index_filename, FILE *metadata_index_fp, struct metadata_item *metadata_item);

struct metadata_index *metadata_index_new();
void read_metadata_conf(struct metadata_index *metadata_index, char *metadata_conf_filename);
void write_metadata_index_header(struct metadata_index *metadata_index);
void read_metadata_index_header(struct metadata_index *metadata_index);

void print_metadata_index(struct metadata_index *metadata_index);
void print_metadata_data(struct metadata_type *type, union metadata_data data);
void print_metadata_item(struct metadata_item *metadata_item);
void print_metadata_row(struct metadata_row *metadata_row);
void print_metadata_rows(struct metadata_rows *metadata_rows);

// public interfaces

struct metadata_index *metadata_index_init(char *metadata_conf_filename, char *metadata_index_filename);
uint64_t metadata_index_add(struct metadata_index *metadata_index, uint32_t file_id, kstring_t *line);
void metadata_index_store(struct metadata_index *metadata_index);
struct metadata_index *metadata_index_load(char *metadata_index_filename);
void metadata_index_destroy(struct metadata_index **metadata_index_ptr);

struct metadata_rows *read_metadata_rows(struct metadata_index *metadata_index);
void metadata_rows_destroy(struct metadata_rows *metadata_rows);

struct metadata_row *read_metadata_row(struct metadata_index *metadata_index, uint64_t interval_id);
void metadata_row_destroy(struct metadata_row *metadata_row);

struct metadata_item *read_metadata_item_by_column_id(struct metadata_index *metadata_index, uint64_t interval_id, uint8_t column_id);
struct metadata_item *read_metadata_item_by_column_name(struct metadata_index *metadata_index, uint64_t interval_id, char *column_name);
void metadata_item_destroy(struct metadata_item *metadata_item);

#endif
