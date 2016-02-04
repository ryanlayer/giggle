#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include "giggle.h"
#include "cache.h"

int main(int argc, char **argv)
{
    uint32_t num_chrms = 100;

    if ((argc != 3)) {
        errx(1,
             "usage:\t%s <index dir> <region>",
             argv[0]);
    }

    char *index_dir = argv[1];
    char *region_s = argv[2];

    char *chrm = region_s;
    uint32_t start = 0, end = 0;
    uint32_t i, len = strlen(region_s);
    
    for (i = 0; i < len; ++i) {
        if (region_s[i] == ':') {
            region_s[i] = '\0';
            start = atoi(region_s + i + 1);
        } else if (region_s[i] == '-') {
            region_s[i] = '\0';
            end = atoi(region_s + i + 1);
            break;
        }
    }

    struct giggle_index *gi = giggle_load(index_dir,
                                          uint32_t_ll_giggle_set_data_handler);

    struct uint32_t_ll *R =
            (struct uint32_t_ll *)giggle_query_region(gi,
                                                      chrm,
                                                      start,
                                                      end);

    if (R != NULL)
        printf("Hits:%u\n", R->len);
    else
        printf("Hits:0\n");


    giggle_index_destroy(&gi);
    cache.destroy();
}
