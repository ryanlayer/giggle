#include <stdio.h>
#include "metadata_index.h"

void metadata_intervals_metadata_index_add(struct metadata_index *metadata_index, uint32_t file_id, char *intervals_filename) {
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
  char *metadata_index_filename = "metadata_index2.dat";

  // 1. metadata_index_init
  struct metadata_index *metadata_index = metadata_index_init(metadata_conf_filename, metadata_index_filename);
  
  // 2. metadata_index_add 
  char *intervals_filename1 = "intervals1.tsv";
  uint32_t file_id1 = 5; 
  metadata_intervals_metadata_index_add(metadata_index, file_id1, intervals_filename1);
  
  // 3. metadata_index_store
  metadata_index_store(metadata_index);

  // 4. metadata_index_destroy
  metadata_index_destroy(&metadata_index);

  // 1. metadata_index_load
  metadata_index = metadata_index_load(metadata_index_filename);
  
  // 2. Read ith metadata_row

  // 3. Display ith metadata_row

  // 4. Read query filter
  
  // 5. Filter row using query filter
  // item = read_metadata_item_by_column_name
  // is_filtered = filter_metadata_row_by_item(item, filter)

  // 6. metadata_index_destroy
  metadata_index_destroy(&metadata_index);

  return 0;
}
