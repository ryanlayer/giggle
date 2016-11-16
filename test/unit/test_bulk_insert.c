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
#include <glob.h>
#include <string.h>

#include "unity.h"
#include "bpt.h"
#include "giggle_index.h"
#include "lists.h"
#include "file_read.h"
#include "wah.h"
#include "ll.h"
#include "jsw_avltree.h"
#include "pq.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

struct pq_data
{
    uint32_t file_id, interval_id;
};

void setUp(void) { }
void tearDown(void) { }

//{{{ void test_giggle_bulk_insert(void)
void test_giggle_bulk_insert(void)
{
    //{{{ Testing constants
    uint32_t BPT_keys_0[100] = {1, 50481, 50514, 144527, 144577, 404661, 404708,
        422395, 422436, 562161, 562202, 566581, 567102, 585763, 585804, 816018,
        816215, 829181, 829221, 935394, 936147, 936482, 936605, 1004540,
        1004903, 1014910, 1015319, 1243704, 1244171, 1284817, 1284894, 1333543,
        1334139, 1334987, 1335286, 1406946, 1407324, 1446566, 1446696, 1550487,
        1551148, 1585271, 1585317, 1624220, 1624282, 1689977, 1690749, 1705115,
        1705170, 1713720, 1714070, 1777263, 1777312, 1850348, 1850535, 2023080,
        2023417, 2050152, 2050195, 2111658, 2112294, 2125435, 2125603, 2139348,
        2139395, 2142838, 2142876, 2144034, 2144521, 2322920, 2323328, 2517907,
        2518312, 2540473, 2540521, 2574446, 2574807, 2741456, 2741507, 3142094,
        3142139, 3190381, 3190416, 3252166, 3252207, 3455260, 3455310, 3578853,
        3579009, 3585079, 3585112, 3612184, 3612525, 3636454, 3636501, 3689319,
        3691116, 3696692, 3696741, 3774727}; 

    uint32_t BPT_keys_1[100] = {3774986, 3809712, 3809783, 3825025, 3825065,
        4046152, 4046215, 4324287, 4324327, 4364646, 4364680, 4413593, 4413626,
        4480292, 4480341, 4569164, 4569196, 4582988, 4583026, 4643102, 4643134,
        4643898, 4643933, 4739868, 4739913, 4873368, 4873409, 4992134, 4992166,
        5068291, 5068337, 5068817, 5068850, 5082558, 5082596, 5309091, 5309133,
        5336759, 5336798, 5748446, 5748479, 5781921, 5782215, 5866231, 5866265,
        5945881, 5946136, 5976058, 5976417, 6077466, 6077500, 6087279, 6087335,
        6187730, 6187938, 6192940, 6192971, 6219287, 6219320, 6289843, 6289885,
        6383797, 6383831, 6423193, 6423390, 6430517, 6430567, 6453604, 6454441,
        6474367, 6474735, 6483544, 6483800, 6486473, 6486687, 6769020, 6769062,
        6793250, 6793281, 6800601, 6800646, 6849663, 6849671, 6850435, 6850470,
        6944351, 6944397, 6959614, 6959655, 6962099, 6962134, 7134888, 7134899,
        7172076, 7172248, 7189979, 7190016, 7257688, 7257739, 7278281};

    uint32_t bpt_node_tested_0 = 0, bpt_node_tested_1 = 0;
    //}}}

    char *path_name = "../data/many/*gz";

    glob_t results;
    int ret = glob(path_name, 0, NULL, &results);
    if (ret != 0) 
        errx(1,
             "Problem with %s (%s), stopping early\n",
             path_name,
             (ret == GLOB_ABORTED ? "filesystem problem" :
             ret == GLOB_NOMATCH ? "no match of pattern" :
             ret == GLOB_NOSPACE ? "no dynamic memory" :
             "unknown problem"));

    //Array of open pre-sorted input files
    struct input_file **i_files = (struct input_file **)
            malloc(results.gl_pathc * sizeof(struct input_file *));

    // Use these to read intervals from files
    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len * sizeof(char));
    uint32_t start, end;
    long offset;

    // Priority queue of starts
    pri_queue pq_start = priq_new(results.gl_pathc);
    priority pri_start;
    // Since we know that each file will have at most one start in the priority
    // queue, we can reduce mallocs by reusing the array
    struct pq_data *pqd_starts = (struct pq_data *)
            malloc(results.gl_pathc * sizeof(struct pq_data));

    // Priority queue of ends
    pri_queue pq_end = priq_new(results.gl_pathc);
    priority pri_end;
    // We cannot assume that there will be some set numberof ends per file
    // (contained intervals) so we must malloc on each insert
    struct pq_data *pqd_end;

    // Init other associated indexes
    struct file_index *file_idx = file_index_init(10,
                                                  "test_bulk_insert.file.idx");
    uint32_t interval_id = 0;
    struct offset_index *offset_idx = 
            offset_index_init(1000,
                              "test_bulk_insert.offset.idx");
 
    struct chrm_index *chrm_idx = chrm_index_init(24,
                                                  "test_bulk_insert.chrm.idx");
   
    //{{{ add one interval from each file to the priority queue
    uint32_t i;
    for (i = 0; i < results.gl_pathc; i++) {
        i_files[i] = input_file_init(results.gl_pathv[i]);
        // register the file with the file index
        uint32_t file_id = file_index_add(file_idx, results.gl_pathv[i]);
        TEST_ASSERT_EQUAL(i, file_id);
        ret = i_files[i]->input_file_get_next_interval(i_files[i],
                                                       &chrm,
                                                       &chrm_len,
                                                       &start,
                                                       &end,
                                                       &offset);
        // register the interval with the offset index
        interval_id = offset_index_add(offset_idx,
                                       offset,
                                       file_id);

        //Update the pq data for the start, use the array to reduce mallocs
        pqd_starts[i].file_id = file_id;
        pqd_starts[i].interval_id = interval_id;
        pri_start.pos = start;
        strcpy(pri_start.chrm, chrm);
        priq_push(pq_start, &(pqd_starts[i]), pri_start);

        //{{{ debug
        /*
        fprintf(stderr,
                "%s\t%u %u %u\n",
                results.gl_pathv[i],
                interval_id,
                pri.pos, start);
        */
        //}}}

        //Update the pq data for the end
        pqd_end = (struct pq_data *) malloc(sizeof(struct pq_data));
        pqd_end->file_id = file_id;
        pqd_end->interval_id = interval_id;
        pri_end.pos = end + 1; // use end + 1
        strcpy(pri_end.chrm, chrm);
        priq_push(pq_end, pqd_end, pri_end);
    }
    globfree(&results);
    //}}}

    TEST_ASSERT_EQUAL(22, file_idx->index->num);

    struct pq_data *pqd_start =
            (struct pq_data *)priq_top(pq_start, &pri_start);

    // curr_pos and curr_chrm track the status of the indexing
    uint32_t curr_pos = pri_start.pos;
    char curr_chrm[10];
    strcpy(curr_chrm, pri_start.chrm);

    // register the chrom with chrom index
    uint32_t curr_chrm_id = chrm_index_add(chrm_idx, curr_chrm);

    //{{{ init disk store, do this at the start of every chrom
    char *ds_curr_index_file_name = NULL, *ds_curr_data_file_name = NULL;
    ret = asprintf(&ds_curr_index_file_name,
                   "test_bulk_insert.ds_idx.%u",
                   curr_chrm_id);
    ret = asprintf(&ds_curr_data_file_name,
                   "test_bulk_insert.ds_data.%u",
                   curr_chrm_id);
    struct disk_store *curr_ds = disk_store_init(10,
                                                 NULL,
                                                 ds_curr_index_file_name,
                                                 NULL,
                                                 ds_curr_data_file_name);
    //}}}

    // Collect the values into this node, then write it and clear 
    struct bpt_node *bpn = (struct bpt_node *) malloc(sizeof(struct bpt_node));
    bpn->data = (uint32_t *) malloc(BPT_NODE_NUM_ELEMENTS  * sizeof(uint32_t));
    memset(bpn->data, 0, BPT_NODE_SIZE);

    //BPT_ID(bpn) =  curr_ds->num;
    BPT_ID(bpn) = 1;
    BPT_PARENT(bpn) = 0;
    BPT_IS_LEAF(bpn) = 1;
    BPT_LEADING(bpn) = 0;
    BPT_NEXT(bpn) = 0;
    BPT_NUM_KEYS(bpn) = 0;
    BPT_POINTERS_BLOCK(bpn) = 0;

    // These will be used to create the leaf data for each node
    uint32_t num_leading = 0, num_starts = 0, num_ends = 0;
    struct uint32_t_array *leading, *starts, *ends;
    leading = uint32_t_array_init(100);
    starts = uint32_t_array_init(100);
    ends = uint32_t_array_init(100);

    // This tree will track intervals that have begun and not yet ended and
    // will be used to populate the leading value of nodes
    jsw_avltree_t *avl = jsw_avlnew(uint_cmp_f, uint_dup_f, uint_rel_f);

    // add the current possition to the node
    ret = giggle_bulk_insert_append_bpt_key(bpn,
                                            curr_pos,
                                            curr_ds,
                                            avl,
                                            leading,
                                            starts,
                                            ends);

    // Loop over the start queue until it is empty
    while (priq_top(pq_start, &pri_start) != NULL) {
        // Grab the top element
        pqd_start = (struct pq_data *)priq_pop(pq_start, &pri_start);
        //fprintf(stderr, "%s s:%u\n", pri_start.chrm, pri_start.pos);

        /* The posibilities for this start position are that:
         * 1) it has been seen before, in which case we will need to add the
         * interval id associated with that position to the starts leaf data
         * and leave the bp tree node alone
         * 2) it has not been seen before, so it will need to be eventually
         * added it to the tree we need to first let the ends catch up by
         * popping any end that is less than to the start that was just seen 3)
         * it is on a new chromosome and we need to do everything that is in 2)
         * as well as close out the disk store for the current chrom and start
         * a new one
         *
         * - Every start and end must be added to the starts and ends arrays.
         * - Any time a new node is created, we need to move the leading,
         *   starts, and ends arrays to a leaf node, and reset the arrays
         */

        if ((pri_start.pos == curr_pos) && 
            (strcmp(curr_chrm, pri_start.chrm) == 0)) {
            // The key didnt' change, so append the current
            // interval id to the end of the leaf data starts

            uint32_t idx = uint32_t_array_add(starts, pqd_start->interval_id);
            // bump starts 
            giggle_bulk_insert_set_starts(bpn, idx);

            // Add interval to tree to track intervals for leading value
#if DEBUG
            fprintf(stderr,
                    "-> %s %u %u\n",
                    pri_start.chrm,
                    pri_start.pos,
                    pqd_start->interval_id);
#endif

            jsw_avlinsert(avl, &(pqd_start->interval_id));
        } else {
            //{{{ #2, we need to go through the ends to catch up
            pqd_end = (struct pq_data *)priq_top(pq_end, &pri_end);

            // Since the key changed, flush out the ends to this or new keys
            // up to the value of the next start
            while ( (pqd_end != NULL) && //not empy
                    ((strcmp(pri_start.chrm, pri_end.chrm) != 0) || //same chr
                     (pri_end.pos < pri_start.pos)) ) { // < the start we saw

                pqd_end = (struct pq_data *)priq_pop(pq_end, &pri_end);
                //fprintf(stderr, "%s e:%u\n", pri_end.chrm, pri_end.pos);

                if (curr_pos == pri_end.pos)  {
                    // The key didnt' change, so append the current
                    // interval id to the end of the leaf data ends
                    uint32_t idx = 
                            uint32_t_array_add(ends,
                                               pqd_end->interval_id);
                    // bump ends
                    giggle_bulk_insert_set_ends(bpn, idx);

                    // remove end from tree tracking leading values
#if DEBUG
                    fprintf(stderr,
                            "<- %s %u %u\n",
                            pri_end.chrm,
                            pri_end.pos,
                            pqd_end->interval_id);
#endif

                    ret = jsw_avlerase(avl, &(pqd_end->interval_id));
                    if (ret == 0)
                        errx(1, "Error removing element from tree.");
                } else {
                    ret = giggle_bulk_insert_append_bpt_key(bpn,
                                                            pri_end.pos,
                                                            curr_ds,
                                                            avl,
                                                            leading,
                                                            starts,
                                                            ends);

                    uint32_t idx =
                            uint32_t_array_add(ends,
                                               pqd_end->interval_id);
                    // bump ends
                    giggle_bulk_insert_set_ends(bpn, idx);

                    // remove end from tree tracking leading values
#if DEBUG
                    fprintf(stderr,
                            "<- %s %u %u\n",
                            pri_end.chrm,
                            pri_end.pos,
                            pqd_end->interval_id);
#endif
                    ret = jsw_avlerase(avl, &(pqd_end->interval_id));
                    if (ret == 0)
                        errx(1, "Error removing element from tree.");


                    curr_pos = pri_end.pos;

                    //{{{ bpt node contents test
                    if (BPT_NUM_KEYS(bpn) == ORDER) {
                        if (bpt_node_tested_0 == 0) {
                            uint32_t i;
                            for (i = 0; i < ORDER; ++i)
                                TEST_ASSERT_EQUAL(BPT_keys_0[i],
                                                  BPT_KEYS(bpn)[i]);
                            bpt_node_tested_0 = 1;
                        } else if (bpt_node_tested_1 == 0) {
                            uint32_t i;
                            for (i = 0; i < ORDER; ++i)
                                TEST_ASSERT_EQUAL(BPT_keys_1[i],
                                                  BPT_KEYS(bpn)[i]);
                            bpt_node_tested_1 = 1;
                        }
                    }
                    //}}}
                }

                free(pqd_end);
                pqd_end = (struct pq_data *)priq_top(pq_end, &pri_end);
            }
            //}}}

            // If the chrom did change, we need to sync up the disk store and
            // open up a new one
            if (strcmp(curr_chrm, pri_start.chrm) != 0) {

                if (BPT_NUM_KEYS(bpn) > 0) {
                    BPT_POINTERS_BLOCK(bpn) = (curr_ds->num + 1) + 1;//1-based
                    BPT_NEXT(bpn) = 0;

                    giggle_bulk_insert_write_leaf_node(bpn,
                                                       curr_ds,
                                                       leading,
                                                       starts,
                                                       ends);
                    // Reset the bpt node
                    memset(bpn->data, 0, BPT_NODE_SIZE);
                    BPT_ID(bpn) =  1;
                    BPT_PARENT(bpn) = 0;
                    BPT_IS_LEAF(bpn) = 1;
                    BPT_LEADING(bpn) = 0;
                    BPT_NEXT(bpn) = 0;
                    BPT_NUM_KEYS(bpn) = 0;
                    BPT_POINTERS_BLOCK(bpn) = 0;
                }

                strcpy(curr_chrm, pri_start.chrm);
                //register the new chrom
                curr_chrm_id = chrm_index_add(chrm_idx,
                                              curr_chrm);
                //{{{ fix up the disk store
                disk_store_sync(curr_ds);
                disk_store_destroy(&curr_ds);
                ret = asprintf(&ds_curr_index_file_name,
                               "test_bulk_insert.ds_idx.%u",
                               curr_chrm_id);
                ret = asprintf(&ds_curr_data_file_name,
                               "test_bulk_insert.ds_data.%u",
                               curr_chrm_id);
                curr_ds = disk_store_init(10,
                                          NULL,
                                          ds_curr_index_file_name,
                                          NULL,
                                          ds_curr_data_file_name);
                //}}}
            }

            curr_pos = pri_start.pos;
            ret = giggle_bulk_insert_append_bpt_key(bpn,
                                                    curr_pos,
                                                    curr_ds,
                                                    avl,
                                                    leading,
                                                    starts,
                                                    ends);
            uint32_t idx = uint32_t_array_add(starts, pqd_start->interval_id);
            // bump starts
            giggle_bulk_insert_set_starts(bpn, idx);

            // add to tree tracking the leading values
#if DEBUG
            fprintf(stderr,
                    "-> %s %u %u\n",
                    pri_start.chrm,
                    pri_start.pos,
                    pqd_start->interval_id);
#endif

            jsw_avlinsert(avl, &(pqd_start->interval_id));
            //{{{ bpt node contents test
            if (BPT_NUM_KEYS(bpn) == ORDER) {
                if (bpt_node_tested_0 == 0) {
                    uint32_t i;
                    for (i = 0; i < ORDER; ++i)
                        TEST_ASSERT_EQUAL(BPT_keys_0[i],
                                          BPT_KEYS(bpn)[i]);
                    bpt_node_tested_0 = 1;
                } else if (bpt_node_tested_1 == 0) {
                    uint32_t i;
                    for (i = 0; i < ORDER; ++i)
                        TEST_ASSERT_EQUAL(BPT_keys_1[i],
                                          BPT_KEYS(bpn)[i]);
                    bpt_node_tested_1 = 1;
                }
            }
            //}}}
            //{{{ debug
            /*
            fprintf(stderr,
                    "%s %u\ts: %u(%u)",
                    curr_chrm,
                    curr_pos,
                    pqd_start->interval_id,
                    pri.pos);
            */
            //}}}
        }

        //{{{ put another interval from the file that just lost one 
        int ret = i_files[pqd_start->file_id]->
                    input_file_get_next_interval(i_files[pqd_start->file_id],
                                                 &chrm,
                                                 &chrm_len,
                                                 &start,
                                                 &end,
                                                 &offset);

        if (ret >= 0) {
            interval_id = offset_index_add(offset_idx,
                                           offset,
                                           pqd_start->file_id);

            pqd_starts[pqd_start->file_id].interval_id = interval_id;
            pri_start.pos = start;
            strcpy(pri_start.chrm, chrm);
            priq_push(pq_start, &(pqd_starts[pqd_start->file_id]), pri_start);

            pqd_end = (struct pq_data *) malloc(sizeof(struct pq_data));
            pqd_end->file_id = pqd_start->file_id;
            pqd_end->interval_id = interval_id;
            pri_end.pos = end + 1;
            strcpy(pri_end.chrm, chrm);
            priq_push(pq_end, pqd_end, pri_end);
        }
        //}}}
    }


    // Once the start queue is empty we need to drain the end queue
    while (priq_top(pq_end, &pri_end) != NULL) {
        pqd_end = (struct pq_data *)priq_pop(pq_end, &pri_end);
        //fprintf(stderr, "%s e:%u\n", pri_end.chrm, pri_end.pos);

        if (curr_pos == pri_end.pos)  {
            uint32_t idx = uint32_t_array_add(ends, pqd_end->interval_id);
            // bump ends
            giggle_bulk_insert_set_ends(bpn, idx);

            // remove from tree tracking leading values
#if DEBUG
            fprintf(stderr,
                    "-> %s %u %u\n",
                    pri_end.chrm,
                    pri_end.pos,
                    pqd_end->interval_id);
#endif
            ret = jsw_avlerase(avl, &(pqd_end->interval_id));

            if (ret == 0)
                errx(1, "Error removing element from tree.");
            //{{{debug
            /*
            fprintf(stderr,
                    " %u(%u)",
                    pqd_end->interval_id,
                    pri_end.pos);
            */
            //}}}
        } else {
            curr_pos = pri_end.pos;
            ret = giggle_bulk_insert_append_bpt_key(bpn,
                                                    curr_pos,
                                                    curr_ds,
                                                    avl,
                                                    leading,
                                                    starts,
                                                    ends);
            uint32_t idx = uint32_t_array_add(ends, curr_pos); 
            // bump ends
            giggle_bulk_insert_set_ends(bpn, idx);

#if DEBUG
            fprintf(stderr,
                    "-> %s %u %u\n",
                    pri_end.chrm,
                    pri_end.pos,
                    pqd_end->interval_id);
#endif
            // remove from tree tracking leading values
            ret = jsw_avlerase(avl, &(pqd_end->interval_id));
            if (ret == 0)
                errx(1, "Error removing element from tree.");

            //{{{ test
            if (BPT_NUM_KEYS(bpn) == ORDER) {
                if (bpt_node_tested_0 == 0) {
                    uint32_t i;
                    for (i = 0; i < ORDER; ++i)
                        TEST_ASSERT_EQUAL(BPT_keys_0[i],
                                          BPT_KEYS(bpn)[i]);
                    bpt_node_tested_0 = 1;
                } else if (bpt_node_tested_1 == 0) {
                    uint32_t i;
                    for (i = 0; i < ORDER; ++i)
                        TEST_ASSERT_EQUAL(BPT_keys_1[i],
                                          BPT_KEYS(bpn)[i]);
                    bpt_node_tested_1 = 1;
                }
            }
            //}}}
            //{{{ debug
            /*
            fprintf(stderr,
                    "\n%s %u\ts:\te: %u(%u)",
                    curr_chrm,
                    curr_pos,
                    pqd_end->interval_id,
                    pri_end.pos);
            */
            //}}}
        }
    }
    
    if (BPT_NUM_KEYS(bpn) > 0) {
        BPT_POINTERS_BLOCK(bpn) = (curr_ds->num + 1) + 1;//1-based
        BPT_NEXT(bpn) = 0;

        giggle_bulk_insert_write_leaf_node(bpn,
                                           curr_ds,
                                           leading,
                                           starts,
                                           ends);
    }

    TEST_ASSERT_EQUAL(0, jsw_avlsize(avl));
    TEST_ASSERT_EQUAL(24, chrm_idx->index->num);
    TEST_ASSERT_EQUAL(21024, offset_idx->index->num);

    disk_store_sync(curr_ds);
    disk_store_destroy(&curr_ds);

    jsw_avldelete(avl);

    //{{{ tests
    for (i = 0; i < chrm_idx->index->num; ++i) {
        ret = asprintf(&ds_curr_index_file_name,
                       "test_bulk_insert.ds_idx.%u",
                       i);
        ret = asprintf(&ds_curr_data_file_name,
                       "test_bulk_insert.ds_data.%u",
                       i);
        curr_ds = disk_store_load(NULL,
                                  ds_curr_index_file_name,
                                  NULL,
                                  ds_curr_data_file_name);

        uint32_t num_starts = 0, num_ends = 0;

        uint32_t j;
        for (j = 0; j < curr_ds->num; ++j) {
            uint64_t size;
            void *v = disk_store_get(curr_ds, j, &size);
            struct bpt_node *bpn_in;
            uint64_t deserialized_size =
                    bpt_node_deserialize(v,
                                        size,
                                        (void **)&bpn_in); 
        
            TEST_ASSERT_EQUAL(j + 1, BPT_ID(bpn_in));//1-based
            TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(bpn_in));
            if (j < curr_ds->num - 2) {
                TEST_ASSERT_EQUAL(j + 2, BPT_NEXT(bpn_in) - 1);//1-based
            } else {
                TEST_ASSERT_EQUAL(0, BPT_NEXT(bpn_in));
            }
            TEST_ASSERT_EQUAL(j + 1, BPT_POINTERS_BLOCK(bpn_in) - 1);//1-based

            free(bpn_in);
            free(v);
            v = NULL;

            j += 1;
        
            v = disk_store_get(curr_ds, j, &size);
            struct leaf_data *ld_in;
            deserialized_size = leaf_data_deserialize(v,
                                                      size,
                                                      (void **)&ld_in); 
            num_starts += ld_in->num_starts;
            num_ends += ld_in->num_ends;

            /*
            fprintf(stderr,
                    "%u %u %u\n",
                    ld_in->num_leading,
                    ld_in->num_starts,
                    ld_in->num_ends);
            */

            free(ld_in->data);
            free(ld_in);
            free(v);
            v = NULL;
        }

        TEST_ASSERT_EQUAL(num_starts, num_ends);

        disk_store_destroy(&curr_ds);
    }
    //}}}

    for (i = 0; i < chrm_idx->index->num; ++i) {
       
        ret = asprintf(&ds_curr_index_file_name,
                       "test_bulk_insert.ds_idx.%u",
                       i);
        ret = asprintf(&ds_curr_data_file_name,
                       "test_bulk_insert.ds_data.%u",
                       i);
        curr_ds = disk_store_load(NULL,
                                  ds_curr_index_file_name,
                                  NULL,
                                  ds_curr_data_file_name);

        fprintf(stderr, "chrm:%u\n", i);

        // Here we need to loop over each level of the tree until the current
        // level has just one element in which case that element is the root

        uint32_t num_leaf_node_leaf_data = curr_ds->num;
        uint32_t curr_level_num_nodes = num_leaf_node_leaf_data / 2;
        uint32_t curr_level_first_id = 0;
        uint32_t curr_level_is_leaf = 1;
        uint32_t new_level_first_id = 0;
        uint32_t new_level_len = 
                giggle_bulk_insert_add_tree_level(curr_ds,
                                                  curr_level_first_id,
                                                  curr_level_num_nodes,
                                                  curr_level_is_leaf,
                                                  &new_level_first_id);

        if (curr_level_num_nodes > 1)
            TEST_ASSERT_EQUAL((curr_level_num_nodes + ORDER - 1) / ORDER, 
                              new_level_len);
        else
            TEST_ASSERT_EQUAL(0, new_level_len);

        disk_store_destroy(&curr_ds);
    }
    
    //{{{ remove files
    for (i = 0; i < chrm_idx->index->num; ++i) {
        ret = asprintf(&ds_curr_index_file_name,
                       "test_bulk_insert.ds_idx.%u",
                       i);
        ret = asprintf(&ds_curr_data_file_name,
                       "test_bulk_insert.ds_data.%u",
                       i);
        curr_ds = disk_store_load(NULL,
                                  ds_curr_index_file_name,
                                  NULL,
                                  ds_curr_data_file_name);
        remove(ds_curr_index_file_name);
        remove(ds_curr_data_file_name);
    }
    //}}}
}
//}}}
