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

struct query_filter *parse_query_filter_string(struct metadata_types *metadata_types, char *query_filter_string);
int perform_metadata_comparison(struct metadata_type *metadata_type, enum comparison comparison, union metadata_data source, union metadata_data target);
int filter_metadata_row_by_item(struct metadata_item *metadata_item, struct query_filter *query_filter);  
int filter_metadata_row_by_row(struct metadata_row *metadata_row, struct query_filter *query_filter);  

void free_query_filter(struct query_filter *query_filter);

void display_comparison(enum comparison comparison);
void display_query_filter(struct query_filter *query_filter);

#endif
