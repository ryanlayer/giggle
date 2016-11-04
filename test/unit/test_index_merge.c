#define _GNU_SOURCE

#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>

#include "unity.h"
#include "bpt.h"
#include "giggle_index.h"
#include "lists.h"
#include "file_read.h"
#include "wah.h"
#include "ll.h"
#include "jsw_avltree.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void setUp(void) { }
void tearDown(void) { }

//{{{ void test_giggle_init_store_load_block(void)
void test_giggle_index_search_store_search_block(void)
{
    /*
     * To merge:
     * chrm_index: make a union of chrm_index elements
     * file_index: make a union of file_index elmennts
     * offset_index: the ids from each offset_index must be mapped to new ids
     * here
     * data_dir: new dest dir
     */
    struct giggle_index *gi = giggle_init(
                23,
                "tmp_0",
                1,
                uint32_t_ll_giggle_set_data_handler);
    giggle_data_handler.write_tree = giggle_write_tree_leaf_data;
    //char *path_name_0 = "../data/many/[0-1].bed.gz";
    char *path_name_0 = "../data/many/0.1.bed.gz";
    uint32_t r = giggle_index_directory(gi, path_name_0, 0);
    if (giggle_store(gi) != 0)
        errx(1, "Error storing giggle index.");
 
    giggle_index_destroy(&gi);
    cache.destroy();

    gi = giggle_init(23,
                     "tmp_1",
                     1,
                     uint32_t_ll_giggle_set_data_handler);
    giggle_data_handler.write_tree = giggle_write_tree_leaf_data;
    //char *path_name_1 = "../data/many/[2-3].bed.gz";
    char *path_name_1 = "../data/many/0.bed.gz";
    r = giggle_index_directory(gi, path_name_1, 0);
    if (giggle_store(gi) != 0)
        errx(1, "Error storing giggle index.");
 
    giggle_index_destroy(&gi);
    cache.destroy();

    CACHE_NAME_SPACE = 0;

    struct giggle_index *gi_0 =
            giggle_load("tmp_0",
                        uint32_t_ll_giggle_set_data_handler);

    CACHE_NAME_SPACE = 1;

    struct giggle_index *gi_1 =
            giggle_load("tmp_1",
                        uint32_t_ll_giggle_set_data_handler);

    giggle_data_handler.giggle_collect_intersection =
            giggle_collect_intersection_data_in_block;

    giggle_data_handler.map_intersection_to_offset_list =
            leaf_data_map_intersection_to_offset_list;

    //{{{ Get/test set of uniq chroms between the two trees
    char **merged_chrm_set;
    uint32_t num_uniq_chrm = giggle_merge_chrm_union(gi_0,
                                                     gi_1,
                                                     &merged_chrm_set);

    TEST_ASSERT_EQUAL(23, num_uniq_chrm);
    char *A_chrms[23] = { "1", "10", "11", "12", "13", "14", "15", "16",
                           "17", "18", "19", "2", "20", "21", "22", "3",
                           "4", "5", "6", "7", "8", "9", "X"};
    uint32_t i;
    for (i = 0; i < 23; ++i) {
        uint32_t j, times_seen = 0;
        for (j = 0; j < num_uniq_chrm; ++j) {
            if (strcmp(A_chrms[i], merged_chrm_set[j]) == 0)
                times_seen += 1;
        }
        TEST_ASSERT_EQUAL(1, times_seen);
    }
    //}}}

    //{{{ Get/test merged file index
    struct indexed_list *file_index_id_map_0 = 
            indexed_list_init(gi_0->file_idx->index->num,
                              sizeof(uint64_t));
    struct indexed_list *file_index_id_map_1 = 
            indexed_list_init(gi_1->file_idx->index->num,
                              sizeof(uint64_t));

    struct unordered_list *merged_file_index =
            unordered_list_init(gi_0->file_idx->index->num + gi_1->file_idx->index->num);

    TEST_ASSERT_EQUAL(gi_0->file_idx->index->num, 
                      giggle_merge_add_file_index(gi_0,
                                                  file_index_id_map_0,
                                                  merged_file_index));

    TEST_ASSERT_EQUAL(gi_1->file_idx->index->num, 
                      giggle_merge_add_file_index(gi_1,
                                                  file_index_id_map_1,
                                                  merged_file_index));


    TEST_ASSERT_EQUAL(gi_0->file_idx->index->num + gi_1->file_idx->index->num, 
                      merged_file_index->num);

    for (i = 0 ; i < gi_0->file_idx->index->num; ++i) {
        struct file_data *fd = (struct file_data *)
                unordered_list_get(gi_0->file_idx->index, i);
        uint32_t j, times_seen = 0;
        for (j = 0; j < merged_file_index->num; ++j) {
            struct file_data *m_fd = (struct file_data *)
                    unordered_list_get(merged_file_index, j);

            if ((strcmp(fd->file_name, m_fd->file_name) == 0) &&
                (fd->num_intervals == m_fd->num_intervals) &&
                (fd->mean_interval_size == m_fd->mean_interval_size)) {

                uint32_t *mi = (uint32_t *)
                        indexed_list_get(file_index_id_map_0, i);
                TEST_ASSERT_EQUAL(j, *mi);
                times_seen += 1;
            }
        }
        TEST_ASSERT_EQUAL(1, times_seen);
    }

    for (i = 0 ; i < gi_1->file_idx->index->num; ++i) {
        struct file_data *fd = (struct file_data *)
                unordered_list_get(gi_1->file_idx->index, i);
        uint32_t j, times_seen = 0;
        for (j = 0; j < merged_file_index->num; ++j) {
            struct file_data *m_fd = (struct file_data *)
                    unordered_list_get(merged_file_index, j);

            if ((strcmp(fd->file_name, m_fd->file_name) == 0) &&
                (fd->num_intervals == m_fd->num_intervals) &&
                (fd->mean_interval_size == m_fd->mean_interval_size)) {

                uint32_t *mi = (uint32_t *)
                        indexed_list_get(file_index_id_map_1, i);
                TEST_ASSERT_EQUAL(j, *mi);
                times_seen += 1;
            }
        }
        TEST_ASSERT_EQUAL(1, times_seen);
    }
    //}}}

    // Create dir for the new tree
    char *data_dir = "tmp_merge";
    uint32_t force = 1;

    struct stat st = {0};
    if (stat(data_dir, &st) == -1) {
        mkdir(data_dir, 0700);
    } else if (force == 1) {
        rmrf(data_dir);
        mkdir(data_dir, 0700);
    } else {
        fprintf(stderr,
                "The directory '%s' already exists. "
                "Use the force option to overwrite.\n",
                data_dir);
        //return NULL;
        return;
    }

    struct ordered_set *merged_chrm_index = 
            ordered_set_init(MAX(gi_0->chrm_idx->index->num, 
                                 gi_1->chrm_idx->index->num),
                             str_uint_pair_sort_element_cmp,
                             str_uint_pair_search_element_cmp,
                             str_uint_pair_search_key_cmp);


    struct file_id_offset_pairs *merged_offset_index =
            (struct file_id_offset_pairs *)
            malloc(sizeof(struct file_id_offset_pairs));
    merged_offset_index->num = 0;
    merged_offset_index->size = 2000;
    merged_offset_index->vals = (struct file_id_offset_pair *)
            malloc(merged_offset_index->size * 
                   sizeof(struct file_id_offset_pair));

    //for (i = 0; i < num_uniq; ++i) {
    // Loop over all the chroms
    for (i = 0; i < 1; ++i) {
        char *index_file_name = NULL, *data_file_name  = NULL;

        asprintf(&index_file_name, "%s/cache.%u.idx", data_dir, i);
        asprintf(&data_file_name, "%s/cache.%u.dat", data_dir, i);

        struct disk_store *ds = disk_store_init(1000,
                                                NULL,
                                                index_file_name,
                                                NULL,
                                                data_file_name);

        // build a new tree
        uint32_t root_id = giggle_merge_chrom(merged_chrm_set[i],
                                              gi_0,
                                              file_index_id_map_0,
                                              0,
                                              gi_1,
                                              file_index_id_map_1,
                                              1,
                                              ds,
                                              &merged_offset_index);

        // add the root id to the chrom index
    //uint32_t unordered_list_add(struct unordered_list *ul,
                            //void *data);

        disk_store_sync(ds);
        disk_store_destroy(&ds);
    }

    unordered_list_destroy(&merged_file_index, file_data_free);
    CACHE_NAME_SPACE = 0;
    giggle_index_destroy(&gi_0);
    cache.destroy();
    CACHE_NAME_SPACE = 1;
    giggle_index_destroy(&gi_1);
    cache.destroy();
}
//}}}
