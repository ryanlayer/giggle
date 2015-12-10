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

//{{{ uint32_t giggle_insert(struct bpt_node **root,
uint32_t giggle_insert(struct bpt_node **root,
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


    struct bpt_node *start_leaf_r = NULL;
    int start_pos_r;

    void *start_r = bpt_find(*root,
                             &start_leaf_r,
                             &start_pos_r,
                             start);

    if (start_r == NULL) {
        void *d = new_non_leading();
        non_leading_SA_add_scalar(d, &id);
        *root = bpt_insert(*root, start, d, &start_leaf_r, &start_pos_r);
    } else {
        non_leading_SA_add_scalar(start_r, &id);
    }

    struct bpt_node *end_leaf_r = NULL;
    int end_pos_r;

    void *end_r = bpt_find(*root,
                           &end_leaf_r,
                           &end_pos_r,
                           end + 1);

    if (end_r == NULL) {
        void *d = new_non_leading();
        non_leading_SE_add_scalar(d, &id);
        *root = bpt_insert(*root, end + 1, d, &end_leaf_r, &end_pos_r);
    } else {
        non_leading_SE_add_scalar(end_r, &id);
    }

#if DEBUG
    fprintf(stderr, "s:%p e:%p\t", start_leaf_r, end_leaf_r);
#endif


    // For now we need to search to see which leaf the values ended up in
    // because it is possible that the leaf split on the second insert but both
    // keys ended up on the same leaf.  If they are differnet we just double
    // check to see that this is not the case.
    if (start_leaf_r != end_leaf_r) 
        start_leaf_r = bpt_find_leaf(*root, start);

#if DEBUG
    fprintf(stderr, "s:%p e:%p\n", start_leaf_r, end_leaf_r);
#endif

    if (start_leaf_r != end_leaf_r) {
        struct bpt_node *curr_leaf = start_leaf_r;
        do {
            curr_leaf = curr_leaf->next;

            if (curr_leaf->leading == NULL) {
                void *d = new_leading();
                leading_B_add_scalar(d, &id);
            } else {
                leading_B_add_scalar(curr_leaf->leading, &id);
            }
        } while (curr_leaf != end_leaf_r);
    }

    return 0;
}
//}}}

//{{{struct uint32_t_ll *giggle_search(struct bpt_node *root,
void *giggle_search(struct bpt_node *root,
                    uint32_t start,
                    uint32_t end)
{
#if DEBUG
    fprintf(stderr, "giggle_search\n");
    fprintf(stderr, "start:%u\tend:%u\n", start, end);
#endif

    if (root == NULL)
        return NULL;

    void *r = NULL;

    struct bpt_node *leaf_start_r, *leaf_end_r;
    int pos_start_r, pos_end_r;

    void *nld_start_r = bpt_find(root,
                                 &leaf_start_r, 
                                 &pos_start_r,
                                 start);
    if ((pos_start_r == 0) && (leaf_start_r->keys[0] != start))
        pos_start_r = -1;

#if DEBUG
    fprintf(stderr, "pos_start_r:%d\t", pos_start_r);
#endif

    void *nld_end_r = bpt_find(root,
                               &leaf_end_r, 
                               &pos_end_r,
                               end);

    if ((pos_end_r == 0) && (leaf_end_r->keys[0] != end))
        pos_end_r = -1;
    else if ( (pos_end_r >=0) && 
              (pos_end_r < leaf_end_r->num_keys) &&
              (leaf_end_r->keys[pos_end_r] > end))
        pos_end_r -= 1;

#if DEBUG
    fprintf(stderr, "pos_end_r:%d %u\n", pos_end_r,
            ( ((pos_end_r >=0)&&(pos_end_r<leaf_end_r->num_keys)) ?
              leaf_end_r->keys[pos_end_r] : 0)
            );
#endif

    // get everything in the leading value
    if (leaf_start_r->leading != NULL)
        leading_union_with_B(&r, leaf_start_r->leading);

    // add any SA and remove any that are an SE up to and including this point
    int i;
    for (i = 0; (i < leaf_start_r->num_keys) && (i <= pos_start_r); ++i) {
        non_leading_union_with_SA_subtract_SE(&r,
                                              leaf_start_r->pointers[i]);
    }

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start_r;
    int pos_curr = pos_start_r + 1;

    // any intermediate leaves
    while (leaf_curr != leaf_end_r) {
        // do from pos_curr to the last key
        for (i = pos_curr; i < leaf_curr->num_keys; ++i) {
            non_leading_union_with_SA(&r,
                                      leaf_curr->pointers[i]);
        }

        leaf_curr = leaf_curr->next;
        pos_curr = 0;
    }

    if (leaf_curr == leaf_end_r) {
        // add all SA's from here to either the end point
        for ( i = pos_curr;
             (i < leaf_curr->num_keys) && (i <= pos_end_r); 
              ++i) {
            non_leading_union_with_SA(&r,
                                      leaf_curr->pointers[i]);
        }
    }

    return (struct uint32_t_ll *)r;
}
//}}}

//{{{struct giggle_index *giggle_init_index(uint32_t init_size);
struct giggle_index *giggle_init_index(uint32_t init_size)
{
    struct giggle_index *gi = (struct giggle_index *)
            malloc(sizeof(struct giggle_index));
    gi->len = init_size;
    gi->num = 0;

    gi->roots = (struct bpt_node **)malloc(gi->len * sizeof(struct bpt_node *));

    gi->chrm_index = ordered_set_init(init_size,
                                      str_uint_pair_sort_element_cmp,
                                      str_uint_pair_search_element_cmp,
                                      str_uint_pair_search_key_cmp);

    gi->file_index = unordered_list_init(3);

    gi->offset_index = unordered_list_init(1000);

    int i;
    for (i = 0; i < gi->len; ++i)
        gi->roots[i] = NULL;

    return gi;
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
            gi->roots = realloc(gi->roots,
                                gi->chrm_index->size*sizeof(struct bpt_node *));
            gi->len = gi->chrm_index->size;
            uint32_t i;
            for (i = gi->num; i < gi->len; ++i)
                gi->roots[i] = NULL;
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
        uint32_t r = giggle_insert(&(gi->roots[chrm_id]), start, end, intrv_id);
        j += 1;
    }

    input_file_destroy(&i);

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
            giggle_search(gi->roots[chr_id], start, end);
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
