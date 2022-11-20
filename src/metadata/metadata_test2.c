#include <stdio.h>
#include "metadata_index.h"

int main(void) {
  char *metadata_conf_filename = "metadata.conf";
  char *metadata_index_filename = "metadata_index.dat";

  // 1. metadata_index_init
  struct metadata_index *metadata_index = metadata_index_init(metadata_conf_filename, metadata_index_filename);
  
  // 2. metadata_index_add 
  // 3. metadata_index_store
  // 4. metadata_index_destroy
  metadata_index_destroy(&metadata_index);
  
  return 0;
}
