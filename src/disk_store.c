#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>
#include "util.h"
#include "disk_store.h"

//{{{struct disk_store *disk_store_init(uint32_t size, FILE **fp, char
struct disk_store *disk_store_init(uint32_t size, FILE **fp, char *file_name)
{
    struct disk_store *ds =
            (struct disk_store *)calloc(1, sizeof(struct disk_store));
    ds->num = 0;
    ds->size = size;
    ds->file_name = strdup(file_name);
    ds->offsets = (uint64_t *)calloc(size, sizeof(uint64_t));

    if (*fp == NULL) {
        *fp = fopen(file_name, "wb");
        if (*fp == NULL)
            err(1, "Could not open file '%s'.", file_name);
    }
    ds->fp = *fp;
    ds->index_start_offset = ftell(*fp);

    if (fwrite(&(ds->size), sizeof(uint32_t), 1, ds->fp) != 1)
        err(1, "Could not write size to '%s'.", ds->file_name);

    if (fwrite(&(ds->num), sizeof(uint32_t), 1, ds->fp) != 1)
        err(1, "Could not write num to '%s'.", ds->file_name);

    if (fwrite(ds->offsets, sizeof(uint64_t), ds->size, ds->fp) != ds->size)
        err(1, "Could not write offsets to '%s'.", ds->file_name);

    ds->data_start_offset = ftell(ds->fp);

    return ds;
}
//}}}

//{{{struct disk_store *disk_store_load(FILE **fp, char *file_name)
struct disk_store *disk_store_load(FILE **fp, char *file_name)
{

    struct disk_store *ds =
            (struct disk_store *)calloc(1, sizeof(struct disk_store));
    ds->file_name = strdup(file_name);

    if (*fp == NULL) {
        *fp = fopen(file_name, "r+");
        if (*fp == NULL)
            err(1, "Could not open file '%s'.", file_name);
    }

    ds->fp = *fp;
    ds->index_start_offset = ftell(ds->fp);

    size_t fr = fread(&(ds->size), sizeof(uint32_t), 1, ds->fp);
    check_file_read(ds->file_name, ds->fp, 1, fr);

    fr = fread(&(ds->num), sizeof(uint32_t), 1, ds->fp);
    check_file_read(ds->file_name, ds->fp, 1, fr);

    ds->offsets = (uint64_t *)calloc(ds->size, sizeof(uint64_t));

    fr = fread(ds->offsets, sizeof(uint64_t), ds->size, ds->fp);
    check_file_read(ds->file_name, ds->fp, ds->size, fr);

    ds->data_start_offset = ftell(ds->fp);

    return ds;
}
//}}}

//{{{void disk_store_sync(struct disk_store *ds)
void disk_store_sync(struct disk_store *ds)
{
    if (fseek(ds->fp, ds->index_start_offset, SEEK_SET) != 0)
        err(1, "Could not seek to index start in '%s'.", ds->file_name);

    if (fwrite(&(ds->size), sizeof(uint32_t), 1, ds->fp) != 1)
        err(1, "Could not write size to '%s'.", ds->file_name);

    if (fwrite(&(ds->num), sizeof(uint32_t), 1, ds->fp) != 1)
        err(1, "Could not write num to '%s'.", ds->file_name);

    if (fwrite(ds->offsets, sizeof(uint64_t), ds->size, ds->fp) != ds->size)
        err(1, "Could not write offsets to '%s'.", ds->file_name);
}
//}}}

//{{{void disk_store_destroy(struct disk_store **ds)
void disk_store_destroy(struct disk_store **ds)
{
    disk_store_sync(*ds);

    free((*ds)->file_name);
    free((*ds)->offsets);
    fclose((*ds)->fp);
    free(*ds);
    *ds = NULL;
}
//}}}

//{{{uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t
uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t size)
{
    if (ds->num + 1 >= ds->size)
        errx(1, "Disk store is full.");

    if (fseek(ds->fp, 0, SEEK_END) != 0)
        err(1, "Could not seek to the end to append in '%s'.", ds->file_name);

    uint32_t curr_id = ds->num;
    uint64_t curr_offset = ftell(ds->fp);

    if ( ((curr_id == 0) && (curr_offset != ds->data_start_offset)) ||
         ((curr_id > 0) && (curr_offset != ds->offsets[curr_id - 1])) )
        err(1, "Index and file '%s' are out of sync.", ds->file_name);
    
    if (fwrite(data, size, 1, ds->fp) != 1)
        err(1, "Could not write data to '%s'.", ds->file_name);

    ds->offsets[curr_id] = ftell(ds->fp);
    
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

    uint64_t start_offset = ds->data_start_offset;
    if (id > 0)
        start_offset = ds->offsets[id - 1];
    uint64_t end_offset = ds->offsets[id];
    *size = end_offset - start_offset;

    void *data = (void *) calloc(1, *size);

    if (fseek(ds->fp, start_offset, SEEK_SET) != 0)
        err(1, "Could not seek to data in '%s'.", ds->file_name);

    size_t fr = fread(data, *size, 1, ds->fp);
    check_file_read(ds->file_name, ds->fp, 1, fr);

    return data;
}
//}}}
