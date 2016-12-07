#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sysexits.h>
#include <inttypes.h>
#include <htslib/kstring.h>
#include <err.h>
#include <sys/mman.h>

#include "offset_index.h"
#include "util.h"

char *OFFSET_INDEX_FILE_NAME = "offset_index.dat";

uint32_t offset_data_size = 0;

void (*offset_data_append_data)(uint8_t *dest, kstring_t *line) = NULL;

//{{{struct offset_index *offset_index_init(uint32_t init_size, char
struct offset_index *offset_index_init(uint32_t init_size, char *file_name)
{
    struct offset_index *oi = 
            (struct offset_index *) malloc(sizeof(struct offset_index));
    oi->width = sizeof(struct file_id_offset_pair) + offset_data_size;
    oi->index = (struct file_id_offset_pairs *)
            malloc(sizeof(struct file_id_offset_pairs));
    oi->index->num = 0;
    oi->index->size = 1000;
    oi->index->vals = (struct file_id_offset_pair *)
            calloc(oi->index->size, oi->width);

    oi->file_name = NULL;
    if (file_name != NULL) {
        oi->file_name = strdup(file_name);
    }

    oi->f = NULL;

    return oi;
}
//}}}

//{{{void offset_index_destroy(struct offset_index **oi);
void offset_index_destroy(struct offset_index **oi)
{

    //free((*oi)->index->vals);
    free((*oi)->index);

    if ((*oi)->file_name != NULL) {
        free((*oi)->file_name);
        (*oi)->file_name = NULL;
    }

    if ((*oi)->f != NULL) {
        munmap((*oi)->index->vals, (*oi)->index->size * (*oi)->width);
        fclose((*oi)->f);
    }

    free(*oi);
    *oi = NULL;
}
//}}}

//{{{uint32_t offset_index_add(struct offset_index *oi)
uint32_t offset_index_add(struct offset_index *oi,
                          long offset,
                          kstring_t *line,
                          uint32_t file_id)
{
    uint32_t id = oi->index->num;
    oi->index->num = oi->index->num + 1;

    if (oi->index->num == oi->index->size) {
        oi->index->size = oi->index->size * 2;

        oi->index->vals = (struct file_id_offset_pair *)
                realloc(oi->index->vals, oi->index->size * oi->width);

        memset((uint8_t *)oi->index->vals + (oi->index->num * oi->width),
               0,
               oi->index->num * oi->width);
    }


    OFFSET_INDEX_PAIR(oi, id)->offset = offset;
    OFFSET_INDEX_PAIR(oi, id)->file_id = file_id;

    if (offset_data_append_data != NULL) 
        offset_data_append_data((uint8_t *)OFFSET_INDEX_DATA(oi, id), line);

   return id;
}
//}}}

//{{{void offset_index_store(struct offset_index *oi)
void offset_index_store(struct offset_index *oi)
{

    if (oi->file_name == NULL)
        errx(1,"No output file given for offset_index.");

    FILE *f = fopen(oi->file_name, "wb");
    if (f == NULL)
        err(1, "Could not open %s.", oi->file_name);

    if (fwrite(&(oi->index->num),
               sizeof(uint64_t),1, f) != 1)
        err(EX_IOERR, "Error writing offset_index num to '%s'.",
            oi->file_name);

    if (fwrite(&(oi->width),
               sizeof(uint32_t),1, f) != 1)
        err(EX_IOERR, "Error writing offset_index width to '%s'.",
            oi->file_name);

    if (fwrite(oi->index->vals, 
               oi->width,
               oi->index->num, f) != oi->index->num)
        err(EX_IOERR, "Error writing file_id offset pairs to '%s'.",
            oi->file_name);
    fclose(f);
}
//}}}

//{{{struct offset_index *offset_index_load(char *file_name)
struct offset_index *offset_index_load(char *file_name)
{
    struct offset_index *oi = (struct offset_index *)
            malloc(sizeof(struct offset_index));

    oi->file_name = strdup(file_name);

    oi->f = fopen(file_name, "rb");
    if (oi->f == NULL)
        err(1, "Could not open %s.", oi->file_name);

    oi->index = (struct file_id_offset_pairs *)
            malloc(sizeof(struct file_id_offset_pairs));
    size_t fr = fread(&(oi->index->num), sizeof(uint64_t), 1, oi->f);
    check_file_read(oi->file_name, oi->f, 1, fr);

    fr = fread(&(oi->width), sizeof(uint32_t), 1, oi->f);
    check_file_read(oi->file_name, oi->f, 1, fr);

    rewind(oi->f);

    oi->index->size = oi->index->num;
    /*
    oi->index->vals = (struct file_id_offset_pair *)
            malloc(oi->index->size * oi->width);
    fr = fread(oi->index->vals,
               oi->width,
               oi->index->num,
               f);
    check_file_read(oi->file_name, f, oi->index->num, fr);
    fclose(f);
    */

    oi->index->vals = (struct file_id_offset_pair *)
            mmap(0,
                 oi->index->size * oi->width,
                 PROT_READ,
                 MAP_FILE,
                 fileno(oi->f),
                 0);
    
    return oi;
}
//}}}

//{{{struct file_id_offset_pair offset_index_get(struct offset_index *oi,
struct file_id_offset_pair offset_index_get(struct offset_index *oi,
                                            uint32_t id)
{
    return *(OFFSET_INDEX_PAIR(oi, id));
}
//}}}
