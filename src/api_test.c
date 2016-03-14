#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include "giggle.h"
#include "wah.h"
#include "cache.h"
#include "file_read.h"
#include "kfunc.h"

int main(int argc, char **argv)
{

    if ((argc != 5)) {
        errx(1,
             "usage:\t%s <index dir> <chrom> <start> <end>\n",
             argv[0]);
    }

    char *index_dir = argv[1];
    char *chrom = argv[2];
    uint32_t start = atoi(argv[3]);
    uint32_t end = atoi(argv[4]);

    struct giggle_index *gi = giggle_load(index_dir,
                                          uint32_t_ll_giggle_set_data_handler);

    struct gigle_query_result *gqr = giggle_query(gi, chrom, start, end);



    uint32_t i;
    for(i = 0; i < gqr->num_files; i++) {
        char *result;
        struct giggle_query_iter *gqi = giggle_get_query_itr(gqr, i);
        while (giggle_query_next(gqi, &result) == 0) {
            printf("%s\n", result);
        }
        giggle_iter_destroy(&gqi);
    }

    giggle_index_destroy(&gi);
    cache.destroy();
}
