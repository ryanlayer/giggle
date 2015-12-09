#ifndef __FILE_READ_H__
#define __FILE_READ_H__

#include <htslib/bgzf.h>
#include <stdint.h>

struct input_file
{
    char *file_name;
    BGZF *fp;
    kstring_t *kstr;
    long last_offset;
};

int scan_s(char *str, int str_len, int *s, int *e, const char delim);
struct input_file *input_file_init(char *file_name);
void input_file_destroy(struct input_file **i);
void input_file_seek(struct input_file *i, long offset);
int input_file_get_next_interval(struct input_file *i,
                                 char **chrm,
                                 int *chrm_len,
                                 uint32_t *start,
                                 uint32_t *end,
                                 long *offset);

#endif
