#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include "giggle.h"
#include "wah.h"
#include "cache.h"
#include "file_read.h"
#include "kfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int main(int argc, char **argv)
{
    WAH_SIZE = 32;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t num_chrms = 100;

    if ((argc != 4)) {
        errx(1,
             "usage:\t%s <input file> <index dir> <w|i>",
             argv[0]);
    }

    double genome_size =  3095677412.0;

    char *input_file = argv[1];
    char *index_dir = argv[2];
    char *i_type = argv[3];

    struct input_file *in_f = input_file_init(input_file);

    int chrm_len = 50;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;

    struct giggle_index *gi;

    gi = giggle_load(index_dir,
                     uint32_t_ll_giggle_set_data_handler);

    uint32_t *file_counts = (uint32_t *)
            calloc(gi->file_index->num, sizeof(uint32_t));

    uint32_t num_intervals = 0;
    double mean_interval_size = 0.0;
    while ( input_file_get_next_interval(in_f, 
                                         &chrm,
                                         &chrm_len,
                                         &start,
                                         &end,
                                         &offset) >= 0 ) {
        num_intervals += 1;
        mean_interval_size += end - start;

        struct uint32_t_ll *R =
                (struct uint32_t_ll *)giggle_query_region(gi,
                                                          chrm,
                                                          start,
                                                          end);
        if (R != NULL) {
            struct uint32_t_ll_node *curr = R->head;

            uint32_t count = 0;
            while (curr != NULL) {
                struct file_id_offset_pair *fid_off = 
                    (struct file_id_offset_pair *)
                    unordered_list_get(gi->offset_index, curr->val);
                struct file_data *fd = 
                    (struct file_data *)
                    unordered_list_get(gi->file_index, fid_off->file_id);

                struct input_file *i = input_file_init(fd->file_name);

                input_file_seek(i, fid_off->offset);

                int chrm_len = 10;
                char *r_chrm = (char *)malloc(chrm_len*sizeof(char));
                uint32_t r_start, r_end;
                long r_offset;
            
                int x = input_file_get_next_interval(i,
                                        &r_chrm,
                                        &chrm_len,
                                        &r_start,
                                        &r_end,
                                        &r_offset);
                fprintf(stderr, "%u\t%u\t%s\t%u\t%u\t%u\n",
                       fid_off->file_id,
                       curr->val,
                       r_chrm,
                       r_start,
                       r_end,
                       count);

                input_file_destroy(&i);
                curr = curr->next;
                count += 1;
            }
            uint32_t_ll_free((void **)&R);
            R=NULL;
        }
    }


    giggle_index_destroy(&gi);
    cache.destroy();
}
