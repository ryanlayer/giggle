#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>
#include <sys/time.h>
#include "util.h"
#include "disk_store.h"
#include "zlib_wrapper.h"

//{{{ struct disk_store *disk_store_init(uint32_t size,
struct disk_store *disk_store_init(uint32_t size,
                                   FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name)
{
    struct disk_store *ds =
            (struct disk_store *)calloc(1, sizeof(struct disk_store));
    if (ds == NULL)
        err(1, "calloc error in disk_store_init().");

    ds->num = 0;
    ds->size = size;
    ds->index_file_name = strdup(index_file_name);
    ds->data_file_name = strdup(data_file_name);
    ds->offsets = (uint64_t *)calloc(size, sizeof(uint64_t));
    if (ds->offsets == NULL)
        err(1, "calloc error in disk_store_init().");

    ds->uncompressed_sizes = (uint32_t *)calloc(size, sizeof(uint32_t));
    if (ds->uncompressed_sizes == NULL)
        err(1, "calloc error in disk_store_init().");

    if ((index_fp == NULL) || (*index_fp == NULL)) {
        ds->index_fp = fopen(index_file_name, "wb");
        if (ds->index_fp == NULL)
            err(1, "Could not open index file '%s'.", index_file_name);
    } else {
        ds->index_fp = *index_fp;
    }
    ds->index_start_offset = ftell(ds->index_fp);

    if (fwrite(&(ds->size), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write size to '%s'.", ds->index_file_name);

    if (fwrite(&(ds->num), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write num to '%s'.", ds->index_file_name);

    
    // if (fwrite(ds->offsets, sizeof(uint64_t), ds->size, ds->index_fp) != ds->size)
    //     err(1, "Could not write offsets to '%s'.", ds->index_file_name);
    // if (fwrite(ds->uncompressed_sizes, sizeof(uint32_t), ds->size, ds->index_fp) != ds->size)
    //     err(1, "Could not write uncompressed_sizes to '%s'.", ds->index_file_name);
    

    if ((data_fp == NULL) || (*data_fp == NULL)) {
        ds->data_fp = fopen(data_file_name, "wb");
        if (ds->data_fp == NULL)
            err(1, "Could not open data file '%s'.", data_file_name);
    } else {
        ds->data_fp = *data_fp;
    }
    ds->data_start_offset = ftell(ds->data_fp);

    return ds;
}
//}}}

//{{{ struct disk_store *disk_store_load(FILE **index_fp,
struct disk_store *disk_store_load(FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name)
{

    struct disk_store *ds =
            (struct disk_store *)calloc(1, sizeof(struct disk_store));
    if (ds == NULL)
        err(1, "calloc error in disk_store_load().");

    ds->index_file_name = strdup(index_file_name);

    if ((index_fp == NULL) || (*index_fp == NULL)) {
        ds->index_fp = fopen(index_file_name, "r+");
        if (ds->index_fp == NULL)
            err(1, "Could not open index_file '%s'.", index_file_name);
    } else {
        ds->index_fp = *index_fp;
    }
    ds->index_start_offset = ftell(ds->index_fp);

    size_t fr = fread(&(ds->size), sizeof(uint32_t), 1, ds->index_fp);
    check_file_read(ds->index_file_name, ds->index_fp, 1, fr);

    fr = fread(&(ds->num), sizeof(uint32_t), 1, ds->index_fp);
    check_file_read(ds->index_file_name, ds->index_fp, 1, fr);

    ds->offsets = (uint64_t *)calloc(ds->size, sizeof(uint64_t));
    if (ds->offsets == NULL)
        err(1, "calloc error in disk_store_load().");

    fr = fread(ds->offsets, sizeof(uint64_t), ds->size, ds->index_fp);
    check_file_read(ds->index_file_name, ds->index_fp, ds->size, fr);

    ds->uncompressed_sizes = (uint32_t *)calloc(ds->size, sizeof(uint32_t));
    if (ds->uncompressed_sizes == NULL)
        err(1, "calloc error in disk_store_load().");

    fr = fread(ds->uncompressed_sizes, sizeof(uint32_t), ds->size, ds->index_fp);
    check_file_read(ds->index_file_name, ds->index_fp, ds->size, fr);

    ds->data_file_name = strdup(data_file_name);

    if ((data_fp == NULL) || (*data_fp == NULL)) {
        ds->data_fp = fopen(data_file_name, "r+");
        if (ds->data_fp == NULL)
            err(1, "Could not open data file '%s'.", data_file_name);
    } else {
        ds->data_fp = *data_fp;
    }
    ds->data_start_offset = ftell(ds->data_fp);

    return ds;
}
//}}}

//{{{void disk_store_sync(struct disk_store *ds)
void disk_store_sync(struct disk_store *ds)
{
    if (fseek(ds->index_fp, ds->index_start_offset, SEEK_SET) != 0)
        err(1, "Could not seek to index start in '%s'.", ds->index_file_name);

    if (fwrite(&(ds->size), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write size to '%s'.", ds->index_file_name);

    if (fwrite(&(ds->num), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write num to '%s'.", ds->index_file_name);

    if (fwrite(ds->offsets, sizeof(uint64_t), ds->size, ds->index_fp) != 
            ds->size)
        err(1, "Could not write offsets to '%s'.", ds->index_file_name);

    if (fwrite(ds->uncompressed_sizes, sizeof(uint32_t), ds->size, ds->index_fp) != 
            ds->size)
        err(1, "Could not write uncompressed_sizes to '%s'.", ds->index_file_name);
}
//}}}

//{{{void disk_store_destroy(struct disk_store **ds)
void disk_store_destroy(struct disk_store **ds)
{
    disk_store_sync(*ds);

    free((*ds)->index_file_name);
    free((*ds)->data_file_name);
    free((*ds)->offsets);
    free((*ds)->uncompressed_sizes);

    if ((*ds)->index_fp != (*ds)->data_fp) {
        fclose((*ds)->data_fp);
        (*ds)->data_fp = NULL;
    }

    fclose((*ds)->index_fp);
    (*ds)->index_fp = NULL;

    free(*ds);
    *ds = NULL;
}
//}}}

//{{{uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t
uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t size)
{
    //if (ds->num + 1 >= ds->size)
    //if (ds->num  >= ds->size)
        //errx(1, "Disk store is full.");
    if (ds->num >= ds->size) {
        uint32_t old_size = ds->size;
        ds->size = ds->size * 2;
        ds->offsets = (uint64_t *)realloc(ds->offsets,
                                          ds->size * sizeof(uint64_t));
        if (ds->offsets == NULL)
            err(1, "realloc error in disk_store_append().");

        memset(ds->offsets + old_size,
               0,
               old_size * sizeof(uint64_t));

        ds->uncompressed_sizes = (uint32_t *)realloc(ds->uncompressed_sizes,
                                          ds->size * sizeof(uint32_t));
        if (ds->uncompressed_sizes == NULL)
            err(1, "realloc error in disk_store_append().");

        memset(ds->uncompressed_sizes + old_size,
               0,
               old_size * sizeof(uint32_t));
    }

    if (fseek(ds->data_fp, 0, SEEK_END) != 0)
        err(1,
            "Could not seek to the end to append in '%s'.",
            ds->data_file_name);

    uint32_t curr_id = ds->num;
    uint64_t curr_offset = ftell(ds->data_fp);

    if ( ((curr_id == 0) && (curr_offset != ds->data_start_offset)) ||
         ((curr_id > 0) && (curr_offset != ds->offsets[curr_id - 1])) )
        err(1, "Index and file '%s' are out of sync.", ds->data_file_name);
    
    uLong compressed_size;
    // TODO: Typecasting uint64_t to uLong -> Potential integer overflow. Will crash if (uncompressed) size > 4GB
    void *compressed_data = zlib_compress(data, size, &compressed_size);
    if (fwrite(compressed_data, compressed_size, 1, ds->data_fp) != 1)
        err(1, "Could not write data to '%s'.", ds->data_file_name);

    ds->offsets[curr_id] = ftell(ds->data_fp);
    ds->uncompressed_sizes[curr_id] = size;
    
    ds->num += 1;

    return curr_id; 
}
//}}}

//{{{void *disk_store_get(struct disk_store *ds, uint32_t id, uint64_t *size)
void *disk_store_get(struct disk_store *ds, uint32_t id, uint64_t *size)
{
    if (id >= ds->num) {
        *size = 0;
        return NULL;
    }
    FILE *fp = fopen("disk_zlib_log.csv", "a");
    if (fp == NULL)
        err(1, "Could not open log file.");

    // start time
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    uint64_t start_offset = ds->data_start_offset;
    if (id > 0)
        start_offset = ds->offsets[id - 1];
    uint64_t end_offset = ds->offsets[id];
    uLong compressed_size = end_offset - start_offset;

    void *data = (void *) calloc(1, compressed_size);
    if (data == NULL)
        err(1, "calloc error in disk_store_get().");

    if (fseek(ds->data_fp, start_offset, SEEK_SET) != 0)
        err(1, "Could not seek to data in '%s'.", ds->data_file_name);

    size_t fr = fread(data, compressed_size, 1, ds->data_fp);
    check_file_read(ds->data_file_name, ds->data_fp, 1, fr);

    uint32_t uncompressed_size = ds->uncompressed_sizes[id];
    void *uncompressed_data = zlib_uncompress(data, compressed_size, uncompressed_size);

    *size = uncompressed_size;

    // end time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    long elapsed = seconds * 1e6 + microseconds;
    // append time, compressed size, uncompressed size
    fprintf(fp, "%u\t%lu\t%lu\n", uncompressed_size, compressed_size, elapsed);
    fclose(fp);

    return uncompressed_data;
}
//}}}
