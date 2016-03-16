#define _GNU_SOURCE

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <glob.h>
#include <sysexits.h>

#include "bpt.h"
#include "cache.h"
#include "giggle.h"
#include "ll.h"
#include "lists.h"
#include "file_read.h"
#include "util.h"
#include "timer.h"

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
    end_leaf_id = bpt_find_leaf(domain, *root_id, end);

#if DEBUG
    fprintf(stderr, "s_id:%u e_id:%u\n", start_leaf_id, end_leaf_id);

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
#if DEBUG
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


#if DEBUG
    fprintf(stderr,
            "leaf_start_id:%u\tpos_start_id:%d\n",
            leaf_start_id,
            pos_start_id);
#endif

    uint32_t leaf_end_id;
    int pos_end_id;

    void *r = NULL;

    uint32_t nld_end_id = bpt_find(domain,
                                   root_id,
                                   &leaf_end_id, 
                                   &pos_end_id,
                                   end);

    struct bpt_node *leaf_end = cache.get(domain,
                                          leaf_end_id - 1,
                                          &bpt_node_cache_handler);

#if DEBUG
    fprintf(stderr,
            "leaf_end_id:%u\tpos_end_id:%u\t\n",
            leaf_end_id,
            pos_end_id);
#endif

    if ((pos_end_id == 0) && (BPT_KEYS(leaf_end)[0] != end))
        pos_end_id = -1;
    else if ( (pos_end_id >=0) && 
              (pos_end_id < BPT_NUM_KEYS(leaf_end)) &&
              (BPT_KEYS(leaf_end)[pos_end_id] > end))
        pos_end_id -= 1;

#if DEBUG
    fprintf(stderr,
            "leaf_end_id:%u\tpos_end_id:%u\t\n",
            leaf_end_id,
            pos_end_id);
#endif

#if DEBUG
    fprintf(stderr, "pos_end_id:%d %u\n", pos_end_id,
            ( ((pos_end_id >=0)&&(pos_end_id<BPT_NUM_KEYS(leaf_end))) ?
              BPT_KEYS(leaf_end)[pos_end_id] : 0)
            );
#endif

    if ((leaf_start_id == leaf_end_id) && (pos_start_id > pos_end_id))
        return r;

#if DEBUG
    if (BPT_LEADING(leaf_start) == 0)
        fprintf(stderr, "BPT_LEADING(leaf_start) == 0\n");

#endif

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
            malloc(gi->offset_index->size *
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
    ORDER = 100;

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

    return gi;
}
//}}}

//{{{ uint32_t giggle_store(struct giggle_index *gi)
uint32_t giggle_store(struct giggle_index *gi)
{
    if (gi->chrm_index_file_name == NULL)
        return 1;

    uint32_t i;
    for (i = 0; i < 30; ++i) {
        bpt_write_tree(i, gi->root_ids[i]);
    }

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
    ORDER = 100;

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
#ifdef TIME
    fprintf(stderr,
            "giggle_load\tread load_simple_cache\t%lu\n",
            out(load_simple_cache));
#endif

    giggle_set_data_handler();

    return gi;
}
//}}}

//{{{struct gigle_query_result *giggle_query(struct giggle_index *gi,
struct gigle_query_result *giggle_query(struct giggle_index *gi,
                                        char *chrm,
                                        uint32_t start,
                                        uint32_t end)
{
    uint32_t off = 0;
    if (strncmp("chr", chrm, 3) == 0)
        off = 3;


    uint32_t chr_id = giggle_get_chrm_id(gi, chrm + off);
    struct uint32_t_ll *R  = giggle_search(chr_id,
                                           gi->root_ids[chr_id],
                                           start,
                                           end);

    struct gigle_query_result *gqr = (struct gigle_query_result *) 
                malloc(sizeof(struct gigle_query_result));

    gqr->gi = gi;
    gqr->num_files = gi->file_index->num;
    gqr->offsets = (struct long_ll **)
        calloc(gi->file_index->num, sizeof(struct long_ll *));

    uint32_t i,j;
    for (i = 0; i < gi->file_index->num; ++i)
        gqr->offsets[i] = NULL;

    if (R != NULL) {
        struct uint32_t_ll_node *curr = R->head;

#ifdef DEBUG
        fprintf(stderr,
                "giggle_query R->len:%u\n",
                R->len);
#endif

        while (curr != NULL) {
            struct file_id_offset_pair fid_off = 
                    gi->offset_index->vals[curr->val];
            long_ll_append(&(gqr->offsets[fid_off.file_id]),fid_off.offset);
            curr = curr->next;
        }

        uint32_t_ll_free((void **)&R);
        R=NULL;
    } 
#ifdef DEBUG
    else {
        fprintf(stderr,
                "giggle_query R->len:%u\n",
                0);
    }
#endif
    return gqr;
}
//}}}

//{{{struct giggle_query_iter *giggle_get_query_itr(struct gigle_query_result
struct giggle_query_iter *giggle_get_query_itr(struct gigle_query_result *gqr,
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
    if ((*gqi)->sorted_offsets != NULL);
        free((*gqi)->sorted_offsets);
    free(*gqi);
    *gqi = NULL;
}
//}}}
