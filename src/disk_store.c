#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>
#include "util.h"
#include "disk_file_header.h"
#include "disk_store.h"
#include "zlib_wrapper.h"

#define DISK_LOG 0

#if DISK_LOG
#include <sys/time.h>
#define DISK_LOG_FILENAME "disk_zlib_log.csv"
#endif

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

    ds->is_compressed = true;
    if (ds->is_compressed) {
        ds->file_header = new_disk_file_header('z');
    } else {
        ds->file_header = NULL;
    }

    ds->num = 0;
    ds->size = size;
    ds->index_file_name = strdup(index_file_name);
    ds->data_file_name = strdup(data_file_name);
    ds->offsets = (uint64_t *)calloc(size, sizeof(uint64_t));
    if (ds->offsets == NULL)
        err(1, "calloc error in disk_store_init().");
    
    if (ds->is_compressed) {
        ds->uncompressed_sizes = (uint32_t *)calloc(size, sizeof(uint32_t));
        if (ds->uncompressed_sizes == NULL)
            err(1, "calloc error in disk_store_init().");
    } else {
        ds->uncompressed_sizes = NULL;
    }

    if ((index_fp == NULL) || (*index_fp == NULL)) {
        ds->index_fp = fopen(index_file_name, "wb");
        if (ds->index_fp == NULL)
            err(1, "Could not open index file '%s'.", index_file_name);
    } else {
        ds->index_fp = *index_fp;
    }

    write_disk_file_header(GIGGLE_INDEX_FILE_MARKER, ds->file_header, ds->index_fp, ds->index_file_name);

    ds->index_start_offset = ftell(ds->index_fp);

    if (fwrite(&(ds->size), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write size to '%s'.", ds->index_file_name);

    if (fwrite(&(ds->num), sizeof(uint32_t), 1, ds->index_fp) != 1)
        err(1, "Could not write num to '%s'.", ds->index_file_name);

    if ((data_fp == NULL) || (*data_fp == NULL)) {
        ds->data_fp = fopen(data_file_name, "wb");
        if (ds->data_fp == NULL)
            err(1, "Could not open data file '%s'.", data_file_name);
    } else {
        ds->data_fp = *data_fp;
    }

    write_disk_file_header(GIGGLE_DATA_FILE_MARKER, ds->file_header, ds->data_fp, ds->data_file_name);

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
    
    struct disk_file_header *index_h = read_disk_file_header(ds->index_fp, ds->index_file_name, GIGGLE_INDEX_FILE_MARKER);
    if (index_h == NULL) {
        ds->is_compressed = false;
        ds->file_header = NULL;
    } else {
        ds->file_header = index_h;
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

    if (ds->is_compressed) {
        ds->uncompressed_sizes = (uint32_t *)calloc(ds->size, sizeof(uint32_t));
        if (ds->uncompressed_sizes == NULL)
            err(1, "calloc error in disk_store_load().");

        fr = fread(ds->uncompressed_sizes, sizeof(uint32_t), ds->size, ds->index_fp);
        check_file_read(ds->index_file_name, ds->index_fp, ds->size, fr);
    } else {
        ds->uncompressed_sizes = NULL;
    }

    ds->data_file_name = strdup(data_file_name);

    if ((data_fp == NULL) || (*data_fp == NULL)) {
        ds->data_fp = fopen(data_file_name, "r+");
        if (ds->data_fp == NULL)
            err(1, "Could not open data file '%s'.", data_file_name);
    } else {
        ds->data_fp = *data_fp;
    }
    
    struct disk_file_header *data_h = read_disk_file_header(ds->data_fp, ds->data_file_name, GIGGLE_DATA_FILE_MARKER);
    if (data_h == NULL) {
        if (ds->is_compressed) {
            err(1, "Missing file header in data file '%s' when index file '%s' has file header.", data_file_name, index_file_name);
        }
    } else {
        if (ds->is_compressed) {
            if (data_h->compression_method != index_h->compression_method || data_h->extra != index_h->extra || data_h->flag != index_h->flag) {
                err(1, "File header mismatch in data file '%s' and index file '%s'.", data_file_name, index_file_name);
            }
        }
        free(data_h);
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

    if (ds->is_compressed) {
        if (fwrite(ds->uncompressed_sizes, sizeof(uint32_t), ds->size, ds->index_fp) != 
                ds->size)
            err(1, "Could not write uncompressed_sizes to '%s'.", ds->index_file_name);
    }
}
//}}}

//{{{void disk_store_destroy(struct disk_store **ds)
void disk_store_destroy(struct disk_store **ds)
{
    disk_store_sync(*ds);

    free((*ds)->index_file_name);
    free((*ds)->data_file_name);
    free((*ds)->offsets);
    if ((*ds)->is_compressed) {
        free((*ds)->uncompressed_sizes);
    }
    if ((*ds)->index_fp != (*ds)->data_fp) {
        fclose((*ds)->data_fp);
        (*ds)->data_fp = NULL;
    }

    fclose((*ds)->index_fp);
    (*ds)->index_fp = NULL;

    if ((*ds)->file_header != NULL) {
        free((*ds)->file_header);
    }

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

        if (ds->is_compressed) {
            ds->uncompressed_sizes = (uint32_t *)realloc(ds->uncompressed_sizes,
                                            ds->size * sizeof(uint32_t));
            if (ds->uncompressed_sizes == NULL)
                err(1, "realloc error in disk_store_append().");

            memset(ds->uncompressed_sizes + old_size,
                0,
                old_size * sizeof(uint32_t));
        }
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
    
    if (ds->is_compressed) {
        uLong compressed_size;
        // TODO: Typecasting uint64_t to uLong -> Potential integer overflow. Will crash if (uncompressed) size > 4GB
        void *compressed_data = zlib_compress(data, size, &compressed_size);
        if (fwrite(compressed_data, compressed_size, 1, ds->data_fp) != 1)
            err(1, "Could not write data to '%s'.", ds->data_file_name);
    } else {
        if (fwrite(data, size, 1, ds->data_fp) != 1)
            err(1, "Could not write data to '%s'.", ds->data_file_name);
    }

    ds->offsets[curr_id] = ftell(ds->data_fp);

    if (ds->is_compressed) {
        ds->uncompressed_sizes[curr_id] = size;
    }
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

#if DISK_LOG
    FILE *fp = fopen(DISK_LOG_FILENAME, "a");
    if (fp == NULL)
        err(1, "Could not open log file.");

    // start time
    struct timeval begin, end;
    gettimeofday(&begin, 0);
#endif

    uint64_t start_offset = ds->data_start_offset;
    if (id > 0)
        start_offset = ds->offsets[id - 1];
    uint64_t end_offset = ds->offsets[id];

    void *data;
    uLong compressed_size = 0;
    if (ds->is_compressed) {
        compressed_size = end_offset - start_offset;
        data = (void *) calloc(1, compressed_size);
    } else {
        *size = end_offset - start_offset;
        data = (void *) calloc(1, *size);
    }

    if (data == NULL)
        err(1, "calloc error in disk_store_get().");

    if (fseek(ds->data_fp, start_offset, SEEK_SET) != 0)
        err(1, "Could not seek to data in '%s'.", ds->data_file_name);

    size_t fr;
    if (ds->is_compressed) {
        fr = fread(data, compressed_size, 1, ds->data_fp);
    } else {
        fr = fread(data, *size, 1, ds->data_fp);
    }

    check_file_read(ds->data_file_name, ds->data_fp, 1, fr);

    void *uncompressed_data;
    if (ds->is_compressed) {
        uint32_t uncompressed_size = ds->uncompressed_sizes[id];
        uncompressed_data = zlib_uncompress(data, compressed_size, uncompressed_size);
        *size = uncompressed_size;
    }

#if DISK_LOG
    // end time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    long elapsed = seconds * 1e6 + microseconds;

    if (ds->is_compressed) {
        fprintf(fp, "%u\t%lu\t%lu\n", uncompressed_size, compressed_size, elapsed);
    } else {
        fprintf(fp, "%u\t%lu\n", *size, elapsed);
    }

    fclose(fp);
#endif

    if (ds->is_compressed) {
        return uncompressed_data;
    } else {
        return data;
    }
}
