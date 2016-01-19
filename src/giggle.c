#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <glob.h>

#include "bpt.h"
#include "giggle.h"
#include "ll.h"
#include "lists.h"
#include "file_read.h"

void *(*new_non_leading)() = NULL;
void *(*new_leading)() = NULL;
void (*non_leading_SA_add_scalar)(void *non_leading, void *scalar) = NULL;
void (*non_leading_SE_add_scalar)(void *non_leading, void *scalar) = NULL;
void (*leading_B_add_scalar)(void *leading, void *scalar) = NULL;
void (*leading_union_with_B)(void **result, void *leading) = NULL;
void (*non_leading_union_with_SA)(void **result, void *non_leading) = NULL;
void (*non_leading_union_with_SA_subtract_SE)(void **result,
                                              void *non_leading) = NULL;
void (*non_leading_free)(void **non_leading) = NULL;
void (*leading_free)(void **leading) = NULL;

//{{{ uint32_t giggle_insert(struct bpt_node **root,
uint32_t giggle_insert(uint32_t *root_id,
                       uint32_t start,
                       uint32_t end,
                       uint32_t id)
{
#if DEBUG
    fprintf(stderr, "giggle_insert\n");
#endif
    /*
     * BP: the set of all indexing points
     * t: is a time point
     * t-: the point in BP immediatly before t
     *   the point s.t. t- < t and there does not exist a point t_m in BP such
     *   that t- < t_m < t
     * t+: the point in BP immediatly after t
     *   the point s.t. t < t+ and there does not exist a point t_m in BP such
     *   that t < t_m < t+
     * B(t): is a bucket containing pointers to all object versions whose
     *   valid time contis the interval [t_i, t_i+ - 1] 
     * SA(t): is the set of object versions whose start time is t
     * SE(t): is the set of object versions whose end time is is t -1
     *
     * t_a <- e.start
     * t_b <- e.end + 1
     *
     * search for t_a
     *
     * if not found:
     *   add t_a
     *
     * if t_a is not a leading leaf node entry:
     *   add e into SA(t_a)
     *
     * search for t_b
     *
     * if not found:
     *   add t_b
     *
     * if t_b is not a leading leaf node entry:
     *   add e into SE(t_b)
     *
     * for each leading entry t_i = t_a...t_b of a leaf node
     *   add e to B(t_i)
     *
     *
     */

#if DEBUG
    fprintf(stderr, "%u %u\n", start, end);
#endif

    uint32_t start_leaf_id = 0;
    int start_pos;

    uint32_t start_id = bpt_find(*root_id,
                                 &start_leaf_id,
                                 &start_pos,
                                 start);

    if (start_id == 0) {
        void *d = new_non_leading();
        non_leading_SA_add_scalar(d, &id);
        uint32_t v_id;
        int pos;
        *root_id = bpt_insert_new_value(*root_id,
                                        start,
                                        d,
                                        non_leading_free,
                                        &v_id,
                                        &start_leaf_id,
                                        &start_pos);
    } else {
        void *start_v = cache.get(cache.cache, start_id);
        non_leading_SA_add_scalar(start_v, &id);
    }

    uint32_t end_leaf_id = 0;
    int end_pos;

    uint32_t end_id = bpt_find(*root_id,
                               &end_leaf_id,
                               &end_pos,
                               end + 1);

    if (end_id == 0) {
        void *d = new_non_leading();
        non_leading_SE_add_scalar(d, &id);

        uint32_t v_id;
        int pos;
        *root_id = bpt_insert_new_value(*root_id,
                                        end + 1,
                                        d,
                                        non_leading_free,
                                        &v_id,
                                        &end_leaf_id,
                                        &end_pos);
    } else {
        void *end_v = cache.get(cache.cache, end_id);
        non_leading_SE_add_scalar(end_v, &id);
    }

#if DEBUG
    fprintf(stderr, "s_id:%u e_id:%u\t", start_leaf_id, end_leaf_id);
#endif


    // For now we need to search to see which leaf the values ended up in
    // because it is possible that the leaf split on the second insert but both
    // keys ended up on the same leaf.  If they are differnet we just double
    // check to see that this is not the case.
    
    if (start_leaf_id != end_leaf_id) 
        start_leaf_id = bpt_find_leaf(*root_id, start);

#if DEBUG
    fprintf(stderr, "s_id:%u e_id:%u\n", start_leaf_id, end_leaf_id);
#endif

    if (start_leaf_id != end_leaf_id) {
        struct bpt_node *curr_leaf = cache.get(cache.cache, start_leaf_id);
        do {
            curr_leaf = cache.get(cache.cache, BPT_NEXT(curr_leaf));

            if (BPT_LEADING(curr_leaf) == 0) {
                void *d = new_leading();
                uint32_t v_id = cache.seen(cache.cache) + 1;
                cache.add(cache.cache, v_id, d, leading_free);
                leading_B_add_scalar(d, &id);
                BPT_LEADING(curr_leaf) = v_id; 
            } else {
                void *d = cache.get(cache.cache, BPT_LEADING(curr_leaf));
                leading_B_add_scalar(d, &id);
            }
        } while (BPT_ID(curr_leaf) != end_leaf_id);
    }

    return 0;
}
//}}}

//{{{ void *giggle_search(uint32_t root_id,
void *giggle_search(uint32_t root_id,
                    uint32_t start,
                    uint32_t end)
{
#if DEBUG
    fprintf(stderr, "giggle_search\n");
    fprintf(stderr, "start:%u\tend:%u\n", start, end);
#endif

    if (root_id == 0)
        return 0;

    uint32_t leaf_start_id;
    int pos_start_id;

    uint32_t nld_start_id = bpt_find(root_id,
                                     &leaf_start_id, 
                                     &pos_start_id,
                                     start);
    struct bpt_node *leaf_start = cache.get(cache.cache, leaf_start_id);
    if ((pos_start_id == 0) && (BPT_KEYS(leaf_start)[0] != start))
        pos_start_id = -1;

#if DEBUG
    fprintf(stderr, "pos_start_id:%d\t", pos_start_id);
#endif

    uint32_t leaf_end_id;
    int pos_end_id;

    void *r = NULL;

    uint32_t nld_end_id = bpt_find(root_id,
                                   &leaf_end_id, 
                                   &pos_end_id,
                                   end);

    struct bpt_node *leaf_end = cache.get(cache.cache, leaf_end_id);
    if ((pos_end_id == 0) && (BPT_KEYS(leaf_end)[0] != end))
        pos_end_id = -1;
    else if ( (pos_end_id >=0) && 
              (pos_end_id < BPT_NUM_KEYS(leaf_end)) &&
              (BPT_KEYS(leaf_end)[pos_end_id] > end))
        pos_end_id -= 1;

#if DEBUG
    fprintf(stderr, "pos_end_id:%d %u\n", pos_end_id,
            ( ((pos_end_id >=0)&&(pos_end_id<BPT_NUM_KEYS(leaf_end))) ?
              BPT_KEYS(leaf_end)[pos_end_r] : 0)
            );
#endif

    // get everything in the leading value
    if (BPT_LEADING(leaf_start) != 0) {
        struct uint32_t_ll_bpt_leading_data *ld = 
                cache.get(cache.cache, BPT_LEADING(leaf_start));
        leading_union_with_B(&r, ld);
    }

    // add any SA and remove any that are an SE up to and including this point
    int i;
    for (i = 0; (i < BPT_NUM_KEYS(leaf_start)) && (i <= pos_start_id); ++i) {
        struct uint32_t_ll_bpt_non_leading_data *nld = 
                cache.get(cache.cache, BPT_POINTERS(leaf_start)[i]);
        non_leading_union_with_SA_subtract_SE(&r, nld);
    }

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start;
    int pos_curr_id = pos_start_id + 1;

    // any intermediate leaves
    while (BPT_ID(leaf_curr) != leaf_end_id) {
        // do from pos_curr to the last key
        for (i = pos_curr_id; i < BPT_NUM_KEYS(leaf_curr); ++i) {
            struct uint32_t_ll_bpt_non_leading_data *nld = 
                    cache.get(cache.cache, BPT_POINTERS(leaf_curr)[i]);
            non_leading_union_with_SA(&r, nld);
        }

        leaf_curr = cache.get(cache.cache, BPT_NEXT(leaf_curr));
        pos_curr_id = 0;
    }

    if (BPT_ID(leaf_curr) == leaf_end_id) {
        // add all SA's from here to either the end point
        for ( i = pos_curr_id;
             (i < BPT_NUM_KEYS(leaf_curr)) && (i <= pos_end_id); 
              ++i) {
            struct uint32_t_ll_bpt_non_leading_data *nld = 
                    cache.get(cache.cache, BPT_POINTERS(leaf_curr)[i]);
            non_leading_union_with_SA(&r, nld);
        }
    }

    /*
    if (r != NULL) {
        uint32_t v_id = cache.seen(cache.cache) + 1;
        cache.add(cache.cache, v_id, r, uint32_t_ll_free);
        return v_id;
    } else {
        return 0;
    }
    */
    return r;
}
//}}}

//{{{struct giggle_index *giggle_init_index(uint32_t init_size);
struct giggle_index *giggle_init_index(uint32_t init_size)
{
    struct giggle_index *gi = (struct giggle_index *)
            malloc(sizeof(struct giggle_index));

    gi->len = init_size;

    gi->num = 0;

    gi->root_ids = (uint32_t *)calloc(sizeof(uint32_t), gi->len);

    gi->chrm_index = ordered_set_init(init_size,
                                      str_uint_pair_sort_element_cmp,
                                      str_uint_pair_search_element_cmp,
                                      str_uint_pair_search_key_cmp);

    gi->file_index = unordered_list_init(3);

    gi->offset_index = unordered_list_init(1000);

    cache.cache = cache.init(CACHE_SIZE, NULL);

    return gi;
}
//}}}

//{{{void giggle_index_destroy(struct giggle_index **gi)
void giggle_index_destroy(struct giggle_index **gi)
{
    free((*gi)->root_ids);
    unordered_list_destroy(&((*gi)->file_index), free_wrapper);
    unordered_list_destroy(&((*gi)->offset_index), free_wrapper);
    ordered_set_destroy(&((*gi)->chrm_index), str_uint_pair_free);
    cache.destroy(&(cache.cache));
    free(*gi);
    *gi = NULL;
}
//}}}

//{{{int giggle_get_chrm_id(struct giggle_index *gi, char *chrm)
uint32_t giggle_get_chrm_id(struct giggle_index *gi, char *chrm)
{

    struct str_uint_pair *r = (struct str_uint_pair *)
            ordered_set_get(gi->chrm_index, chrm);

    if (r == NULL) {
        struct str_uint_pair *p = (struct str_uint_pair *)
                malloc(sizeof(struct str_uint_pair));
        p->uint = gi->chrm_index->num;
        p->str = strdup(chrm);

        r = (struct str_uint_pair *) ordered_set_add(gi->chrm_index, p);

        gi->num += 1;

        if (gi->len < gi->chrm_index->size) {
            gi->root_ids = realloc(gi->root_ids,
                                   gi->chrm_index->size*sizeof(uint32_t));
            gi->len = gi->chrm_index->size;
            uint32_t i;
            for (i = gi->num; i < gi->len; ++i)
                gi->root_ids[i] = 0;
        }
    }

    return r->uint;
}
//}}}

//{{{uint32_t giggle_index_file(struct giggle_index *gi,
uint32_t giggle_index_file(struct giggle_index *gi,
                           char *file_name)
{
    struct input_file *i = input_file_init(file_name);
    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;

    uint32_t file_id = unordered_list_add(gi->file_index, strdup(file_name));

    uint32_t j = 0;

    struct file_id_offset_pair *p;
    uint32_t intrv_id;

    while (input_file_get_next_interval(i,
                                        &chrm,
                                        &chrm_len,
                                        &start,
                                        &end,
                                        &offset) >= 0) {
        
        p = (struct file_id_offset_pair *)
                malloc(sizeof(struct file_id_offset_pair));
        p->offset = offset;
        p->file_id = file_id;
        intrv_id = unordered_list_add(gi->offset_index, p);
        uint32_t chrm_id = giggle_get_chrm_id(gi, chrm);
        uint32_t r = giggle_insert(&(gi->root_ids[chrm_id]),
                                   start,
                                   end,
                                   intrv_id);
        j += 1;
    }

    input_file_destroy(&i);
    free(chrm);
    return(j);
}
//}}}

//{{{void giggle_query_region(struct giggle_index *gi,
void *giggle_query_region(struct giggle_index *gi,
                          char *chrm,
                          uint32_t start,
                          uint32_t end)
{
    uint32_t chr_id = giggle_get_chrm_id(gi, chrm);
    struct uint32_t_ll *R = (struct uint32_t_ll *)
            giggle_search(gi->root_ids[chr_id], start, end);
    return R;
}
//}}}

//{{{uint32_t giggle_index_directory(struct giggle_index *gi,
uint32_t giggle_index_directory(struct giggle_index *gi,
                                char *path_name,
                                int verbose)
{
    glob_t results;
    int ret = glob(path_name, 0, NULL, &results);
    if (ret != 0)
        fprintf(stderr,
                "Problem with %s (%s), stopping early\n",
                path_name,
                /* ugly: */ (ret == GLOB_ABORTED ? "filesystem problem" :
                ret == GLOB_NOMATCH ? "no match of pattern" :
                ret == GLOB_NOSPACE ? "no dynamic memory" :
                "unknown problem"));

    int i;
    uint32_t total = 0;
    for (i = 0; i < results.gl_pathc; i++) {
        if (verbose)
            fprintf(stderr, "%s\n", results.gl_pathv[i]);
        total += giggle_index_file(gi, results.gl_pathv[i]);
    }

    globfree(&results);

    return total;
}
//}}}
