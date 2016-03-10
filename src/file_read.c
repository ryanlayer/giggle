#define _GNU_SOURCE

#include <htslib/bgzf.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>

#include "util.h"
#include "file_read.h"

//{{{int scan_s(char *str, int str_len, int *s, int *e, const char delim)
int scan_s(char *str, int str_len, int *s, int *e, const char delim) {
    if (*e == str_len)
        return -1;
    for (*e = *s; *e <= str_len; *e+=1) {
        if ((str[*e] == delim) || (*e == str_len)) {
            return *e - *s;
        }
    }
    return -2;
}
//}}}

//{{{struct input_file *input_file_init(char *file_name)
struct input_file *input_file_init(char *file_name)
{
    struct input_file *i = (struct input_file *)
            malloc(sizeof(struct input_file));
    i->file_name = strdup(file_name);
    i->kstr = (kstring_t*)calloc(1, sizeof(kstring_t));

    if ( bgzf_is_bgzf(file_name) !=1 )
        errx(1,"Not a BGZF file: %s\n", file_name);

    if ((i->fp = bgzf_open(file_name, "r")) == 0)
        err(1,"Could not open file: %s\n", file_name);

    if ( !i->fp->is_compressed )
        err(1,"Not a bgzip compressed file: %s\n", file_name);

    i->last_offset = 0;

    

    return i;
}
//}}}

//{{{void input_file_destroy(struct input_file **i)
void input_file_destroy(struct input_file **i)
{
    free((*i)->file_name);
    bgzf_close((*i)->fp);
    free((*i)->kstr->s);
    free((*i)->kstr);
    free(*i);
    *i = NULL;
}
//}}}

//{{{int input_file_get_next_interval(struct input_file *i,
int input_file_get_next_interval(struct input_file *i,
                                 char **chrm,
                                 int *chrm_len,
                                 uint32_t *start,
                                 uint32_t *end,
                                 long *offset)
{
    *offset = i->last_offset;
    int ret = bgzf_getline(i->fp, '\n', i->kstr);
    i->last_offset = bgzf_tell(i->fp);


    if (ret < 0 )
        return ret;

    int s=0, e=0;
    int col = 0;
    while ( scan_s(i->kstr->s, i->kstr->l, &s, &e, '\t') >= 0 ) {
        col += 1;
        if (col == 1) {
            if ((e - s + 1) > *chrm_len) {
                *chrm_len = *chrm_len *2;
                *chrm = realloc(*chrm, *chrm_len * sizeof(char));
                if (*chrm == NULL)
                    errx(1, "Realloc error.\n");
            }
            memcpy(*chrm, i->kstr->s + s, e - s);
            (*chrm)[e - s] = '\0';
        } else if (col == 2) {
            *start = strtol(i->kstr->s + s, NULL, 0);
        } else if (col == 3) {
            *end = strtol(i->kstr->s + s, NULL, 0);
        } else {
            break;
        }
        s = e + 1;
    }

    return ret;
}
//}}}

//{{{void input_file_seek(struct input_file *i, long offset)
void input_file_seek(struct input_file *i, long offset)
{
    bgzf_seek(i->fp, offset, SEEK_SET);
    i->last_offset = offset;
}
//}}}

//{{{void file_data_store(void *v, FILE *f, char *file_name)
void file_data_store(void *v, FILE *f, char *file_name)
{
    struct file_data *fd = (struct file_data *)v;
    uint32_t size = strlen(fd->file_name) +
                    sizeof(uint32_t) + 
                    sizeof(double);

    if (fwrite(&size, sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing file_data size to '%s'.", file_name);

    if (fwrite(fd->file_name,
               sizeof(char),
               strlen(fd->file_name), f) != strlen(fd->file_name))
        err(EX_IOERR,
            "Error writing file_data file_name to '%s'.", file_name);

    if (fwrite(&(fd->num_intervals),
               sizeof(uint32_t),
               1, f) != 1)
        err(EX_IOERR,
            "Error writing file_data num_intervals to '%s'.", file_name);

    if (fwrite(&(fd->mean_interval_size),
               sizeof(double),
               1, f) != 1)
        err(EX_IOERR,
            "Error writing file_data mean_interval_size to '%s'.", file_name);

}
//}}}

//{{{ void *file_data_load(FILE *f, char *file_name)
void *file_data_load(FILE *f, char *file_name)
{
    uint32_t size;
    size_t fr = fread(&size, sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    uint32_t str_len = size - sizeof(uint32_t) - sizeof(double);

    struct file_data *fd = (struct file_data *)
        calloc(1, sizeof(struct file_data));

    fd->file_name = (char *)calloc(str_len + 1, sizeof(char));

    fr = fread(fd->file_name, sizeof(char), str_len, f);
    check_file_read(file_name, f, str_len, fr);

    fd->file_name[str_len] = '\0';

    uint32_t v;
    fr = fread(&v, sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    fd->num_intervals = v;

    fr = fread(&(fd->mean_interval_size), sizeof(double), 1, f);
    check_file_read(file_name, f, 1, fr);

    return (void *)fd;
}
//}}}

//{{{void file_data_free(void **v)
void file_data_free(void **v)
{
    struct file_data **fd = (struct file_data **)v;
    free((*fd)->file_name);
    free(*fd);
    *fd = NULL;
}
//}}}
