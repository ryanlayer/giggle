#define _GNU_SOURCE

#include <err.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ftw.h>

#include "util.h"

int doubles_uint32_t_tuple_cmp(const void *_a, const void *_b)
{
    struct doubles_uint32_t_tuple *a = (struct doubles_uint32_t_tuple *)_a;
    struct doubles_uint32_t_tuple *b = (struct doubles_uint32_t_tuple *)_b;

    if (a->d1 < b->d1)
        return -1;
    else if (a->d1 > b->d1)
        return 1;
    else {
        if (a->d2 < b->d2)
            return -1;
        else if (a->d2 > b->d2)
            return 1;

        return 0;
    }
}


void check_file_read(char *file_name, FILE *fp, size_t exp, size_t obs)
{
    if (exp != obs) {
        if (feof(fp))
            errx(EX_IOERR,
                 "Error reading file \"%s\": End of file",
                 file_name);
        err(EX_IOERR, "Error reading file \"%s\"", file_name);
    }
}

int unlink_cb(const char *fpath,
              const struct stat *sb,
              int typeflag,
              struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int uint32_t_cmp(const void *_a, const void *_b)
{
    uint32_t *a = (uint32_t *)_a;
    uint32_t *b = (uint32_t *)_b;

    if (*a < *b)
        return -1;
    else if (*a > *b)
        return 1;
    else
        return 0;
}

uint32_t bin_char_to_int(char *bin)
{
    uint32_t i = 0;
    int j = 0;

    while (bin[j] != '\0') {
        i = i << 1;
        if (bin[j] == '1')
            i += 1;
        j+=1;
    }

    return i;
}

int long_cmp(const void *_a, const void *_b)
{
    long *a = (long *)_a;
    long *b = (long *)_b;

    if (*a < *b)
        return -1;
    else if (*a > *b)
        return 1;
    else
        return 0;
}


