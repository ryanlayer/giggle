#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <htslib/khash_str2int.h>
#include "query_filter.h"

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

struct query_filter *parse_query_filter_string(struct metadata_index *metadata_index, char *query_filter_string_original) {
  struct metadata_types *metadata_types = metadata_index->metadata_types;
  struct query_filter *query_filter = (struct query_filter *)malloc(sizeof(struct query_filter));
  if (query_filter == NULL) {
    err(1, "malloc failure for query_filter in parse_query_filter_string.\n");
  }

  char *query_filter_string = strdup(query_filter_string_original);

  int start_comparison, end_comparison, lookup_result;
  char *data_string;
  char null_tmp;
  int column_id;

  query_filter->comparison = comparison_string_to_enum(query_filter_string, &start_comparison, &end_comparison);
  
  null_tmp = query_filter_string[start_comparison];
  query_filter_string[start_comparison] = 0; // null-terminate the column name after parsing comparison operator

  lookup_result = khash_str2int_get(metadata_types->column_name_to_index, query_filter_string, &column_id);
  if (lookup_result == -1) {
    err(1, "Column %s not found in metadata.\n", query_filter_string);
  }

  query_filter->type = metadata_types->types[column_id];
  query_filter->column_id = column_id;

  data_string = query_filter_string + end_comparison + 1;
  query_filter->data = parse_query_filter_data_string(query_filter->type, data_string);
  
  free(query_filter_string);

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
      // note: float equality comparison may not always work as expected
      // due to rounding errors 
      less = (f < 0); equal = (f == 0); greater = (f > 0);
      break;
    case DOUBLE: 
      d = source.d - target.d;
      // note: double equality comparison may not always work as expected
      // due to rounding errors 
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
  struct metadata_type *item_metadata_type = metadata_item->type;
  if (strcmp(metadata_type->name, item_metadata_type->name) != 0) {
    err(1, "The query filter metadata type '%s' does not match the item metadata type '%s'.\n", metadata_type->name, item_metadata_type->name);
  }
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

void query_filter_destroy(struct query_filter *query_filter) {
  if (query_filter->type->data_type == STRING) {
    free(query_filter->data.s);
  }
  free(query_filter);
}

void print_comparison(enum comparison comparison) {
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

void print_query_filter(struct query_filter *query_filter) {
  int i;
  print_metadata_data(query_filter->type, query_filter->data);
  printf(", comparison: ");
  print_comparison(query_filter->comparison);
  printf("\n");
}
