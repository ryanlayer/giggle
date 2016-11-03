#define _GNU_SOURCE

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <glob.h>
#include <sysexits.h>
#include <inttypes.h>

#include "bpt.h"
#include "cache.h"
#include "giggle_index.h"
#include "ll.h"
#include "lists.h"
#include "file_read.h"
#include "util.h"
#include "timer.h"
#include "fastlz.h"
#include "jsw_avltree.h"

//{{{ void *file_id_offset_pair_load(FILE *f, char *file_name)
void *file_id_offset_pair_load(FILE *f, char *file_name)
{
    struct file_id_offset_pair *p = (struct file_id_offset_pair*)
            malloc(sizeof( struct file_id_offset_pair));

    size_t fr = fread(&(p->file_id), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    fr = fread(&(p->offset), sizeof(long), 1, f);
    check_file_read(file_name, f, 1, fr);

    return p;
}
//}}}

//{{{void file_id_offset_pair_store(void *v, FILE *f, char *file_name)
void file_id_offset_pair_store(void *v, FILE *f, char *file_name)
{
    struct file_id_offset_pair *p = (struct file_id_offset_pair*)v;

    if (fwrite(&(p->file_id), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing file_id_offset_pair file_id '%s'.", file_name);

    if (fwrite(&(p->offset), sizeof(long), 1, f) != 1)
        err(EX_IOERR,
            "Error writing file_id_offset_pair offset '%s'.", file_name);
}
//}}}

//{{{ void *c_str_load(FILE *f, char *file_name)
void *c_str_load(FILE *f, char *file_name)
{
    uint32_t size;
    size_t fr = fread(&size, sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    char *c_str = (char *)calloc(size, sizeof(char));

    fr = fread(c_str, sizeof(char), size, f);
    check_file_read(file_name, f, size, fr);

    return c_str;
}
//}}}

//{{{void c_str_store(void *v, FILE *f, char *file_name)
void c_str_store(void *v, FILE *f, char *file_name)
{
    char *c_str = (char *)v;
    uint32_t size = strlen(c_str);

    if (fwrite(&size, sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing c_str size '%s'.", file_name);

    if (fwrite(c_str, sizeof(char), size, f) != size)
        err(EX_IOERR,
            "Error writing file_id_offset_pair offset '%s'.", file_name);
}
//}}}

//{{{ uint32_t giggle_insert(struct bpt_node **root,
uint32_t giggle_insert(uint32_t domain,
                       uint32_t *root_id,
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

    uint32_t end_leaf_id = 0;
    int end_pos;

    uint32_t end_id = bpt_find(domain,
                               *root_id,
                               &end_leaf_id,
                               &end_pos,
                               end + 1);

    if (end_id == 0) {
#if DEBUG
        fprintf(stderr ,"->%u\n", end);
#endif
        void *d = giggle_data_handler.new_non_leading(domain);
        giggle_data_handler.non_leading_SE_add_scalar(domain, d, &id);

        uint32_t v_id;
        int pos;
        *root_id = bpt_insert_new_value(
                domain,
                *root_id,
                end + 1,
                d,
                &giggle_data_handler.non_leading_cache_handler,
                &v_id,
                &end_leaf_id,
                &end_pos);
    } else {
#if DEBUG
        fprintf(stderr ,"||%u\n", end);
#endif
        void *end_v = cache.get(domain,
                                end_id - 1,
                                &giggle_data_handler.non_leading_cache_handler);
        giggle_data_handler.non_leading_SE_add_scalar(domain, end_v, &id);
    }

    uint32_t start_leaf_id = 0;
    int start_pos;

    uint32_t start_id = bpt_find(domain,
                                 *root_id,
                                 &start_leaf_id,
                                 &start_pos,
                                 start);

    if (start_id == 0) {
#if DEBUG
        fprintf(stderr ,"->%u\n", start);
#endif
        void *d = giggle_data_handler.new_non_leading(domain);
        giggle_data_handler.non_leading_SA_add_scalar(domain, d, &id);
        uint32_t v_id;
        int pos;
        *root_id = bpt_insert_new_value(
                domain,
                *root_id,
                start,
                d,
                &giggle_data_handler.non_leading_cache_handler,
                &v_id,
                &start_leaf_id,
                &start_pos);
    } else {
#if DEBUG
        fprintf(stderr ,"||%u\n", start);
#endif
        void *start_v = cache.get(
                domain,
                start_id - 1,
                &giggle_data_handler.non_leading_cache_handler);
        giggle_data_handler.non_leading_SA_add_scalar(domain, start_v, &id);
    }


#if DEBUG
    fprintf(stderr, "s_id:%u e_id:%u\t", start_leaf_id, end_leaf_id);
#endif


    // For now we need to search to see which leaf the values ended up in
    // because it is possible that the leaf split on the second insert but both
    // keys ended up on the same leaf.  If they are differnet we just double
    // check to see that this is not the case.
    
    //if (start_leaf_id != end_leaf_id) 
    start_leaf_id = bpt_find_leaf(domain, *root_id, start);
    end_leaf_id = bpt_find_leaf(domain, *root_id, end + 1);

#if DEBUG
    fprintf(stderr, ":::\ts_id:%u e_id:%u\n", start_leaf_id, end_leaf_id);
#endif

#if DEBUG
    struct bpt_node *test_leaf = cache.get(domain,
                                           start_leaf_id - 1,
                                           &bpt_node_cache_handler);
    uint32_t x;
    for (x = 0; x < BPT_NUM_KEYS(test_leaf); ++x) {
        fprintf(stderr, "%u ", BPT_KEYS(test_leaf)[x]);
    }
    fprintf(stderr, "\n");
#endif

    if (start_leaf_id != end_leaf_id) {
        struct bpt_node *curr_leaf = cache.get(domain,
                                               start_leaf_id - 1,
                                               &bpt_node_cache_handler);
        do {
            curr_leaf = cache.get(domain,
                                  BPT_NEXT(curr_leaf) - 1,
                                  &bpt_node_cache_handler);

            if (BPT_LEADING(curr_leaf) == 0) {
                void *d = giggle_data_handler.new_leading(domain);
                uint32_t v_id = cache.seen(domain) + 1;
                cache.add(domain,
                          v_id - 1,
                          d,
                          &giggle_data_handler.leading_cache_handler);
                giggle_data_handler.leading_B_add_scalar(domain, d, &id);
                BPT_LEADING(curr_leaf) = v_id; 
            } else {
                void *d = cache.get(domain,
                                    BPT_LEADING(curr_leaf) - 1,
                                    &giggle_data_handler.leading_cache_handler);
                giggle_data_handler.leading_B_add_scalar(domain, d, &id);
            }
        } while (BPT_ID(curr_leaf) != end_leaf_id);
    }

    return 0;
}
//}}}

//{{{ void *giggle_search(uint32_t domain,
void *giggle_search(uint32_t domain,
                    uint32_t root_id,
                    uint32_t start,
                    uint32_t end)
{
#if DEBUG_GIGGLE_SEARCH
    fprintf(stderr, "giggle_search\n");
    fprintf(stderr, "start:%u\tend:%u\n", start, end);
#endif

    if (root_id == 0)
        return 0;

    uint32_t leaf_start_id;
    int pos_start_id;

    uint32_t nld_start_id = bpt_find(domain,
                                     root_id,
                                     &leaf_start_id, 
                                     &pos_start_id,
                                     start);
    struct bpt_node *leaf_start = cache.get(domain,
                                            leaf_start_id - 1,
                                            &bpt_node_cache_handler);
    if ((pos_start_id == 0) && (BPT_KEYS(leaf_start)[0] != start))
        pos_start_id = -1;
    else if ( (pos_start_id >=0) && 
              (pos_start_id < BPT_NUM_KEYS(leaf_start)) &&
              (BPT_KEYS(leaf_start)[pos_start_id] > start))
        pos_start_id -= 1;


#if DEBUG_GIGGLE_SEARCH
    fprintf(stderr,
            "leaf_start_id:%u\tpos_start_id:%d\n",
            leaf_start_id,
            pos_start_id);
#endif

    uint32_t leaf_end_id;
    int pos_end_id;

    uint32_t nld_end_id = bpt_find(domain,
                                   root_id,
                                   &leaf_end_id, 
                                   &pos_end_id,
                                   end);

    struct bpt_node *leaf_end = cache.get(domain,
                                          leaf_end_id - 1,
                                          &bpt_node_cache_handler);

    if ((pos_end_id == 0) && (BPT_KEYS(leaf_end)[0] != end))
        pos_end_id = -1;
    else if ( (pos_end_id >=0) && 
              (pos_end_id < BPT_NUM_KEYS(leaf_end)) &&
              (BPT_KEYS(leaf_end)[pos_end_id] > end))
        pos_end_id -= 1;

#if DEBUG_GIGGLE_SEARCH
    fprintf(stderr,
            "leaf_end_id:%u\tpos_end_id:%u\t\n",
            leaf_end_id,
            pos_end_id);
#endif

#if DEBUG_GIGGLE_SEARCH
    fprintf(stderr, "pos_end_id:%d %u\n", pos_end_id,
            ( ((pos_end_id >=0)&&(pos_end_id<BPT_NUM_KEYS(leaf_end))) ?
              BPT_KEYS(leaf_end)[pos_end_id] : 0)
            );
#endif

    //if ((leaf_start_id == leaf_end_id) && (pos_start_id >= pos_end_id))
    if ((leaf_start_id == leaf_end_id) && (pos_start_id > pos_end_id))
        return NULL;

#if DEBUG_GIGGLE_SEARCH
    if (BPT_LEADING(leaf_start) == 0)
        fprintf(stderr, "BPT_LEADING(leaf_start) == 0\n");
#endif

    return giggle_data_handler.
            giggle_collect_intersection(leaf_start_id,
                                        pos_start_id,
                                        leaf_end_id,
                                        pos_end_id,
                                        domain,
                                        NULL); 
}
//}}}

//{{{void *giggle_collect_intersection_data_in_block(uint32_t leaf_start_id,
void *giggle_collect_intersection_data_in_block(uint32_t leaf_start_id,
                                                int pos_start_id,
                                                uint32_t leaf_end_id,
                                                int pos_end_id,
                                                uint32_t domain,
                                                void **r)
{
#if DEBUG
    fprintf(stderr, "giggle_collect_intersection_data_in_block\n");
#endif

    uint32_t I_size =
            giggle_leaf_data_get_intersection_size(leaf_start_id,
                                                   pos_start_id,
                                                   leaf_end_id,
                                                   pos_end_id,
                                                   domain);

#if DEBUG
    fprintf(stderr, "I_size:%u\n", I_size);
#endif

    uint32_t *I = (uint32_t *)calloc(I_size, sizeof(uint32_t));

    struct bpt_node *leaf_start = cache.get(domain,
                                            leaf_start_id - 1,
                                            &bpt_node_cache_handler);
    struct leaf_data *leaf_start_data = 
            cache.get(domain,
                      BPT_POINTERS_BLOCK(leaf_start) - 1,
                      &leaf_data_cache_handler);

    // get everything in the leading value

    // The first step is to take the leading and the starts up to 
    // and including pos_start_id and remove ends up to and including 
    // pos_start_id
    uint32_t buff_size = leaf_start_data->num_leading +
            LEAF_DATA_STARTS_END(leaf_start, pos_start_id) +
            LEAF_DATA_ENDS_END(leaf_start,  pos_start_id);

    uint32_t *buff = (uint32_t *)calloc(buff_size, sizeof(uint32_t));

    memcpy(buff,
           leaf_start_data->leading,
           leaf_start_data->num_leading * sizeof(uint32_t));

    memcpy(buff + leaf_start_data->num_leading,
           leaf_start_data->starts,
           LEAF_DATA_STARTS_END(leaf_start, pos_start_id)*sizeof(uint32_t));

    memcpy(buff + leaf_start_data->num_leading + 
                LEAF_DATA_STARTS_END(leaf_start, pos_start_id),
           leaf_start_data->ends,
           LEAF_DATA_ENDS_END(leaf_start, pos_start_id)*sizeof(uint32_t));

    qsort(buff, buff_size, sizeof(uint32_t), uint32_t_cmp);

    uint32_t i, I_i = 0;
    for (i = 0; i < buff_size; ++i) {
        if ( ((i + 1) == buff_size) || (buff[i] != buff[i+1]))
            I[I_i++] =  buff[i];
        else
            i+=1;
    }
    free(buff);

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start;
    int pos_curr_id = pos_start_id + 1;
    struct leaf_data *ld;
    uint32_t curr_size;

    // any intermediate leaves
    while (BPT_ID(leaf_curr) != leaf_end_id) {
        // do from pos_curr to the last key
        curr_size = LEAF_DATA_STARTS_END(leaf_curr,
                                         BPT_NUM_KEYS(leaf_curr) - 1) -
                     LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id);

        ld = cache.get(domain,
                       BPT_POINTERS_BLOCK(leaf_curr) - 1,
                       &leaf_data_cache_handler);

        memcpy(I + I_i,
               ld->starts + LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id),
               curr_size * sizeof(uint32_t));

        I_i += curr_size;

        leaf_curr = cache.get(domain,
                              BPT_NEXT(leaf_curr) - 1,
                              &bpt_node_cache_handler);
        pos_curr_id = 0;
    }

    ld = cache.get(domain,
                   BPT_POINTERS_BLOCK(leaf_curr) - 1,
                   &leaf_data_cache_handler);


    curr_size = LEAF_DATA_STARTS_END(leaf_curr, pos_end_id) - 
                LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id);

    memcpy(I + I_i,
           ld->starts + LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id),
           curr_size * sizeof(uint32_t));

    struct leaf_data_result *ldr = (struct leaf_data_result *)
            calloc(1,sizeof(struct leaf_data_result));

    ldr->len = I_size;
    ldr->data = I;
    ldr->next = NULL;

    if ((r == NULL) || (*r == NULL)) {
        return ldr;
    } else {
        ((struct leaf_data_result *)*r)->next = ldr;
        //*r = ldr;
    }

    return ldr;
}
//}}}

//{{{uint32_t giggle_leaf_data_get_intersection_size(uint32_t leaf_start_id,
uint32_t giggle_leaf_data_get_intersection_size(uint32_t leaf_start_id,
                                                int pos_start_id,
                                                uint32_t leaf_end_id,
                                                int pos_end_id,
                                                uint32_t domain)
{
#if DEBUG
    fprintf(stderr, "giggle_leaf_data_get_intersection_size\n");
    fprintf(stderr,
            "leaf_start_id:%u\t"
            "pos_start_id:%d\t"
            "leaf_end_id:%u\t"
            "pos_end_id:%d\t"
            "domain:%u\n",
            leaf_start_id,
            pos_start_id,
            leaf_end_id,
            pos_end_id,
            domain);
#endif

    struct bpt_node *leaf_start = cache.get(domain,
                                            leaf_start_id - 1,
                                            &bpt_node_cache_handler);
    struct leaf_data *leaf_start_data = 
            cache.get(domain,
                      BPT_POINTERS_BLOCK(leaf_start) - 1,
                      &leaf_data_cache_handler);

    // Find sizes
    // get everything in the leading value
    uint32_t i_size = leaf_start_data->num_leading;
#if DEBUG
    fprintf(stderr,
            "leaf_start_data->num_leading:%u\t"
            "->num_starts:%u\t"
            "->num_ends:%u\n",
            leaf_start_data->num_leading,
            leaf_start_data->num_starts,
            leaf_start_data->num_ends);
#endif
//
    uint32_t i;

    i_size += LEAF_DATA_STARTS_END(leaf_start, pos_start_id);
    i_size -= LEAF_DATA_ENDS_END(leaf_start,  pos_start_id);

#if DEBUG
    fprintf(stderr,
            "LEAF_DATA_STARTS_END(leaf_start, pos_start_id):%u\t"
            "LEAF_DATA_ENDS_END(leaf_start,  pos_start_id):%u\n",
            LEAF_DATA_STARTS_END(leaf_start, pos_start_id),
            LEAF_DATA_ENDS_END(leaf_start,  pos_start_id));
            
    fprintf(stderr,
            "i_size:%u\n",
            i_size);
#endif

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start;
    int pos_curr_id = pos_start_id + 1;

    // any intermediate leaves
    while (BPT_ID(leaf_curr) != leaf_end_id) {
#if DEBUG
        fprintf(stderr,
                "leaf_curr:%u\t"
                "BPT_NUM_KEYS(leaf_curr):%u\t"
                "pos_curr_id:%u\t"
                "LEAF_DATA_STARTS_END(leaf_curr,BPT_NUM_KEYS(leaf_curr)-1):%u\t"
                "LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id):%u\t%u\n",
                BPT_ID(leaf_curr),
                BPT_NUM_KEYS(leaf_curr),
                pos_curr_id,
                LEAF_DATA_STARTS_END(leaf_curr,BPT_NUM_KEYS(leaf_curr)-1),
                LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id),
                LEAF_DATA_STARTS_END(leaf_curr,BPT_NUM_KEYS(leaf_curr)-1) -
                LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id));
#endif

        // do from pos_curr to the last key
        i_size += LEAF_DATA_STARTS_END(leaf_curr,
                                       BPT_NUM_KEYS(leaf_curr) - 1) -
                  LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id);

#if DEBUG
        fprintf(stderr,
                "i_size:%u\n",
                i_size);
#endif

        leaf_curr = cache.get(domain,
                              BPT_NEXT(leaf_curr) - 1,
                              &bpt_node_cache_handler);
        pos_curr_id = 0;
    }

    i_size += LEAF_DATA_STARTS_END(leaf_curr, pos_end_id) - 
                  LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id);

#if DEBUG
    fprintf(stderr,
            "pos_end_id:%d BPT_POINTERS(node)[i]:%u\tBPT_NUM_KEYS(node)%u\n",
            pos_end_id,
            BPT_POINTERS(leaf_curr)[pos_end_id],
            BPT_NUM_KEYS(leaf_curr));

    fprintf(stderr,
            "LEAF_DATA_STARTS_END(leaf_curr, pos_end_id):%u\t"
            "LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id):%u\t%u\n",
            LEAF_DATA_STARTS_END(leaf_curr, pos_end_id),
            LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id),
            LEAF_DATA_STARTS_END(leaf_curr, pos_end_id) - 
            LEAF_DATA_STARTS_START(leaf_curr, pos_curr_id));

    fprintf(stderr,
            "i_size:%u\n",
            i_size);
#endif
    return i_size;
}
//}}}

//{{{void *giggle_collect_intersection_data_in_pointers(uint32_t
void *giggle_collect_intersection_data_in_pointers(uint32_t leaf_start_id,
                                                   int pos_start_id,
                                                   uint32_t leaf_end_id,
                                                   int pos_end_id,
                                                   uint32_t domain,
                                                   void **_r)
{
    void *r = NULL;
#if DEBUG
    fprintf(stderr, "giggle_collect_intersection_data_in_pointers\n");
#endif

    struct bpt_node *leaf_start = cache.get(domain,
                                            leaf_start_id - 1,
                                            &bpt_node_cache_handler);

    // get everything in the leading value
    if (BPT_LEADING(leaf_start) != 0) {
        void *ld = cache.get(domain,
                             BPT_LEADING(leaf_start) - 1,
                             &giggle_data_handler.leading_cache_handler);

        giggle_data_handler.leading_union_with_B(domain, &r, ld);
    }

    // add any SA and remove any that are an SE up to and including this point
    int i;
    for (i = 0; (i < BPT_NUM_KEYS(leaf_start)) && (i <= pos_start_id); ++i) {
#if DEBUG
        fprintf(stderr,
                "BPT_KEY(leaf_start)[%u] == %u\n",
                i,
                BPT_KEYS(leaf_start)[i]);
#endif
        void *nld = cache.get(domain,
                              BPT_POINTERS(leaf_start)[i] - 1,
                              &giggle_data_handler.non_leading_cache_handler);
        giggle_data_handler.
                non_leading_union_with_SA_subtract_SE(domain,&r, nld);
    }

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start;
    int pos_curr_id = pos_start_id + 1;

    // any intermediate leaves
    while (BPT_ID(leaf_curr) != leaf_end_id) {
        // do from pos_curr to the last key
        for (i = pos_curr_id; i < BPT_NUM_KEYS(leaf_curr); ++i) {
            void *nld = cache.get(
                    domain,
                    BPT_POINTERS(leaf_curr)[i] - 1,
                    &giggle_data_handler.non_leading_cache_handler);
            giggle_data_handler.non_leading_union_with_SA(domain, &r, nld);
        }

        leaf_curr = cache.get(domain,
                              BPT_NEXT(leaf_curr) - 1,
                              &bpt_node_cache_handler);
        pos_curr_id = 0;
    }

    if (BPT_ID(leaf_curr) == leaf_end_id) {
        // add all SA's from here to either the end point
        for ( i = pos_curr_id;
             (i < BPT_NUM_KEYS(leaf_curr)) && (i <= pos_end_id); 
              ++i) {
            void *nld = cache.get(
                    domain,
                    BPT_POINTERS(leaf_curr)[i] - 1,
                    &giggle_data_handler.non_leading_cache_handler);
            giggle_data_handler.non_leading_union_with_SA(domain, &r, nld);
        }
    }

    return r;
}
//}}}

//{{{struct giggle_index *giggle_init_index(uint32_t init_size);
struct giggle_index *giggle_init_index(uint32_t init_size)
{
    struct giggle_index *gi = (struct giggle_index *)
            malloc(sizeof(struct giggle_index));
    gi->data_dir = NULL;
    gi->len = init_size;
    gi->num = 0;
    gi->root_ids = (uint32_t *)calloc(sizeof(uint32_t), gi->len);
    gi->chrm_index = ordered_set_init(init_size,
                                      str_uint_pair_sort_element_cmp,
                                      str_uint_pair_search_element_cmp,
                                      str_uint_pair_search_key_cmp);

    gi->file_index = unordered_list_init(3);

    //gi->offset_index = unordered_list_init(1000);
    gi->offset_index = (struct file_id_offset_pairs *)
            malloc(sizeof(struct file_id_offset_pairs));
    gi->offset_index->num = 0;
    gi->offset_index->size = 1000;
    gi->offset_index->vals = (struct file_id_offset_pair *)
            calloc(gi->offset_index->size,
                   sizeof(struct file_id_offset_pair));

    gi->chrm_index_file_name = NULL;
    gi->file_index_file_name = NULL;
    gi->offset_index_file_name = NULL;
    gi->root_ids_file_name = NULL;

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

//{{{void giggle_index_destroy(struct giggle_index **gi)
void giggle_index_destroy(struct giggle_index **gi)
{
    if ((*gi)->chrm_index_file_name != NULL)
        free((*gi)->chrm_index_file_name);
    if ((*gi)->file_index_file_name != NULL)
        free((*gi)->file_index_file_name);
    if ((*gi)->offset_index_file_name != NULL)
        free((*gi)->offset_index_file_name);
    if ((*gi)->root_ids_file_name != NULL)
        free((*gi)->root_ids_file_name);
    if ((*gi)->data_dir != NULL)
        free((*gi)->data_dir);

    free((*gi)->root_ids);
    //unordered_list_destroy(&((*gi)->file_index), free_wrapper);
    unordered_list_destroy(&((*gi)->file_index), file_data_free);
    //unordered_list_destroy(&((*gi)->offset_index), free_wrapper);
    free((*gi)->offset_index->vals);
    free((*gi)->offset_index);
    ordered_set_destroy(&((*gi)->chrm_index), str_uint_pair_free);
    free(*gi);
    *gi = NULL;
}
//}}}

//{{{uint32_t giggle_index_file(struct giggle_index *gi,
uint32_t giggle_index_file(struct giggle_index *gi,
                           char *file_name)
{
    fprintf(stderr, "%s\n", file_name);
    struct input_file *i = input_file_init(file_name);
    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;

    //uint32_t file_id = unordered_list_add(gi->file_index, strdup(file_name));
    struct file_data *fd = (struct file_data *)
        calloc(1, sizeof(struct file_data));
    fd->file_name = strdup(file_name);
    //uint32_t file_id = unordered_list_add(gi->file_index, strdup(file_name));
    uint32_t file_id = unordered_list_add(gi->file_index, fd);

    uint32_t j = 0;

    struct file_id_offset_pair *p;
    uint32_t intrv_id;

    while (i->input_file_get_next_interval(i,
                                           &chrm,
                                           &chrm_len,
                                           &start,
                                           &end,
                                           &offset) >= 0) {
        //fprintf(stderr, "%s %u %u\n", chrm, start, end); 
/*
        p = (struct file_id_offset_pair *)
                malloc(sizeof(struct file_id_offset_pair));
        p->offset = offset;
        p->file_id = file_id;
        intrv_id = unordered_list_add(gi->offset_index, p);
*/
        intrv_id = gi->offset_index->num;
        gi->offset_index->num = gi->offset_index->num + 1;
        if (gi->offset_index->num == gi->offset_index->size) {
            gi->offset_index->size = gi->offset_index->size * 2;
            gi->offset_index->vals = (struct file_id_offset_pair *)
                realloc(gi->offset_index->vals,
                        gi->offset_index->size * 
                        sizeof(struct file_id_offset_pair));
            memset(gi->offset_index->vals + gi->offset_index->num,
                   0,
                   (gi->offset_index->size - gi->offset_index->num) *
                        sizeof(struct file_id_offset_pair));
        }
        gi->offset_index->vals[intrv_id].offset = offset;
        gi->offset_index->vals[intrv_id].file_id = file_id;


        uint32_t chrm_id = giggle_get_chrm_id(gi, chrm);
        uint32_t r = giggle_insert(chrm_id,
                                   &(gi->root_ids[chrm_id]),
                                   start,
                                   end,
                                   intrv_id);
        fd->mean_interval_size += end-start;
        fd->num_intervals += 1;
        j += 1;
    }
    fd->mean_interval_size = fd->mean_interval_size/fd->num_intervals;

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
    uint32_t off = 0;
    if (strncmp("chr", chrm, 3) == 0)
        off = 3;

    uint32_t chr_id = giggle_get_chrm_id(gi, chrm+off);
    return giggle_search(chr_id, gi->root_ids[chr_id], start, end);
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

//{{{struct giggle_index *giggle_init(uint32_t num_chrms)
struct giggle_index *giggle_init(uint32_t num_chrms,
                                 char *data_dir,
                                 uint32_t force,
                                 void (*giggle_set_data_handler)(void))
{

    char **cache_names = NULL;

    struct giggle_index *gi = giggle_init_index(num_chrms);

    if (data_dir != NULL) {
        gi->data_dir = strdup(data_dir);

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
            return NULL;
        }

        cache_names = (char **)calloc(num_chrms, sizeof(char *));
        uint32_t i, ret;
        for (i = 0; i < num_chrms; ++i) {
            ret = asprintf(&(cache_names[i]),
                           "%s/cache.%u",
                           data_dir,
                           i);
        }
    }

    struct simple_cache *sc = simple_cache_init(1000,
                                                num_chrms,
                                                cache_names);

    //uint32_t_ll_giggle_set_data_handler();
    giggle_set_data_handler();


    if (data_dir != NULL) {
        int ret = asprintf(&(gi->chrm_index_file_name),
                           "%s/chrm_index.dat",
                           data_dir);

        ret = asprintf(&(gi->file_index_file_name),
                       "%s/file_index.dat",
                       data_dir);

        ret = asprintf(&(gi->offset_index_file_name),
                       "%s/offset_index.dat",
                       data_dir);

        ret = asprintf(&(gi->root_ids_file_name),
                       "%s/root_ids.dat",
                       data_dir);
    }

    if (cache_names != NULL) {
        uint32_t i;
        for (i = 0; i < num_chrms; ++i)
            free(cache_names[i]);
        free(cache_names);
    }


    return gi;
}
//}}}

//{{{ uint32_t giggle_store(struct giggle_index *gi)
uint32_t giggle_store(struct giggle_index *gi)
{
    if (gi->chrm_index_file_name == NULL)
        return 1;

    uint32_t i;

    giggle_data_handler.write_tree(gi);

    FILE *f = fopen(gi->root_ids_file_name, "wb");

    if (fwrite(&(gi->len), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing len for root_ids'%s'.",
            gi->root_ids_file_name);

    if (fwrite(&(gi->num), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing num for root_ids'%s'.",
            gi->root_ids_file_name);

    if (fwrite(gi->root_ids, sizeof(uint32_t), gi->len, f) != gi->len)
        err(EX_IOERR, "Error writing root_ids '%s'.",
            gi->root_ids_file_name);
    fclose(f);

    f = fopen(gi->chrm_index_file_name, "wb");
    ordered_set_store(gi->chrm_index,
                      f,
                      gi->chrm_index_file_name,
                      str_uint_pair_store);
    fclose(f);

    f = fopen(gi->file_index_file_name, "wb");
    unordered_list_store(gi->file_index,
                         f,
                         gi->file_index_file_name,
                         file_data_store);
                         //c_str_store);
    fclose(f);

    f = fopen(gi->offset_index_file_name, "wb");
    /*
    unordered_list_store(gi->offset_index,
                         f,
                         gi->offset_index_file_name,
                         file_id_offset_pair_store);
    */
    if (fwrite(&(gi->offset_index->num),
               sizeof(uint64_t),1, f) != 1)
        err(EX_IOERR, "Error writing offset_index num to '%s'.",
            gi->offset_index_file_name);

    if (fwrite(gi->offset_index->vals, 
               sizeof(struct file_id_offset_pair), 
               gi->offset_index->num, f) != gi->offset_index->num)
        err(EX_IOERR, "Error writing file_id offset pairs to '%s'.",
            gi->offset_index_file_name);
    fclose(f);

    return 0;
}
//}}}

//{{{struct giggle_index *giggle_load(char *data_dir,
struct giggle_index *giggle_load(char *data_dir,
                                 void (*giggle_set_data_handler)(void))
{
    //ORDER = 100;

    if (data_dir == NULL)
        return NULL;

    struct stat st = {0};
    if (stat(data_dir, &st) == -1)
        return NULL;

    struct giggle_index *gi = (struct giggle_index *)
            malloc(sizeof(struct giggle_index));

    gi->data_dir = strdup(data_dir);

    // root_ids
#ifdef TIME
    struct timeval read_root_ids = in();
#endif
    int ret = asprintf(&(gi->root_ids_file_name),
                       "%s/root_ids.dat",
                       data_dir);

    FILE *f = fopen(gi->root_ids_file_name, "rb");
    if (f == NULL)
        err(1, "Could not open file '%s'.\n", gi->root_ids_file_name);

    size_t fr = fread(&(gi->len), sizeof(uint32_t), 1, f);
    check_file_read(gi->root_ids_file_name, f, 1, fr);

    fr = fread(&(gi->num), sizeof(uint32_t), 1, f);
    check_file_read(gi->root_ids_file_name, f, 1, fr);

    gi->root_ids = (uint32_t *)calloc(gi->len, sizeof(uint32_t));

    fr = fread(gi->root_ids, sizeof(uint32_t), gi->len, f);
    check_file_read(gi->root_ids_file_name, f, gi->len, fr);

    fclose(f);
#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread root_ids\t%lu\n",
            out(read_root_ids));
#endif

    // chrm_index
#ifdef TIME
    struct timeval read_chrm_index = in();
#endif
    ret = asprintf(&(gi->chrm_index_file_name),
                   "%s/chrm_index.dat",
                   data_dir);
    f = fopen(gi->chrm_index_file_name, "rb");
    gi->chrm_index = ordered_set_load(f,
                                      gi->chrm_index_file_name,
                                      str_uint_pair_load,
                                      str_uint_pair_sort_element_cmp,
                                      str_uint_pair_search_element_cmp,
                                      str_uint_pair_search_key_cmp);
    fclose(f);
#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread chrm_index\t%lu\n",
            out(read_chrm_index));
#endif

#ifdef TIME
    struct timeval read_file_index = in();
#endif
    ret = asprintf(&(gi->file_index_file_name),
                   "%s/file_index.dat",
                   data_dir);
    f = fopen(gi->file_index_file_name, "rb");
    gi->file_index = unordered_list_load(f,
                                       gi->file_index_file_name,
                                       file_data_load);
                                       //c_str_load);
    fclose(f);
#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread file_index\t%lu\n",
            out(read_file_index));
#endif

#ifdef TIME
    struct timeval read_offset_index = in();
#endif
    ret = asprintf(&(gi->offset_index_file_name),
                   "%s/offset_index.dat",
                   data_dir);

    f = fopen(gi->offset_index_file_name, "rb");
    /*
    gi->offset_index = unordered_list_load(f,
                                           gi->offset_index_file_name,
                                           file_id_offset_pair_load);

    */
    gi->offset_index = (struct file_id_offset_pairs *)
            malloc(sizeof(struct file_id_offset_pairs));
    fr = fread(&(gi->offset_index->num), sizeof(uint64_t), 1, f);
    check_file_read(gi->offset_index_file_name, f, 1, fr);
    gi->offset_index->size = gi->offset_index->num;
    gi->offset_index->vals = (struct file_id_offset_pair *)
            malloc(gi->offset_index->size * 
                   sizeof(struct file_id_offset_pair));
    fr = fread(gi->offset_index->vals,
               sizeof(struct file_id_offset_pair),
               gi->offset_index->num,
               f);
    check_file_read(gi->offset_index_file_name, f, gi->offset_index->num, fr);

    fclose(f);
#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread offset_index\t%lu\n",
            out(read_offset_index));
#endif


    //start();
    char **cache_names = (char **)calloc(gi->len, sizeof(char *));
    uint32_t i;
    for (i = 0; i < gi->len; ++i) {
        ret = asprintf(&(cache_names[i]),
                          "%s/cache.%u",
                          data_dir,
                          i);
    }

#ifdef TIME
    struct timeval load_simple_cache = in();
#endif
    struct simple_cache *sc = simple_cache_init(1000,
                                                gi->len,
                                                cache_names);
    for (i = 0; i < gi->len; ++i)
        free(cache_names[i]);
    free(cache_names);


#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread load_simple_cache\t%lu\n",
            out(load_simple_cache));
#endif

    giggle_set_data_handler();

    return gi;
}
//}}}

//{{{struct giggle_query_result *giggle_query(struct giggle_index *gi,
struct giggle_query_result *giggle_query(struct giggle_index *gi,
                                        char *chrm,
                                        uint32_t start,
                                        uint32_t end,
                                        struct giggle_query_result *_gqr)
{
#if DEBUG_GIGGLE_QUERY
    fprintf(stderr, "giggle_query\t%s\t%u\t%u\n", chrm, start, end);
#endif

    uint32_t off = 0;
    if (strncmp("chr", chrm, 3) == 0)
        off = 3;

    uint32_t chr_id = giggle_get_chrm_id(gi, chrm + off);
    
    // HERE R COULD BE A LIST
    void *R = giggle_search(chr_id,
                            gi->root_ids[chr_id],
                            start,
                            end);


    uint32_t i,j;
    struct giggle_query_result *gqr;
    if (_gqr == NULL) {
        gqr = (struct giggle_query_result *) 
                malloc(sizeof(struct giggle_query_result));

        gqr->gi = gi;
        gqr->num_files = gi->file_index->num;
        gqr->num_hits = 0;
        gqr->offsets = (struct long_ll **)
            calloc(gi->file_index->num, sizeof(struct long_ll *));

        for (i = 0; i < gi->file_index->num; ++i)
            gqr->offsets[i] = NULL;
    } else {
        gqr = _gqr;
    }

    giggle_data_handler.map_intersection_to_offset_list(gi, gqr, R);

    return gqr;
}
//}}}

//{{{void giggle_query_result_destroy(struct giggle_query_result **gqr)
void giggle_query_result_destroy(struct giggle_query_result **gqr)
{
    if (*gqr == NULL)
        return;
    uint32_t i;
    for (i = 0; i < (*gqr)->gi->file_index->num; ++i) {
        long_ll_free((void **)&((*gqr)->offsets[i]));
    }
    free((*gqr)->offsets);
    free(*gqr);
    *gqr = NULL;
}
//}}}

//{{{uint32_t giggle_get_query_len(struct giggle_query_result *gqr,
uint32_t giggle_get_query_len(struct giggle_query_result *gqr,
                              uint32_t file_id)
{
    if (gqr->offsets[file_id] == NULL)
        return 0;
    else 
        return gqr->offsets[file_id]->len;
}
//}}}
                      
//{{{struct giggle_query_iter *giggle_get_query_itr(struct giggle_query_result
struct giggle_query_iter *giggle_get_query_itr(struct giggle_query_result *gqr,
                                               uint32_t file_id)
{
#ifdef DEBUG
    fprintf(stderr ,"giggle_get_query_itr file_id:%u\n", file_id);
#endif

    struct giggle_query_iter *gqi = (struct giggle_query_iter *)
        malloc(sizeof(struct giggle_query_iter));

    gqi->gi = gqr->gi;
    gqi->curr = 0;
    gqi->num = 0;
    gqi->file_id = file_id;
    gqi->sorted_offsets = NULL;
    gqi->ipf = NULL;

    if (gqr->offsets[file_id] == NULL)
        return gqi;

#ifdef DEBUG
    fprintf(stderr,
            "giggle_get_query_itr offsets->len:%u\n", 
            gqr->offsets[file_id]->len);
#endif

    gqi->num = gqr->offsets[file_id]->len;

    gqi->sorted_offsets = (long *)
            malloc(gqr->offsets[file_id]->len * sizeof(long));

    struct long_ll_node *curr = gqr->offsets[file_id]->head;
    uint32_t i = 0;
    while (curr != NULL) {
        gqi->sorted_offsets[i++] = curr->val;
        curr = curr->next;
    }

    qsort(gqi->sorted_offsets,
          gqr->offsets[file_id]->len,
          sizeof(long),
          long_cmp);

    return gqi;
}
//}}}

//{{{int giggle_query_next(struct giggle_query_iter *gqi,
int giggle_query_next(struct giggle_query_iter *gqi,
                      char **result)
{
#ifdef DEBUG
    fprintf(stderr,
            "giggle_query_next num:%u\tcurr:%u\n",
            gqi->num,
            gqi->curr);
#endif

    if ((gqi->num == 0) || (gqi->curr == gqi->num)) {
#ifdef DEBUG
        fprintf(stderr, "giggle_query_next -1\n");
#endif
        return -1; 
    }

    if (gqi->ipf == NULL) {
        struct file_data *fd = 
                (struct file_data *)unordered_list_get(gqi->gi->file_index,
                                                       gqi->file_id); 
        gqi->ipf = input_file_init(fd->file_name);
    }

    gqi->ipf->input_file_seek(gqi->ipf, gqi->sorted_offsets[gqi->curr]);
    gqi->ipf->input_file_get_next_line(gqi->ipf, result);

    gqi->curr += 1;

#ifdef DEBUG
    fprintf(stderr, "giggle_query_next 0\n");
#endif
    return 0;
}
//}}}

//{{{void giggle_iter_destroy(struct giggle_query_iter **gqi)
void giggle_iter_destroy(struct giggle_query_iter **gqi)
{
    if ((*gqi)->ipf != NULL)
        input_file_destroy(&((*gqi)->ipf));
    if ((*gqi)->sorted_offsets != NULL)
        free((*gqi)->sorted_offsets);
    free(*gqi);
    *gqi = NULL;
}
//}}}

//{{{ void giggle_write_tree_cache_dump(void *giggle_index)
void giggle_write_tree_cache_dump(void *giggle_index)
{
    struct giggle_index *gi = (struct giggle_index *)giggle_index;
    uint32_t domain;
    for (domain = 0; domain < gi->num; ++domain) {
        bpt_write_tree(domain, gi->root_ids[domain]);
    }
}
//}}}

//{{{ void giggle_write_tree_leaf_data(void *giggle_index)
void giggle_write_tree_leaf_data(void *giggle_index)
{
#if DEBUG
    fprintf(stderr, "giggle_write_tree_leaf_data\n");
#endif

    struct giggle_index *gi = (struct giggle_index *)giggle_index;
    struct simple_cache *sc = (struct simple_cache *)_cache[CACHE_NAME_SPACE];

    // we will use this node to fill in the new values for all the nodes that
    // are in cache
    struct bpt_node *to_write_node = (struct bpt_node *)
            malloc(sizeof(struct bpt_node));
    to_write_node->data = (uint32_t *)calloc(BPT_NODE_NUM_ELEMENTS,
                                             sizeof(uint32_t));

    uint32_t domain;
    for (domain = 0; domain < gi->num; ++domain) {
        if (sc->dss[domain] != NULL)
            errx(1, "Modifying and existing bpt is not currently supported.");

        // estimate the number of elements we want to write out, this is not
        // exactly right, but that is okay 
        uint32_t num_seen =  cache.seen(domain)/2;

        // Each new node or leaf data in this domain will be appended to this
        // disk store
        struct disk_store *ds = disk_store_init(sc->seens[domain],
                                                NULL,
                                                sc->index_file_names[domain],
                                                NULL,
                                                sc->data_file_names[domain]);


        // Use old_id_to_new_id_os to maintain the mapping between the IDs that
        // are in memory and those that will be written to disk.
        struct ordered_set *old_id_to_new_id_os =
            ordered_set_init(num_seen,
                             uint_pair_sort_by_first_element_cmp,
                             uint_pair_search_by_first_element_cmp,
                             uint_pair_search_by_first_key_cmp);

        // Start with the root node for this domain
        struct bpt_node *curr_node = cache.get(domain,
                                               gi->root_ids[domain] - 1,
                                               &bpt_node_cache_handler);

        struct uint_pair *p, *r;

        // put root into a map between the current id and the on-disk id
        // first will but current id 
        // second is the on-disk id
        p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
        p->first = BPT_ID(curr_node);
        p->second = old_id_to_new_id_os->num + 1;
        r = ordered_set_add(old_id_to_new_id_os, p);

        uint32_t new_root_id = p->second;

        struct fifo_q *node_q = NULL, *leaf_q = NULL;
        uint32_t *id = (uint32_t *)malloc(sizeof(uint32_t));
        *id = BPT_ID(curr_node);
        fifo_q_push(&node_q, id);

        while (fifo_q_peek(node_q) != NULL) {
            // pop the next node off the queue and get it from cache
            uint32_t *curr_idp = fifo_q_pop(&node_q);
            uint32_t curr_id = *curr_idp;
            free(curr_idp);
            // cache is zero-based, while bpt is one-based
            curr_node = cache.get(domain,
                                  curr_id - 1,
                                  &bpt_node_cache_handler);
            // Basic steps:
            // - copy the values over to the temp node that we will write out
            // if the node is not a leaf:
            //   - map the pointer values using old_id_to_new_id_os
            //   - put the child nodes into the queue
            //   - set the pointer head to zero
            //   - put the node onto disk
            // if the node is a leaf:
            //   - get the leaf data
            //   - add the leaf data to the and the queue cache so we can write
            //     it out later
            //   - set the pointer head to the disk id of the leaf data
            //   -- each 32bit pointer will be split into 2 16bit offsets
            //      the first will be the start offset and the second the end

            // Zero out the node that we will write to disk
            memset(to_write_node->data, 0, BPT_NODE_SIZE);

            // Get the on-disk id
            uint32_t key = curr_id;
            r = ordered_set_get(old_id_to_new_id_os, &key);
            if (r == NULL)
                errx(1, "Node %u has not been seen yet.", curr_id);

            // Populate the node that we will write to disk
            BPT_ID(to_write_node) =  r->second;
            BPT_PARENT(to_write_node) = BPT_PARENT(curr_node);
            BPT_IS_LEAF(to_write_node) = BPT_IS_LEAF(curr_node);
            BPT_LEADING(to_write_node) = BPT_LEADING(curr_node);
            BPT_NEXT(to_write_node) = BPT_NEXT(curr_node);
            BPT_NUM_KEYS(to_write_node) = BPT_NUM_KEYS(curr_node);
            BPT_POINTERS_BLOCK(to_write_node) = 0;
            uint32_t i;
            for (i = 0; i <= BPT_NUM_KEYS(curr_node); ++i)
                BPT_KEYS(to_write_node)[i] = BPT_KEYS(curr_node)[i];


            if (BPT_IS_LEAF(curr_node) == 0) {

               for (i = 0; i <= BPT_NUM_KEYS(curr_node); ++i) {
                    if (BPT_POINTERS(curr_node)[i] != 0) {
                        // put a map between the current id and the to disk id
                        p = (struct uint_pair *)
                                malloc(sizeof(struct uint_pair));
                        p->first = BPT_POINTERS(curr_node)[i];
                        p->second = old_id_to_new_id_os->num + 1;
                        r = ordered_set_add(old_id_to_new_id_os, p);

                        if (r->second != p->second)
                            errx(1,
                                 "%u has already been seen at %u\n",
                                 p->first,
                                 r->first);

                        // update the node we are writing to disk with the new 
                        // id
                        BPT_POINTERS(to_write_node)[i] =  p->second;

                        // put the child on the queue
                        id = (uint32_t *)malloc(sizeof(uint32_t));
                        *id = BPT_POINTERS(curr_node)[i];
                        fifo_q_push(&node_q, id);
                    }
                }

                uint8_t *ds_node;
                uint64_t d_size = bpt_node_serialize(to_write_node,
                                                     (void **)&ds_node);

                // Write the mapped node to disk
                uint32_t ret = disk_store_append(ds,
                                                 ds_node,
                                                 d_size);

                // Make sure it gets the ID that we expect
                if (ret + 1 != BPT_ID(to_write_node))
                    errx(1,
                         "Disk write is out of sync.  Saw %u.  Expected %u.",
                         ret + 1, 
                         BPT_ID(to_write_node));

                free(ds_node);
            } else {

                // replace the next id with the on disk id
                if (BPT_NEXT(curr_node) != 0) {
                    key = BPT_NEXT(curr_node);
                    r = ordered_set_get(old_id_to_new_id_os, &key);
                    if (r == NULL)
                        errx(1, "Node %u has not been seen yet.", key);
                    BPT_NEXT(to_write_node) = r->second;
                }

                // get the leaf data, then add it to the cache so we can
                // grab it later
                struct leaf_data *lf = NULL;
                uint16_t *starts_ends_offsets = NULL;
                uint32_t leaf_data_size = 
                        giggle_get_leaf_data(gi,
                                             domain,
                                             curr_id,
                                             &lf,
                                             &starts_ends_offsets);
                if (leaf_data_size == 0)
                    errx(1, "Could not get leaf data.");

                //uint8_t *output = (uint8_t *)malloc(
                                   //2*leaf_data_size * sizeof(uint32_t));

                //int cs = fastlz_compress(lf->data,
                                    //leaf_data_size * sizeof(uint32_t),
                                    //output);

                uint32_t data_id = cache.seen(domain) + 1;
                cache.add(domain,
                          data_id - 1,
                          lf,
                          &leaf_data_cache_handler);

                p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
                p->first = data_id;
                p->second = old_id_to_new_id_os->num + 1;
                r = ordered_set_add(old_id_to_new_id_os, p);

                id = (uint32_t *)malloc(sizeof(uint32_t));
                *id = data_id;
                fifo_q_push(&leaf_q, id);

                BPT_POINTERS_BLOCK(to_write_node) = p->second;

                uint32_t i, a, b;
                for (i = 0; i < BPT_NUM_KEYS(to_write_node); ++i) {
                    a = (uint32_t)starts_ends_offsets[i*2];
                    b = (uint32_t)starts_ends_offsets[i*2+1];
                    BPT_POINTERS(to_write_node)[i] = (a << 16) + b;
                }
                free(starts_ends_offsets);


                uint8_t *ds_node;
                uint64_t d_size = bpt_node_serialize(to_write_node,
                                                     (void **)&ds_node);

                // Write the mapped node to disk
                uint32_t ret = disk_store_append(ds,
                                                 ds_node,
                                                 d_size);

                // Make sure it gets the ID that we expect
                if (ret + 1 != BPT_ID(to_write_node))
                    errx(1,
                         "Disk write is out of sync.  Saw %u.  Expected %u.",
                         ret + 1, 
                         BPT_ID(to_write_node));

                free(ds_node);
            }
        }
        while (fifo_q_peek(leaf_q) != NULL) {
            // pop the next node off the queue and get it from cache
            uint32_t *curr_idp = fifo_q_pop(&leaf_q);
            uint32_t curr_id = *curr_idp;
            free(curr_idp);
            // cache is zero-based, while bpt is one-based
            struct leaf_data *lf = cache.get(domain,
                                             curr_id - 1,
                                             &leaf_data_cache_handler);

            uint8_t *ds_data;
            uint64_t d_size = leaf_data_serialize(lf,
                                                 (void **)&ds_data);

            // Write the mapped node to disk
            uint32_t ret = disk_store_append(ds,
                                             ds_data,
                                             d_size);
            free(ds_data);
        }

        sc->dss[domain] = ds;
        ordered_set_destroy(&old_id_to_new_id_os, free_wrapper);
        gi->root_ids[domain] = new_root_id;
    }

    bpt_node_free_mem((void **)&to_write_node);
}
//}}}

//{{{ leaf_data_cache_handler

    /*
     * struct leaf_data {
     *   uint32_t num_leading, num_starts, num_ends;
     *   uint32_t *leading, *starts, *ends, *data;
     * };
     */
struct cache_handler leaf_data_cache_handler = {leaf_data_serialize, 
                                                leaf_data_deserialize,
                                                leaf_data_free_mem};


//}}}

//{{{ uint32_t giggle_get_leaf_data(struct giggle_index *gi,
uint32_t giggle_get_leaf_data(struct giggle_index *gi,
                              uint32_t domain,
                              uint32_t leaf_id,
                              struct leaf_data **lf,
                              uint16_t **starts_ends_offsets)
{
    // cache is zero-based, while bpt is one-based
    struct bpt_node *curr_node = cache.get(domain,
                                           leaf_id - 1,
                                           &bpt_node_cache_handler);

    // If the node is a leaf we need to deal with the leading values
    if (BPT_IS_LEAF(curr_node)) {
        *lf = (struct leaf_data *) calloc(1, sizeof(struct leaf_data));


        // Do one scan to find the sizes
        if (BPT_LEADING(curr_node) != 0) {
            struct uint32_t_ll_bpt_leading_data *ld = 
                    cache.get(domain,
                              BPT_LEADING(curr_node) - 1,
                              &uint32_t_ll_leading_cache_handler);
            (*lf)->num_leading = ld->B->len;
        }

        uint32_t j, k;

        for (j = 0; j <= BPT_NUM_KEYS(curr_node) - 1; ++j) {
            struct uint32_t_ll_bpt_non_leading_data *nld = 
                    cache.get(domain,
                              BPT_POINTERS(curr_node)[j] - 1,
                              &uint32_t_ll_non_leading_cache_handler);

            (*lf)->num_starts += (nld->SA == NULL ? 0 : nld->SA->len);
            (*lf)->num_ends += (nld->SE == NULL ? 0 : nld->SE->len);
        }

        // Allocate the memory and put in helper pointers
        (*lf)->data = (uint32_t *)calloc(
                (*lf)->num_leading + (*lf)->num_starts + (*lf)->num_ends,
                sizeof(uint32_t));

        if ((*lf)->num_leading > 0)
            (*lf)->leading = (*lf)->data;
        else
            (*lf)->leading = NULL;

        if ((*lf)->num_starts > 0)
            (*lf)->starts = (*lf)->data + (*lf)->num_leading;
        else
            (*lf)->starts = NULL;

        if ((*lf)->num_ends > 0)
            (*lf)->ends = (*lf)->data + (*lf)->num_leading + (*lf)->num_starts;
        else
            (*lf)->ends = NULL;

        //track the end of each array for each pointer
        *starts_ends_offsets = 
            (uint16_t *)calloc(BPT_NUM_KEYS(curr_node)*2, sizeof(uint16_t));

        // Do a second scan to get the data into the array
        uint32_t leading_i = 0, starts_i = 0, ends_i = 0;

        if (BPT_LEADING(curr_node) != 0) {
            struct uint32_t_ll_bpt_leading_data *ld = 
                    cache.get(domain,
                              BPT_LEADING(curr_node) - 1,
                              &uint32_t_ll_leading_cache_handler);
            
            k = leading_i;
            if (ld->B->len > 0) {
                struct uint32_t_ll_node *curr = ld->B->head;
                while (curr != NULL) {
                    (*lf)->leading[k] = curr->val;
                    k += 1;
                    curr = curr->next;
                }
                qsort((*lf)->leading + leading_i,
                      ld->B->len,
                      sizeof(uint32_t),
                      uint32_t_cmp);
            }
            leading_i = k;
        }

        for (j = 0; j <= BPT_NUM_KEYS(curr_node) - 1; ++j) {
            struct uint32_t_ll_bpt_non_leading_data *nld = 
                    cache.get(domain,
                              BPT_POINTERS(curr_node)[j] - 1,
                              &uint32_t_ll_non_leading_cache_handler);

            k = starts_i;
            if ((nld->SA != NULL) && (nld->SA->len > 0)) {
                struct uint32_t_ll_node *curr = nld->SA->head;
                while (curr != NULL) {
                    (*lf)->starts[k] = curr->val;
                    k += 1;
                    curr = curr->next;
                }
                qsort((*lf)->starts + starts_i,
                      nld->SA->len,
                      sizeof(uint32_t),
                      uint32_t_cmp);

                (*starts_ends_offsets)[j*2] = 
                    (( j == 0) ? 0 : (*starts_ends_offsets)[j*2-2]) +
                    nld->SA->len;
            } else {
                (*starts_ends_offsets)[j*2] = 
                    ( j == 0) ? 0 : (*starts_ends_offsets)[j*2-2];
            }

            starts_i = k;

            k = ends_i;
            if ((nld->SE != NULL) && (nld->SE->len > 0)) {
                struct uint32_t_ll_node *curr = nld->SE->head;
                while (curr != NULL) {
                    (*lf)->ends[k] = curr->val;
                    k += 1;
                    curr = curr->next;
                }
                qsort((*lf)->ends + ends_i,
                      nld->SE->len,
                      sizeof(uint32_t),
                      uint32_t_cmp);

                (*starts_ends_offsets)[j*2+1] = 
                    (( j == 0) ? 0 : (*starts_ends_offsets)[j*2+1-2]) +
                    nld->SE->len;
            }else {
                (*starts_ends_offsets)[j*2+1] = 
                    ( j == 0) ? 0 : (*starts_ends_offsets)[j*2+1-2];
            }

            ends_i = k;
        }
        return (*lf)->num_leading + (*lf)->num_starts + (*lf)->num_ends;
    } else {
        return 0;
    }
}
//}}}

//{{{void leaf_data_map_intersection_to_offset_list(struct giggle_index *gi,
void leaf_data_map_intersection_to_offset_list(struct giggle_index *gi,
                                            struct giggle_query_result *gqr,
                                            void *_R)
{
#ifdef DEBUG
    fprintf(stderr,
            "leaf_data_map_intersection_to_offset_list\n");
#endif
    struct leaf_data_result *R = (struct leaf_data_result *)_R;
    /*
    struct leaf_data_result {
        uint32_t len;
        uint32_t *data;
        struct leaf_data_result *next;
    };
    */

    if (R != NULL) {
        gqr->num_hits += R->len;

#ifdef DEBUG
        fprintf(stderr, "R->len:%u\n", R->len);
#endif

        uint32_t i;
        for (i = 0; i < R->len; ++i) {
            struct file_id_offset_pair fid_off = 
                    gi->offset_index->vals[R->data[i]];
            long_ll_append(&(gqr->offsets[fid_off.file_id]),fid_off.offset);
        }

        struct leaf_data_result *tmp_R = R->next;
        free(R->data);
        free(R);
        R = tmp_R;
    } 
}
//}}}

//{{{ uint32_t giggle_merge_chrom(char *chrm_string,
uint32_t giggle_merge_chrom(char *chrm_string,
                            struct giggle_index *gi_0,
                            struct indexed_list *file_index_id_map_0,
                            uint32_t gi_0_cache_name_space,
                            struct giggle_index *gi_1,
                            struct indexed_list *file_index_id_map_1,
                            uint32_t gi_1_cache_name_space,
                            struct disk_store *ds,
                            struct file_id_offset_pairs **merged_offset_index)
{
    // Initialize values for tree 0
    CACHE_NAME_SPACE = gi_0_cache_name_space;
    uint32_t chr_id_0 = giggle_get_chrm_id(gi_0, chrm_string);
    uint32_t curr_leaf_id_0;
    int pos_start_id_0;

    // find the left-most leaf node for tree 0
    uint32_t nld_start_id_0 = bpt_find(chr_id_0,
                                       gi_0->root_ids[chr_id_0],
                                       &curr_leaf_id_0, 
                                       &pos_start_id_0,
                                       0);
    struct bpt_node *curr_leaf_0 = cache.get(chr_id_0,
                                             curr_leaf_id_0 - 1,
                                             &bpt_node_cache_handler);
    struct leaf_data *curr_leaf_data_0 = 
            cache.get(chr_id_0,
                      BPT_POINTERS_BLOCK(curr_leaf_0) - 1,
                      &leaf_data_cache_handler);

    // Initialize values for tree 1
    CACHE_NAME_SPACE = gi_1_cache_name_space;
    uint32_t chr_id_1 = giggle_get_chrm_id(gi_1, chrm_string);
    uint32_t curr_leaf_id_1;
    int pos_start_id_1;

    // find the left-most leaf node for tree 1
    uint32_t nld_start_id_1 = bpt_find(chr_id_1,
                                       gi_1->root_ids[chr_id_1],
                                       &curr_leaf_id_1, 
                                       &pos_start_id_1,
                                       0);
    struct bpt_node *curr_leaf_1 = cache.get(chr_id_1,
                                              curr_leaf_id_1 - 1,
                                              &bpt_node_cache_handler);
    struct leaf_data *curr_leaf_data_1 = 
            cache.get(chr_id_1,
                      BPT_POINTERS_BLOCK(curr_leaf_1) - 1,
                      &leaf_data_cache_handler);

    uint32_t i_0 = 0, i_1 = 0;

    // These trees will track the intervals that are currently in context
    jsw_avltree_t *context_tree_0 = jsw_avlnew(int_cmp_f, int_dup_f, int_rel_f);
    jsw_avltree_t *context_tree_1 = jsw_avlnew(int_cmp_f, int_dup_f, int_rel_f);

    /*
     * As the keys/pointers are scanned in each leaf node a new leaf node is
     * built with the merged data. At the same time a new leaf data is built
     * and any spanning nodes must be tracked for the leading data
     *
     * Must keep a list of in context interval ids
     *
     * Interval ids from each tree must be mapped to new ids for the new tree
     *
     * At each start we need to add all of those nodes to context
     *
     * At each end we must remove those from context
     *
     * Anything that is in context when a leaf node is full must be placed into
     * the leading node of the next leaf node
     *
     *
     *
     */

    // offset ids must be mapped from the values in the distisct trees
    // to merged values, offset_id_map_0 tracks the values from tree 0 and 
    // offset_id_map_1 from tree 1
    // the key will be the original id and value will be the merged id
    struct indexed_list *offset_id_map_0 = indexed_list_init(1000,
                                                             sizeof(uint64_t));
    struct indexed_list *offset_id_map_1 = indexed_list_init(1000,
                                                             sizeof(uint64_t));

    // These lists will become the leaf data for the merged node
    uint32_t merged_starts_size = 1000, merged_starts_num = 0;
    uint32_t merged_ends_size = 1000, merged_ends_num = 0;

    uint32_t *merged_starts =
            (uint32_t *)malloc(merged_starts_size * sizeof(uint32_t));
    uint32_t *merged_ends = 
            (uint32_t *)malloc(merged_ends_size * sizeof(uint32_t));

    // Collect the values into this node, then write it and clear 
    struct bpt_node *to_write_node = (struct bpt_node *)
            malloc(sizeof(struct bpt_node));
    to_write_node->data = (uint32_t *)
            malloc(BPT_NODE_NUM_ELEMENTS  * sizeof(uint32_t));

    memset(to_write_node->data, 0, BPT_NODE_SIZE);

    BPT_ID(to_write_node) =  ds->num;
    BPT_PARENT(to_write_node) = 0;
    BPT_IS_LEAF(to_write_node) = 1;
    BPT_LEADING(to_write_node) = 0;
    BPT_NEXT(to_write_node) = 0;
    BPT_NUM_KEYS(to_write_node) = 0;
    BPT_POINTERS_BLOCK(to_write_node) = 0;


    uint32_t node_key_i = 0;

    // loop over all the elments in the chrom tree and merge into a single tree
    while (true) {

        if ( (curr_leaf_id_1 == 0 ) && (curr_leaf_id_0 == 0) ) 
            break;

        uint32_t bpt_key_value = 0, bpt_pointer_value = 0;

        // Choose the next lowest value to merge in hhhhhhhh 
        if ((curr_leaf_id_1 == 0 ) ||
            (BPT_KEYS(curr_leaf_0)[i_0]) < (BPT_KEYS(curr_leaf_1)[i_1])) {
            //{{{pick the value from 0 if it is the only one left, or it is
            //less

            bpt_key_value = BPT_KEYS(curr_leaf_0)[i_0];
            
            uint32_t orig_merged_starts_num = merged_starts_num;
            uint32_t orig_merged_ends_num = merged_ends_num;

            //fprintf(stderr,"0: ");
            giggle_merge_leaf_key(curr_leaf_0,
                                  curr_leaf_data_0,
                                  i_0,
                                  context_tree_0,
                                  offset_id_map_0,
                                  file_index_id_map_0,
                                  gi_0->offset_index,
                                  merged_offset_index,
                                  &merged_starts, 
                                  &merged_starts_size, 
                                  &merged_starts_num, 
                                  &merged_ends,
                                  &merged_ends_size,
                                  &merged_ends_num);

            bpt_pointer_value = (merged_starts_num << 16) + merged_ends_num;

            /*
            fprintf(stderr,
                    "merged s:%u-%u,%u e:%u-%u,%u\n",
                    orig_merged_starts_num,
                    merged_starts_num,
                    (orig_merged_starts_num << 16) + merged_starts_num,
                    orig_merged_ends_num,
                    merged_ends_num,
                    (orig_merged_ends_num << 16) + merged_ends_num);
            */
            i_0+=1;
            //}}}
        } else if ((curr_leaf_id_0 == 0 ) ||
                   (BPT_KEYS(curr_leaf_0)[i_0] > BPT_KEYS(curr_leaf_1)[i_1])) {
            //{{{ pick the value from 1 if it is the only one left, or it is
            //less
            bpt_key_value = BPT_KEYS(curr_leaf_1)[i_1];

            uint32_t orig_merged_starts_num = merged_starts_num;
            uint32_t orig_merged_ends_num = merged_ends_num;

            //fprintf(stderr,"1: ");
            giggle_merge_leaf_key(curr_leaf_1,
                                  curr_leaf_data_1,
                                  i_1,
                                  context_tree_1,
                                  offset_id_map_1,
                                  file_index_id_map_1,
                                  gi_1->offset_index,
                                  merged_offset_index,
                                  &merged_starts, 
                                  &merged_starts_size, 
                                  &merged_starts_num, 
                                  &merged_ends,
                                  &merged_ends_size,
                                  &merged_ends_num);

            bpt_pointer_value = (merged_starts_num << 16) + merged_ends_num;

            /*
            fprintf(stderr,
                    "merged s:%u-%u,%u e:%u-%u,%u\n",
                    orig_merged_starts_num,
                    merged_starts_num,
                    (orig_merged_starts_num << 16) + merged_starts_num,
                    orig_merged_ends_num,
                    merged_ends_num,
                    (orig_merged_ends_num << 16) + merged_ends_num);
            */

            i_1+=1;
            //}}}
        } else if ((BPT_KEYS(curr_leaf_0)[i_0]) == 
                   (BPT_KEYS(curr_leaf_1)[i_1])) {
            // {{{ they are equal

            uint32_t orig_merged_starts_num = merged_starts_num;
            uint32_t orig_merged_ends_num = merged_ends_num;

            bpt_key_value = BPT_KEYS(curr_leaf_0)[i_0];

            //fprintf(stderr,"0 1: ");
            giggle_merge_leaf_key(curr_leaf_0,
                                  curr_leaf_data_0,
                                  i_0,
                                  context_tree_0,
                                  offset_id_map_0,
                                  file_index_id_map_0,
                                  gi_0->offset_index,
                                  merged_offset_index,
                                  &merged_starts, 
                                  &merged_starts_size, 
                                  &merged_starts_num, 
                                  &merged_ends,
                                  &merged_ends_size,
                                  &merged_ends_num);


            giggle_merge_leaf_key(curr_leaf_1,
                                  curr_leaf_data_1,
                                  i_1,
                                  context_tree_1,
                                  offset_id_map_1,
                                  file_index_id_map_1,
                                  gi_1->offset_index,
                                  merged_offset_index,
                                  &merged_starts, 
                                  &merged_starts_size, 
                                  &merged_starts_num, 
                                  &merged_ends,
                                  &merged_ends_size,
                                  &merged_ends_num);

            bpt_pointer_value = (merged_starts_num << 16) + merged_ends_num;

            /*
            fprintf(stderr,
                    "merged s:%u-%u,%u e:%u-%u,%u\n",
                    orig_merged_starts_num,
                    merged_starts_num,
                    (orig_merged_starts_num << 16) + merged_starts_num,
                    orig_merged_ends_num,
                    merged_ends_num,
                    (orig_merged_ends_num << 16) + merged_ends_num);
            */

            i_0+=1;
            i_1+=1;
            //}}}
        } else {
            errx(1, "Not > < or ==");
        }

        //{{{ see if we are moving to the next leaf node on tree 0
        if ( (curr_leaf_id_0 > 0) &&
             (i_0 == BPT_NUM_KEYS(curr_leaf_0)) ) {

            curr_leaf_id_0 = BPT_NEXT(curr_leaf_0);

            if (curr_leaf_id_0 != 0) {
                CACHE_NAME_SPACE = gi_0_cache_name_space;
                curr_leaf_0 = cache.get(chr_id_0,
                                        curr_leaf_id_0 - 1,
                                        &bpt_node_cache_handler);
                i_0 = 0;

                curr_leaf_data_0 = 
                    cache.get(chr_id_0,
                              BPT_POINTERS_BLOCK(curr_leaf_0) - 1,
                              &leaf_data_cache_handler);
                /*
                fprintf(stderr,
                        "node_0: #:%u\n",
                        BPT_NUM_KEYS(curr_leaf_0));
                fprintf(stderr,
                        "leaf_0: l:%u s:%u e:%u\n",
                        curr_leaf_data_0->num_leading,
                        curr_leaf_data_0->num_starts,
                        curr_leaf_data_0->num_ends);
                */

            }
        }
        //}}}

        //{{{ see if we are moving to the next leaf node on tree 1
        if ( (curr_leaf_id_1 > 0) &&
             (i_1 == BPT_NUM_KEYS(curr_leaf_1)) ) {

            curr_leaf_id_1 = BPT_NEXT(curr_leaf_1);

            if (curr_leaf_id_1 != 0) {
                CACHE_NAME_SPACE = gi_1_cache_name_space;
                curr_leaf_1 = cache.get(chr_id_1,
                                        curr_leaf_id_1 - 1,
                                        &bpt_node_cache_handler);
                i_1 = 0;

                curr_leaf_data_1 = 
                    cache.get(chr_id_1,
                              BPT_POINTERS_BLOCK(curr_leaf_1) - 1,
                              &leaf_data_cache_handler);
            }
        }
        //}}}

        /*
        fprintf(stderr,
                "i:%u key:%u\tpointer:%u\t%u\t%u\n",
                node_key_i,
                bpt_key_value,
                bpt_pointer_value,
                bpt_pointer_value >> 16,
                bpt_pointer_value & 65535);
        */

        //BPT_ID(to_write_node) =  ds->num;
        //BPT_PARENT(to_write_node) = 0;
        //BPT_IS_LEAF(to_write_node) = 1;
        //BPT_LEADING(to_write_node) = 0;
        //BPT_NEXT(to_write_node) = 0;
        //BPT_NUM_KEYS(to_write_node) = 0;
        //BPT_POINTERS_BLOCK(to_write_node) = ds->num + 1;
        
        BPT_KEYS(to_write_node)[node_key_i] = bpt_pointer_value;
        BPT_POINTERS(to_write_node)[node_key_i] = bpt_pointer_value;

        node_key_i += 1;

        // The current node is full
        if (node_key_i == ORDER - 1) {

            uint32_t j;
            for (j = 0; j < node_key_i; ++j) {
                fprintf(stderr,
                        "%u\t%u %u %u %u\n",
                        j,
                        BPT_ID(to_write_node),
                        BPT_KEYS(to_write_node)[j],
                        (BPT_POINTERS(to_write_node)[j]) >> 16,
                        (BPT_POINTERS(to_write_node)[j])  & 65535);
            }

            // Add a leading node
            if ( ( jsw_avlsize(context_tree_0) > 0 ) || 
                 ( jsw_avlsize(context_tree_0) > 0 ) ) {
                BPT_LEADING(to_write_node) = 1;
                BPT_POINTERS_BLOCK(to_write_node) = ds->num + 1;
            }

            // Everything is stored through the disk_store struct ds
            // write the current node and the leaf node 
            // set up the next node

            fprintf(stderr, "tree_0 size:%zu\n", jsw_avlsize(context_tree_0));
            fprintf(stderr, "tree_1 size:%zu\n", jsw_avlsize(context_tree_1));

            //jsw_avltrav_t *trav = jsw_avltnew ( void );
            //void *jsw_avltfirst ( jsw_avltrav_t *trav, jsw_avltree_t *tree );
            //void *jsw_avltnext ( jsw_avltrav_t *trav )
            //void jsw_avltdelete ( jsw_avltrav_t *trav );

            // if void           jsw_avltdelete ( jsw_avltrav_t *trav );
        //if ( (curr_leaf_id_1 == 0 ) && (curr_leaf_id_0 == 0) ) 
        //Not going to be a next node

            node_key_i = 0;
        }
    }

    if (node_key_i > 0) {
        uint32_t j;
        for (j = 0; j < node_key_i; ++j) {
            fprintf(stderr,
                    "%u\t%u %u %u\n",
                    j,
                    BPT_KEYS(to_write_node)[j],
                    (BPT_POINTERS(to_write_node)[j]) >> 16,
                    (BPT_POINTERS(to_write_node)[j])  & 65535);
        }
    }



    jsw_avldelete(context_tree_0);
    jsw_avldelete(context_tree_1);

    indexed_list_destroy(&offset_id_map_0);
    indexed_list_destroy(&offset_id_map_1);
    return 0;
}
//}}}

//{{{ uint32_t giggle_merge_add_file_index(struct giggle_index *gi,
uint32_t giggle_merge_add_file_index(struct giggle_index *gi,
                                     struct indexed_list *file_index_id_map,
                                     struct unordered_list *merged_file_index)
{
    uint32_t i;
    for (i = 0 ; i < gi->file_index->num; ++i) {
        struct file_data *fd = (struct file_data *)
                unordered_list_get(gi->file_index, i);

        struct file_data *merged_fd = (struct file_data *)
                malloc(sizeof(struct file_data));
        merged_fd->num_intervals = fd->num_intervals;
        merged_fd->mean_interval_size = fd->mean_interval_size;
        merged_fd->file_name = strdup(fd->file_name);

        uint32_t merged_id = merged_file_index->num;

        uint32_t r = unordered_list_add(merged_file_index, merged_fd);

        r = indexed_list_add(file_index_id_map, i, &merged_id);
    }

    return gi->file_index->num;
}
//}}}

//{{{ uint32_t giggle_merge_get_chrm_index(struct giggle_index *gi_0,
uint32_t giggle_merge_chrm_union(struct giggle_index *gi_0,
                                 struct giggle_index *gi_1,
                                 char ***merged_chrm_set)
{
    // Find the union of the two chrom sets
    char **full_chrm_set = 
            (char **)malloc(
            (gi_0->chrm_index->num + gi_0->chrm_index->num) * sizeof (char *));

    uint32_t i;
    for (i = 0; i < gi_0->chrm_index->num; ++i) {
        struct str_uint_pair *p = 
                (struct str_uint_pair *)gi_0->chrm_index->data[i];
        full_chrm_set[i] = strdup(p->str);
    }
    for (i = 0; i < gi_1->chrm_index->num; ++i) {
        struct str_uint_pair *p = 
                (struct str_uint_pair *)gi_1->chrm_index->data[i];
        full_chrm_set[i + gi_0->chrm_index->num] = strdup(p->str);
    }

    qsort(full_chrm_set,
          gi_0->chrm_index->num + gi_1->chrm_index->num,
          sizeof(char *),
          char_p_cmp);

    uint32_t j, num_uniq = 0;

    for (i = 0; i < gi_0->chrm_index->num + gi_1->chrm_index->num; ) {
        num_uniq += 1;
        j = i + 1;
        while ((j < gi_0->chrm_index->num + gi_1->chrm_index->num) &&
               (strcmp(full_chrm_set[i], full_chrm_set[j]) == 0)) {
            j += 1;
        }
        i = j;
    }

    *merged_chrm_set = (char **)malloc(num_uniq * sizeof (char *));
    uint32_t merged_chrm_set_i = 0;
    for (i = 0; i < gi_0->chrm_index->num + gi_1->chrm_index->num; ) {
        (*merged_chrm_set)[merged_chrm_set_i] = strdup(full_chrm_set[i]);
        merged_chrm_set_i += 1;
        j = i + 1;
        while ((j < gi_0->chrm_index->num + gi_1->chrm_index->num) &&
               (strcmp(full_chrm_set[i], full_chrm_set[j]) == 0)) {
            j += 1;
        }
        i = j;
    }

    for (i = 0; i < gi_0->chrm_index->num + gi_1->chrm_index->num; ++i)
        free(full_chrm_set[i]);
    free(full_chrm_set);

    return num_uniq;
}
//}}}

//{{{void giggle_merge_leaf_key(struct bpt_node *node,
void giggle_merge_leaf_key(struct bpt_node *node,
                           struct leaf_data *data,
                           uint32_t key_i,
                           jsw_avltree_t *context_tree,
                           struct indexed_list *offset_id_map,
                           struct indexed_list *file_index_id_map,
                           struct file_id_offset_pairs *offset_index,
                           struct file_id_offset_pairs **merged_offset_index,
                           uint32_t **merged_starts, 
                           uint32_t *merged_starts_size, 
                           uint32_t *merged_starts_num, 
                           uint32_t **merged_ends,
                           uint32_t *merged_ends_size,
                           uint32_t *merged_ends_num)
{
    // put all of the offset ids in starts into the tree
    // get merged offset ids for each start
    // add the merged ids to the start for this position
    // take all of the offset ids out of the tree
    fprintf(stderr,
            "%u\t%u\t%u\ts:%u-%u\te:%u-%u"
            "\t",
            key_i,
            BPT_POINTERS(node)[key_i],
            BPT_KEYS(node)[key_i],
            LEAF_DATA_STARTS_START(node,key_i),
            LEAF_DATA_STARTS_END(node,key_i),
            LEAF_DATA_ENDS_START(node,key_i),
            LEAF_DATA_ENDS_END(node,key_i)
            );

    uint32_t *starts = NULL, *ends = NULL;
    uint32_t starts_size = 0, ends_size = 0;
    // get the list of starts and ends at this key
    uint32_t buff_size = leaf_data_get_starts_ends(node,
                                                   data,
                                                   key_i,
                                                   key_i,
                                                   &starts,
                                                   &starts_size,
                                                   &ends,
                                                   &ends_size);

    //grow the merged offset if we need to
    while ((*merged_offset_index)->size < 
           (*merged_offset_index)->num + starts_size) {

        (*merged_offset_index)->size = (*merged_offset_index)->size * 2;

        fprintf(stderr,
                "merged_offset_index size: %llu\n",
                (*merged_offset_index)->size);


        (*merged_offset_index)->vals = (struct file_id_offset_pair *)
            realloc((*merged_offset_index)->vals,
                    (*merged_offset_index)->size * 
                    sizeof(struct file_id_offset_pair));

        memset((*merged_offset_index)->vals + (*merged_offset_index)->num,
               0,
               ((*merged_offset_index)->size - (*merged_offset_index)->num) *
               sizeof(struct file_id_offset_pair));
    }

    //grow the merged starts if we need to
    while (*merged_starts_size < *merged_starts_num + starts_size) {
        *merged_starts_size =  *merged_starts_size * 2;

        fprintf(stderr, "merged_starts: %u\n", *merged_starts_size);

        *merged_starts = (uint32_t *)
            realloc(*merged_starts, *merged_starts_size * sizeof(uint32_t)); 
    }
  
    //grow the merged ends if we need to
    while (*merged_ends_size < *merged_ends_num + ends_size) {
        *merged_ends_size =  *merged_ends_size * 2;

        fprintf(stderr, "merged_ends: %u\n", *merged_ends_size);

        *merged_ends = (uint32_t *)
            realloc(*merged_ends, *merged_ends_size * sizeof(uint32_t)); 
    }

    // loop over the starts
    // get the merged id
    // add to context tree
    // - add to new offset_index
    // - add to new starts list at this position
    uint32_t j;
    fprintf(stderr, "starts:\t");
    for (j = 0; j < starts_size; ++j) {
        int r = indexed_list_add(offset_id_map,
                                 starts[j],
                                 &((*merged_offset_index)->num));

        fprintf(stderr,
                "oid:%u,mid:%llu ",
                starts[j],
                (*merged_offset_index)->num);

        r = jsw_avlinsert(context_tree, starts + j);


        uint32_t *merged_file_id = 
                indexed_list_get(file_index_id_map, 
                                 offset_index->vals[starts[j]].file_id);
        //(*merged_offset_index)->vals[(*merged_offset_index)->num].file_id =
            //offset_index->vals[starts[j]].file_id;
            
        (*merged_offset_index)->vals[(*merged_offset_index)->num].file_id =
            *merged_file_id;

        (*merged_offset_index)->vals[(*merged_offset_index)->num].offset =
            offset_index->vals[starts[j]].offset;

        (*merged_starts)[*merged_starts_num] = (*merged_offset_index)->num;
        *merged_starts_num = *merged_starts_num + 1;

        (*merged_offset_index)->num = (*merged_offset_index)->num + 1;
    }
    fprintf(stderr, "\t");

    // loop over ends
    // remove from context tree
    // get merged id
    // - add to new ends list at this position
    fprintf(stderr, "ends:\t");
    for (j = 0; j < ends_size; ++j) {
        uint32_t *merged_id = (uint32_t *)
                indexed_list_get(offset_id_map, ends[j]);
        fprintf(stderr,
                "oid:%u,mid:%u ",
                ends[j], 
                *merged_id);

        (*merged_ends)[*merged_ends_num] = *merged_id;
        *merged_ends_num = *merged_ends_num + 1;

        int r = jsw_avlerase(context_tree, ends + j);
    }
    fprintf(stderr, "\n");

    if (starts != NULL)
        free(starts);
    if (ends != NULL)
        free(ends);
}
//}}}
