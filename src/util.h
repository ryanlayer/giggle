#ifndef __UTIL_H__
#define __UTIL_H__

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>
#include <stdint.h>

struct doubles_uint32_t_tuple
{
    double d1,d2;
    uint32_t u1,u2,u3;
};
int doubles_uint32_t_tuple_cmp(const void *_a, const void *_b);

extern struct FTW *ftwbuf;
void check_file_read(char *file_name, FILE *fp, size_t exp, size_t obs);
int unlink_cb(const char *fpath,
              const struct stat *sb,
              int typeflag,
              struct FTW *ftwbuf);
int rmrf(char *path);
int uint32_t_cmp(const void *_a, const void *_b);
uint32_t bin_char_to_int(char *bin);
int long_cmp(const void *_a, const void *_b);
#endif
