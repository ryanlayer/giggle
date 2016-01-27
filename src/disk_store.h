#ifndef __DISK_STORE_H__
#define __DISK_STORE_H__

#include <stdint.h>
#include <stdio.h>

struct disk_store
{
    char *index_file_name, *data_file_name;
    FILE *index_fp, *data_fp;
    uint32_t size, num;
    uint64_t index_start_offset, data_start_offset;
    uint64_t *offsets;
};

struct disk_store *disk_store_init(uint32_t size,
                                   FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name);
struct disk_store *disk_store_load(FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name);
void disk_store_sync(struct disk_store *ds);
void disk_store_destroy(struct disk_store **ds);
uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t size);
void *disk_store_get(struct disk_store *ds, uint32_t id, uint64_t *size);

#endif
