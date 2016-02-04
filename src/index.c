#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "giggle.h"
#include "cache.h"

int main(int argc, char **argv)
{
    uint32_t num_chrms = 100;

    if ((argc < 3) || (argc > 4)) {
        errx(1,
             "usage:\t%s <input dir> <output dir> <force (0)>",
             argv[0]);
    }

    char *input_dir = argv[1];
    char *output_dir = argv[2];

    uint32_t force = 0;
    if (argc == 4)
        force = atoi(argv[3]);

    struct giggle_index *gi = giggle_init(
                num_chrms,
                output_dir,
                force,
                uint32_t_ll_giggle_set_data_handler);

    uint32_t r = giggle_index_directory(gi, input_dir, 0);
    
    fprintf(stderr, "Indexed %u intervals.\n", r);


    r = giggle_store(gi);

    if (r != 0)
        errx(1, "Error storing giggle index.");
    
    giggle_index_destroy(&gi);
    cache.destroy();
}
