#include <stdio.h>
#include "metadata_index.h"
#include "query_filter.h"

void metadata_index_add_all_intervals_from_file(struct metadata_index *metadata_index, uint32_t file_id, char *intervals_filename) {
  struct metadata_columns *metadata_columns = metadata_index->metadata_columns;
  FILE *intervals = fopen(intervals_filename, "r");
  if (intervals == NULL) {
    err(1, "%s not found.\n", intervals_filename);
  }
  
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, intervals)) != -1) {
    // printf("Retrieved line of length %zu:\n%s\n", read, line);
    kstring_t kline = {0, 0, NULL};
    kputs(line, &kline);
    metadata_index_add(metadata_index, file_id, &kline);
    free(kline.s);
  }

  if (line)
    free(line);

  fclose(intervals);
}

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index.dat";

  // A. Index

  // 1. metadata_index_init
  struct metadata_index *metadata_index = metadata_index_init(metadata_conf_filename, metadata_index_filename);
  printf("\nInitialized Metadata Index in %s\n", metadata_index_filename);
  print_metadata_columns(metadata_index->metadata_columns);
  
  // 2. metadata_index_add 
  char *intervals_filename1 = "intervals1.tsv";
  uint32_t file_id1 = 5; 
  metadata_index_add_all_intervals_from_file(metadata_index, file_id1, intervals_filename1);
  printf("\nAppended Metadata from %s to %s\n", intervals_filename1, metadata_index_filename);
  
  // 3. metadata_index_store
  metadata_index_store(metadata_index);
  printf("\nStored Metadata Index in %s\n", metadata_index_filename);

  // 4. metadata_index_destroy
  metadata_index_destroy(&metadata_index);
  printf("\nDestroyed Metadata Index\n");

  // B. Search

  // 1. metadata_index_load
  metadata_index = metadata_index_load(metadata_index_filename);
  printf("\nLoaded Metadata Index from %s\n", metadata_index_filename);
  print_metadata_types(metadata_index->metadata_types);
  
  // 2.i. Read metadata rows from metadata_index.dat
  struct metadata_rows *metadata_rows = read_metadata_rows(metadata_index_filename, metadata_index->metadata_types);
  printf("\nRead metadata_rows from %s\n", metadata_index_filename);
  print_metadata_rows(metadata_rows);

  // 2.ii. Read ith metadata_row
  struct metadata_row *metadata_row_1 = read_metadata_row(metadata_index_filename, metadata_index->metadata_types, 1);
  struct metadata_row *metadata_row_3 = read_metadata_row(metadata_index_filename, metadata_index->metadata_types, 3);
  printf("\nRead metadata_rows 1 and 3 from %s\n", metadata_index_filename);
  printf("metadata_row 1 => ");
  print_metadata_row(metadata_row_1);  
  printf("metadata_row 3 => ");
  print_metadata_row(metadata_row_3);

  // 2.iii. Read ith interval's jth metadata column from metadata_index.dat
  struct metadata_item *metadata_row_0_col_2 = read_metadata_item_by_column_id(metadata_index_filename, metadata_index->metadata_types, 0, 2);
  struct metadata_item *metadata_row_2_score = read_metadata_item_by_column_name(metadata_index_filename, metadata_index->metadata_types, 2, "score");
  printf("\nRead metadata_items metadata_row_0_col_2 and metadata_row_2_score from %s\n", metadata_index_filename);
  printf("metadata_row_0_col_2 => ");
  print_metadata_item(metadata_row_0_col_2);  
  printf("metadata_row_2_score => ");
  print_metadata_item(metadata_row_2_score);

  // 4. Read query filter
  char query_filter_string_1[] = "feature<my_feature";
  char query_filter_string_2[] = "score>=456.5";
  struct query_filter *query_filter_1 = parse_query_filter_string(metadata_index->metadata_types, query_filter_string_1);
  struct query_filter *query_filter_2 = parse_query_filter_string(metadata_index->metadata_types, query_filter_string_2);
  printf("\nParsed query filter string: %s\n", query_filter_string_1);
  print_query_filter(query_filter_1);
  printf("\nParsed query filter string: %s\n", query_filter_string_2);
  print_query_filter(query_filter_2);

  // 5.i. Filter rows using query filters
  int row_1_filter_1 = filter_metadata_row_by_row(metadata_row_1, query_filter_1);
  int row_1_filter_2 = filter_metadata_row_by_row(metadata_row_1, query_filter_2);
  int row_3_filter_1 = filter_metadata_row_by_row(metadata_row_3, query_filter_1);
  int row_3_filter_2 = filter_metadata_row_by_row(metadata_row_3, query_filter_2);
  printf("\nFiltered rows using query filters\n");
  printf("row_1_filter_1: %d, row_1_filter_2: %d\n", row_1_filter_1, row_1_filter_2);
  printf("row_3_filter_1: %d, row_3_filter_2: %d\n", row_3_filter_1, row_3_filter_2);

  // 5.ii. Filter items using query filters
  int row_0_col_2 = filter_metadata_row_by_item(metadata_row_0_col_2, query_filter_1);
  int row_2_score = filter_metadata_row_by_item(metadata_row_2_score, query_filter_2);
  printf("\nFiltered items using query filters\n");
  printf("row_0_col_2: %d\n", row_0_col_2);
  printf("row_2_score: %d\n", row_2_score);

  // 6. metadata_index_destroy
  metadata_index_destroy(&metadata_index);
  query_filter_destroy(query_filter_1);
  query_filter_destroy(query_filter_2);
  metadata_item_destroy(metadata_row_0_col_2);
  metadata_item_destroy(metadata_row_2_score);
  metadata_row_destroy(metadata_row_1);
  metadata_row_destroy(metadata_row_3);
  metadata_rows_destroy(metadata_rows);

  return 0;
}
