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

    if ((argc != 5)) {
        errx(1,
             "usage:\t%s <index dir> <region> <w|i> <n>",
             argv[0]);
    }

    char *index_dir = argv[1];
    char *region_s = argv[2];
    char *i_type = argv[3];
    uint32_t N = atoi(argv[4]);

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

        struct uint32_t_ll *R =
                (struct uint32_t_ll *)giggle_query_region(gi,
                                                          chrm,
                                                          start,
                                                          end);



#if 0

        for (i = 0; i < N; ++i) {
            start += 10;
            end += 10;
            char *rand_chr = NULL;
            //asprintf(&rand_chr, "chr%u", 1 + rand() %10);
            //fprintf(stderr, "%s:%u-%u\n", rand_chr, start, end);
            struct uint32_t_ll *R =
                    (struct uint32_t_ll *)giggle_query_region(gi,
                                                              chrm,
                                                              start,
                                                              end);

            /*
            if (R != NULL)
                printf("Hits:%u\n", R->len);
            else
                printf("Hits:0\n");
            */

            free(R);
        }
#endif

    } else {
        gi = giggle_load(index_dir,
                         wah_giggle_set_data_handler);

        for (i = 0; i < N; ++i) {
            start += 10;
            end += 10;
            char *rand_chr = NULL;
            //asprintf(&rand_chr, "chr%u", 1 + rand() %10);
            //fprintf(stderr, "%s:%u-%u\n", rand_chr, start, end);
 
            uint8_t *R = (uint8_t *)giggle_query_region(gi,
                                                        chrm,
                                                        start,
                                                        end);
            /*
        if (R != NULL)
            printf("Hits:%u\n", wah_get_ints_count(R));
        else
            printf("Hits:0\n");
        */
            free(R);
        }

    }

    giggle_index_destroy(&gi);
    cache.destroy();
}
