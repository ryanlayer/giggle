#include <stdio.h>
#include "metadata_index.h"
#include "query_filter.h"

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index1.dat";
  char *intervals_filename = "intervals1.tsv";

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
  printf("metadata_row 1 => ");
  display_metadata_row(metadata_row_1);  
  printf("metadata_row 3 => ");
  display_metadata_row(metadata_row_3);

  // 7. Read ith interval's jth metadata column from metadata_index.dat
  struct metadata_item *metadata_row_0_col_2 = read_metadata_item_by_column_id(metadata_index_filename, metadata_types, 0, 2);
  struct metadata_item *metadata_row_2_score = read_metadata_item_by_column_name(metadata_index_filename, metadata_types, 2, "score");
  printf("\nRead metadata_items metadata_row_0_col_2 and metadata_row_2_score from %s\n", metadata_index_filename);
  printf("metadata_row_0_col_2 => ");
  display_metadata_item(metadata_row_0_col_2);  
  printf("metadata_row_2_score => ");
  display_metadata_item(metadata_row_2_score);

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
  int row_2_score = filter_metadata_row_by_item(metadata_row_2_score, query_filter_2);
  printf("\nFiltered items using query filters\n");
  printf("row_0_col_2: %d\n", row_0_col_2);
  printf("row_2_score: %d\n", row_2_score);

  // free memory
  free_query_filter(query_filter_1);
  free_query_filter(query_filter_2);
  free_metadata_item(metadata_row_0_col_2);
  free_metadata_item(metadata_row_2_score);
  free_metadata_row(metadata_row_1);
  free_metadata_row(metadata_row_3);
  free_metadata_rows(metadata_rows);
  free_metadata_types(metadata_types);
  free_metadata_columns(metadata_columns);

  return 0;
}
