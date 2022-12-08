#ifndef __QUERY_FILTER_H__
#define __QUERY_FILTER_H__

#include <stdint.h>
#include "metadata_index.h"

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

union metadata_data parse_query_filter_data_string(struct metadata_type *metadata_type, char *data);
enum comparison comparison_string_to_enum(char *query_filter_string, int *start_comparison, int *end_comparison);

int perform_metadata_comparison(struct metadata_type *metadata_type, enum comparison comparison, union metadata_data source, union metadata_data target);

void print_comparison(enum comparison comparison);
void print_query_filter(struct query_filter *query_filter);

// public interfaces

struct query_filter *parse_query_filter_string(struct metadata_index *metadata_index, char *query_filter_string);
int filter_metadata_row_by_item(struct metadata_item *metadata_item, struct query_filter *query_filter);  
int filter_metadata_row_by_row(struct metadata_row *metadata_row, struct query_filter *query_filter);  
void query_filter_destroy(struct query_filter **query_filter_ptr);

#endif
