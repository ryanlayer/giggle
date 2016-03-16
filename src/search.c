#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include "giggle.h"
#include "wah.h"
#include "cache.h"

int main(int argc, char **argv)
{

    WAH_SIZE = 32;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t num_chrms = 100;

    if ((argc != 4)) {
        errx(1,
             "usage:\t%s <index dir> <region> <w|i>",
             argv[0]);
    }

    char *index_dir = argv[1];
    char *region_s = argv[2];
    char *i_type = argv[3];

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

    struct giggle_index *gi;
    if (i_type[0] == 'i') {

        gi = giggle_load(index_dir,
                         uint32_t_ll_giggle_set_data_handler);

        struct gigle_query_result *gqr = giggle_query(gi, chrm, start, end);


        uint32_t i;
        for(i = 0; i < gqr->num_files; i++) {
            char *result;
            struct giggle_query_iter *gqi = giggle_get_query_itr(gqr, i);
            while (giggle_query_next(gqi, &result) == 0) {
                printf("%s\n", result);
            }
            giggle_iter_destroy(&gqi);
        }
    } else {

    }

    giggle_index_destroy(&gi);
    cache.destroy();
}
