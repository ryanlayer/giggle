#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include "bpt.h"
#include "lists.h"
#include "ll.h"
#include "util.h"
#include "disk_store.h"

/*
 *  All bpt_nodes have an array of keys, and an array of values.
 *  In non-leaf bpt_nodes, values[i] points to the bpt_node that is either
 *  greater than or equal to key[i-1] or less than key[i].
 */

uint64_t (*serialize_leading)(void *deserialized, uint8_t **serialized) =
        serialize_uint32_t;
uint64_t (*serialize_pointer)(void *deserialized, uint8_t **serialized) =
        serialize_uint32_t;

void (*repair)(struct bpt_node *, struct bpt_node *) = NULL;
uint32_t ORDER = 4;
uint32_t CACHE_SIZE = 10000;

/*
struct cache_def cache = {
    NULL,
    lru_cache_init,
    lru_cache_seen,
    lru_cache_add,
    lru_cache_get,
    lru_cache_remove,
    lru_cache_destroy
};
*/

struct cache_def cache = {
    NULL,
    simple_cache_init,
    simple_cache_seen,
    simple_cache_add,
    simple_cache_get,
    NULL,
    simple_cache_destroy
};

//{{{int b_search(int key, int *D, int D_size)
int b_search(uint32_t key, uint32_t *D, uint32_t D_size)
{
    int lo = -1, hi = D_size, mid;
    while ( hi - lo > 1) {
        mid = (hi + lo) / 2;
        if ( D[mid] < key )
            lo = mid;
        else
            hi = mid;
    }

    return hi;
}
//}}}

//{{{ struct bpt_node *bpt_new_node()
struct bpt_node *bpt_new_node()
{
    if (cache.cache == NULL) {
        cache.cache = cache.init(CACHE_SIZE, NULL);
    }

    struct bpt_node *n = (struct bpt_node *)malloc(sizeof(struct bpt_node));
    n->data = (uint32_t *)calloc(2*ORDER+9, sizeof(uint32_t));

    BPT_ID(n) = cache.seen(cache.cache) + 1;

    cache.add(cache.cache, BPT_ID(n), n, bpt_free_node);

    return n;
}
//}}}

//{{{struct bpt_node *bpt_copy_node(uint32_t *data)
struct bpt_node *bpt_copy_node(uint32_t *data)
{
    struct bpt_node *n = bpt_new_node();
    memcpy(n->data, data, BPT_NODE_SIZE);

    return n;
}
//}}}

//{{{void bpt_free_node(struct bpt_node **n)
void bpt_free_node(void **v)
{
    struct bpt_node **n = (struct bpt_node **) v;
    //fprintf(stderr, "<-- %u\n", BPT_ID(*n));
    free((*n)->data);
    free(*n);
    *n = NULL;
}
//}}}

//{{{ struct bpt_node *bpt_find_leaf(struct bpt_node *curr, int key)
uint32_t bpt_find_leaf(uint32_t curr_id, uint32_t key)
{
    //struct bpt_node *curr = lru_cache_get(cache, curr_id);
    struct bpt_node *curr = cache.get(cache.cache, curr_id);
    if (curr == NULL)
        return 0;

    while (BPT_IS_LEAF(curr) != 1) {
        int i = bpt_find_insert_pos(curr, key);

        if ((i < BPT_NUM_KEYS(curr)) && (BPT_KEYS(curr)[i] == key))
            i+=1;
        //curr = lru_cache_get(cache, BPT_POINTERS(curr)[i]);
        curr = cache.get(cache.cache, BPT_POINTERS(curr)[i]);
    }
    return BPT_ID(curr);
}
//}}}

//{{{int bpt_find_insert_pos(struct bpt_node *leaf, int key)
int bpt_find_insert_pos(struct bpt_node *leaf, uint32_t key)
{
    return b_search(key, BPT_KEYS(leaf), BPT_NUM_KEYS(leaf));
}
//}}}

//{{{struct bpt_node *bpt_split_node(struct bpt_node *root, struct bpt_node
uint32_t bpt_split_node(uint32_t root_id,
                        uint32_t bpt_node_id,
                        uint32_t *lo_result_id,
                        uint32_t *hi_result_id,
                        int *lo_hi_split_point,
                        void (*repair)(struct bpt_node *, struct bpt_node *))

{
#if DEBUG
    {
        fprintf(stderr, "bpt_split_node\n");
        fprintf(stderr, "root_id:%u\tbpt_node_id:%u\n", root_id, bpt_node_id);
    }
#endif

    //struct bpt_node *bpt_node = lru_cache_get(cache, bpt_node_id);
    struct bpt_node *bpt_node = cache.get(cache.cache, bpt_node_id);
    struct bpt_node *n = bpt_new_node();

#if DEBUG
    {
        int i;
        fprintf(stderr, "keys\t");
        for (i = 0; i < BPT_NUM_KEYS(bpt_node); ++i)
            fprintf(stderr, "%u\t", BPT_KEYS(bpt_node)[i]);
        fprintf(stderr, "\n");
    }
#endif

    // set the split location
    int split_point = (ORDER + 1)/2;

    *lo_result_id = BPT_ID(bpt_node);
    *hi_result_id = BPT_ID(n);
    *lo_hi_split_point = split_point;

    // copy the 2nd 1/2 of the values over to the new bpt_node
    int bpt_node_i, new_bpt_node_i = 0;
    for (bpt_node_i = split_point;
         bpt_node_i < BPT_NUM_KEYS(bpt_node);
         ++bpt_node_i) {

        BPT_KEYS(n)[new_bpt_node_i] = BPT_KEYS(bpt_node)[bpt_node_i];
        BPT_POINTERS(n)[new_bpt_node_i] = BPT_POINTERS(bpt_node)[bpt_node_i];
        BPT_NUM_KEYS(n) += 1;
        new_bpt_node_i += 1;
    }

    // if the bpt_node is not a leaf, the far right pointer must be coppied too
    if (BPT_IS_LEAF(bpt_node) == 0) {
        BPT_POINTERS(n)[new_bpt_node_i] = BPT_POINTERS(bpt_node)[bpt_node_i];
        BPT_POINTERS(n)[0] = 0;
    }

    // set status of new bpt_node
    BPT_IS_LEAF(n) = BPT_IS_LEAF(bpt_node);
    BPT_PARENT(n) = BPT_PARENT(bpt_node);

    BPT_NUM_KEYS(bpt_node) = split_point;

    if (BPT_IS_LEAF(bpt_node) == 0) {
        // if the bpt_node is not a leaf, then update the parent pointer of the 
        // children
        for (bpt_node_i = 1; bpt_node_i <= BPT_NUM_KEYS(n); ++bpt_node_i) {
            //struct bpt_node *child = lru_cache_get(cache,
                            //BPT_POINTERS(n)[bpt_node_i]);
            struct bpt_node *child = cache.get(cache.cache,
                                               BPT_POINTERS(n)[bpt_node_i]);
            BPT_PARENT(child) = BPT_ID(n);
        }
    } else {
        // if the bpt_node is a leaf, then connect the two
        BPT_NEXT(n)= BPT_NEXT(bpt_node);
        BPT_NEXT(bpt_node) = BPT_ID(n);
    }

    if (repair != NULL) {
        repair(bpt_node, n);
    }

    if (BPT_ID(bpt_node) == root_id) {
        // if we just split the root, create a new root witha single value
        struct bpt_node *new_root = bpt_new_node();
        BPT_IS_LEAF(new_root) = 0;
        BPT_NUM_KEYS(new_root) += 1;
        BPT_KEYS(new_root)[0] = BPT_KEYS(n)[0];
        BPT_POINTERS(new_root)[0] = BPT_ID(bpt_node); 
        BPT_POINTERS(new_root)[1] = BPT_ID(n); 
        BPT_PARENT(bpt_node) = BPT_ID(new_root);
        BPT_PARENT(n) = BPT_ID(new_root);
        return BPT_ID(new_root);
    } else {
        // if we didnt split the root, place the new value in the parent
        // bpt_node
        int trash_pos;
        uint32_t parent_id =  BPT_PARENT(bpt_node);
        return bpt_place_new_key_value(root_id,
                                       &parent_id,
                                       &trash_pos,
                                       BPT_KEYS(n)[0],
                                       BPT_ID(n));
    }
}
//}}}

//{{{struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
uint32_t bpt_place_new_key_value(uint32_t root_id,
                                 uint32_t *target_id,
                                 int *target_key_pos,
                                 uint32_t key,
                                 uint32_t value_id)
{
#if DEBUG
    {
        fprintf(stderr, "bpt_place_new_key_value\n");
        fprintf(stderr,
                "root_id:%u\ttarget_id:%u\tkey:%u\n",
                root_id,
                *target_id,
                key);
    }
#endif

    //struct bpt_node *target_bpt_node = lru_cache_get(cache, *target_id);
    struct bpt_node *target_bpt_node = cache.get(cache.cache, *target_id);

#if DEBUG
    {
        int i;
        fprintf(stderr, "keys\t");
        for (i = 0; i < BPT_NUM_KEYS(target_bpt_node); ++i)
            fprintf(stderr, "%u\t", BPT_KEYS(target_bpt_node)[i]);
        fprintf(stderr, "\n");
    }
#endif

    int bpt_insert_key_pos = bpt_find_insert_pos(target_bpt_node, key);

    int bpt_insert_value_pos = bpt_insert_key_pos;

    if (BPT_IS_LEAF(target_bpt_node) == 0)
        bpt_insert_value_pos += 1;

    if (BPT_IS_LEAF(target_bpt_node) == 1)
        *target_key_pos = bpt_insert_key_pos;


    if ((BPT_IS_LEAF(target_bpt_node) == 1) &&
         (*target_key_pos < (BPT_NUM_KEYS(target_bpt_node))) &&
        (BPT_KEYS(target_bpt_node)[*target_key_pos] == key )) {

        // If the append function is NULL assume overwrite
        if (append != NULL)
            append(value_id, BPT_POINTERS(target_bpt_node)[*target_key_pos]);
        else
            BPT_POINTERS(target_bpt_node)[*target_key_pos] = value_id;

        return root_id;
    }

    // move everything over
    int i;
    for (i = BPT_NUM_KEYS(target_bpt_node); i > bpt_insert_key_pos; --i) {
        BPT_KEYS(target_bpt_node)[i] = BPT_KEYS(target_bpt_node)[i-1];
    }

    if (BPT_IS_LEAF(target_bpt_node) == 1) {
        for (i = BPT_NUM_KEYS(target_bpt_node); i > bpt_insert_value_pos; --i) 
            BPT_POINTERS(target_bpt_node)[i] = 
                    BPT_POINTERS(target_bpt_node)[i-1];
    } else {
        for (i = BPT_NUM_KEYS(target_bpt_node)+1;
             i > bpt_insert_value_pos;
             --i) {
            BPT_POINTERS(target_bpt_node)[i] = 
                    BPT_POINTERS(target_bpt_node)[i-1];
        }
    }

#if DEBUG
    {
        fprintf(stderr,
                "bpt_insert_key_pos:%u\tbpt_insert_value_pos:%u\n",
                bpt_insert_key_pos,
                bpt_insert_value_pos);
    }
#endif

    BPT_KEYS(target_bpt_node)[bpt_insert_key_pos] = key;
    BPT_POINTERS(target_bpt_node)[bpt_insert_value_pos] = value_id;

    BPT_NUM_KEYS(target_bpt_node) += 1;

    // If there are now too many values in the bpt_node, split it
    if (BPT_NUM_KEYS(target_bpt_node) > ORDER) {
        uint32_t lo_result_id, hi_result_id;
        int lo_hi_split_point = 0;
        uint32_t new_root_id = bpt_split_node(root_id,
                                              BPT_ID(target_bpt_node),
                                              &lo_result_id,
                                              &hi_result_id,
                                              &lo_hi_split_point,
                                              repair);

        //target_bpt_node = lru_cache_get(cache, *target_id);
        target_bpt_node = cache.get(cache.cache, *target_id);

        if (BPT_IS_LEAF(target_bpt_node)) {
            if (bpt_insert_key_pos < lo_hi_split_point)
                *target_id = lo_result_id;
            else {
                *target_id = hi_result_id;
                *target_key_pos = bpt_insert_key_pos - lo_hi_split_point;
            }
        }

        return new_root_id;
    } else {
        return root_id;
    }
}
//}}}

//{{{ uint32_t bpt_insert(uint32_t root_id,
uint32_t bpt_insert(uint32_t root_id,
                    uint32_t key,
                    uint32_t value_id,
                    uint32_t *leaf_id,
                    int *pos)
{
    if (root_id == 0) {
        struct bpt_node *root = bpt_new_node();
        BPT_IS_LEAF(root) = 1;
        BPT_KEYS(root)[0] = key;
        BPT_POINTERS(root)[0] = value_id;
        BPT_NUM_KEYS(root) += 1;

        *leaf_id = BPT_ID(root);
        *pos = 0;

        return BPT_ID(root);
    } else {
        *leaf_id = bpt_find_leaf(root_id, key);

#if DEBUG
        {
            fprintf(stderr, "root_id:%u\tleaf_id:%u\n", root_id, *leaf_id);
        }
#endif

        root_id = bpt_place_new_key_value(root_id,
                                          leaf_id,
                                          pos,
                                          key,
                                          value_id);
        return root_id;
    }
}
//}}}

//{{{uint32_t bpt_insert_new_value(uint32_t root_id,
uint32_t bpt_insert_new_value(uint32_t root_id,
                              uint32_t key,
                              void *value,
                              void (*free_value)(void **data),
                              uint32_t *value_id,
                              uint32_t *leaf_id,
                              int *pos)
{
    //*value_id = cache->seen + 1;
    *value_id = cache.seen(cache.cache) + 1;
    //lru_cache_add(cache, *value_id, value, free_value);
    cache.add(cache.cache, *value_id, value, free_value);
    return bpt_insert(root_id, key, *value_id, leaf_id, pos);
}
//}}}

//{{{ uint32_t bpt_find(uint32_t root_id,
uint32_t bpt_find(uint32_t root_id,
                  uint32_t *leaf_id,
                  int *pos,
                  uint32_t key) 
{
    if (root_id == 0)
        return 0;

    *leaf_id = bpt_find_leaf(root_id, key);
    //struct bpt_node *leaf = lru_cache_get(cache, *leaf_id);
    struct bpt_node *leaf = cache.get(cache.cache, *leaf_id);

    int bpt_insert_key_pos = bpt_find_insert_pos(leaf, key);


    *pos = bpt_insert_key_pos;
    if ((bpt_insert_key_pos + 1) > BPT_NUM_KEYS(leaf)) 
        return 0;
    else if (key != BPT_KEYS(leaf)[bpt_insert_key_pos])
        return 0;
    else {
        return BPT_POINTERS(leaf)[bpt_insert_key_pos];
    }
}
//}}}

//{{{void bpt_write_tree(uint32_t root_id, FILE *f, char *file_name)
void bpt_write_tree(uint32_t root_id, FILE *f, char *file_name)
{
    if (root_id == 0)
        return;
    /*
     * The nodes may not exist in the cache in a way that makes sense to store,
     * so we will do a BFS traversal of the tree and write/renumber the IDs
     * accordingly and store the offsets in an indexed array.
     */

    // Make room in the file for the id -> file offset map so we can come back
    // after and lay it down.
    
    uint32_t num_seen =  cache.seen(cache.cache);

    struct disk_store *ds = disk_store_init(num_seen + 1, &f, file_name);

    struct fifo_q *node_q = NULL, *leaf_q = NULL;
    uint32_t *id;
    id = (uint32_t *)malloc(sizeof(uint32_t));
    *id = root_id;

    struct bpt_node *to_write_node = (struct bpt_node *)
            malloc(sizeof(struct bpt_node));
    to_write_node->data = (uint32_t *)calloc(BPT_NODE_NUM_ELEMENTS,
                                             sizeof(uint32_t));

    // We run through the code an renumber the nodes so that are laid our a
    // nicely

    /*
     * Use old_id_to_new_id_os to maintain the mapping between the IDs that
     * are in memory and those that will be written to disk.
     */
    struct ordered_set *old_id_to_new_id_os =
            ordered_set_init(cache.seen(cache.cache),
                             uint_pair_sort_by_first_element_cmp,
                             uint_pair_search_by_first_element_cmp,
                             uint_pair_search_by_first_key_cmp);

    struct uint_pair *p, *r;

    // put root into a map between the current id and the on-disk id
    p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
    p->first = root_id;
    p->second = old_id_to_new_id_os->num + 1;
    r = ordered_set_add(old_id_to_new_id_os, p);

    fifo_q_push(&node_q, id);

    long node_start_offset = ftell(f);

    while (fifo_q_peek(node_q) != NULL) {
        // Zero out the node that we will write to disk
        memset(to_write_node->data, 0, BPT_NODE_SIZE);

        // Get the current node's id from the queue and data from the cache
        uint32_t *curr_idp = fifo_q_pop(&node_q);
        uint32_t curr_id = *curr_idp;
        free(curr_idp);
        struct bpt_node *curr_node = cache.get(cache.cache, curr_id);

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

        uint32_t i;
        for (i = 0; i <= BPT_NUM_KEYS(curr_node); ++i)
            BPT_KEYS(to_write_node)[i] = BPT_KEYS(curr_node)[i];


        // If the node is a leaf we need to deal with the leading values
        if (BPT_IS_LEAF(curr_node)) {
            if (BPT_LEADING(curr_node) != 0) {
                // put a map between the current id and the to disk id
                p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
                p->first = BPT_LEADING(curr_node);
                p->second = old_id_to_new_id_os->num + 1;
                r = ordered_set_add(old_id_to_new_id_os, p);

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                BPT_LEADING(to_write_node) =  p->second;
            }
        }

        // loop over all the pointers and if the are nodes put them on the q
        uint32_t max_pointer;
        if (BPT_IS_LEAF(curr_node)) 
            max_pointer = BPT_NUM_KEYS(curr_node) - 1;
        else 
            max_pointer = BPT_NUM_KEYS(curr_node);

        for (i = 0; i <= max_pointer; ++i) {
            if (BPT_POINTERS(curr_node)[i] != 0) {
                // put a map between the current id and the to disk id
                p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
                p->first = BPT_POINTERS(curr_node)[i];
                p->second = old_id_to_new_id_os->num + 1;
                r = ordered_set_add(old_id_to_new_id_os, p);

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                // if the node is a leaf, then its pointers are to data not
                // other nodes.  that data will be handled later
                if (! BPT_IS_LEAF(curr_node)) {
                    id = (uint32_t *)malloc(sizeof(uint32_t));
                    *id = BPT_POINTERS(curr_node)[i];
                    fifo_q_push(&node_q, id);
                }

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                BPT_POINTERS(to_write_node)[i] =  p->second;
            }
        }

        // we need to loop back over the leafs so we can write out the 
        // data in the pointers and leading
        if (BPT_IS_LEAF(curr_node)) {
            id = (uint32_t *)malloc(sizeof(uint32_t));
            *id = BPT_ID(curr_node);
            fifo_q_push(&leaf_q, id);
        }

        uint32_t ret = disk_store_append(ds,
                                         to_write_node->data,
                                         BPT_NODE_SIZE);

        if (ret + 1 != BPT_ID(to_write_node))
                errx(1,
                     "Disk write is out of sync.  Saw %u.  Expected %u.",
                     ret + 1, 
                     BPT_ID(to_write_node));
    }

    long node_end_offset = ftell(f);
    long data_start_offset = node_end_offset;

    // Write out the data to disk
    while (fifo_q_peek(leaf_q) != NULL) {
        // Get the current node's id from the queue and data from the cache
        uint32_t *curr_idp = fifo_q_pop(&leaf_q);
        uint32_t curr_id = *curr_idp;
        free(curr_idp);
        struct bpt_node *curr_node = cache.get(cache.cache, curr_id);

        //Write the leading value
        if (BPT_LEADING(curr_node) != 0) {
            // Get the on-disk id
            r = ordered_set_get(old_id_to_new_id_os, &(BPT_LEADING(curr_node)));
            if (r == NULL)
                errx(1,
                     "Node %u has not been seen yet.",
                     BPT_LEADING(curr_node));
            uint32_t on_disk_id = r->second;

            // Get the data
            void *curr_pointer = cache.get(cache.cache, BPT_LEADING(curr_node));

            uint8_t *serialized_data;
            uint64_t serialized_size = serialize_leading(curr_pointer,
                                                         &serialized_data);
            uint32_t ret = disk_store_append(ds,
                                             serialized_data,
                                             serialized_size);
            free(serialized_data);

            if (ret + 1 != on_disk_id)
                errx(1,
                     "Disk write is out of sync.  Saw %u.  Expected %u.",
                     ret + 1, 
                     on_disk_id);
        }

        //Write the pointer values
        uint32_t i;
        for (i = 0; i < BPT_NUM_KEYS(curr_node); ++i) {
            if (BPT_POINTERS(curr_node)[i] != 0) {
                // Get the on-disk id
                r = ordered_set_get(old_id_to_new_id_os,
                                    &(BPT_POINTERS(curr_node)[i]));
                if (r == NULL)
                    errx(1,
                         "Node %u has not been seen yet.",
                         BPT_POINTERS(curr_node)[i]);
                uint32_t on_disk_id = r->second;

                // get the pointer from cache
                void *curr_pointer = cache.get(cache.cache,
                                               BPT_POINTERS(curr_node)[i]);
                if (curr_pointer == NULL)
                    errx(1,
                         "Pointer data %u not in cache.", 
                         BPT_POINTERS(curr_node)[i]);

                uint8_t *serialized_data;
                uint64_t serialized_size = serialize_pointer(curr_pointer,
                                                             &serialized_data);
                uint32_t ret = disk_store_append(ds,
                                                 serialized_data,
                                                 serialized_size);
                free(serialized_data);

                if (ret + 1 != on_disk_id)
                    errx(1,
                         "Disk write is out of sync.  Saw %u.  Expected %u.",
                         ret + 1, 
                         on_disk_id);
            }
        }

    }

    disk_store_sync(ds);
    free(ds->file_name);
    free(ds->offsets);
    free(ds);
    ds = NULL;

    // Move back to the end of the file before we return
    fseek(f, 0, SEEK_END);

    ordered_set_destroy(&old_id_to_new_id_os, free_wrapper);
    free(to_write_node->data);
    free(to_write_node);
}
//}}}

//{{{void bpt_write_tree(uint32_t root_id, FILE *f, char *file_name)
void bpt_write_tree(uint32_t root_id, FILE *f, char *file_name)
{
    if (root_id == 0)
        return;
    /*
     * The nodes may not exist in the cache in a way that makes sense to store,
     * so we will do a BFS traversal of the tree and write/renumber the IDs
     * accordingly and store the offsets in an indexed array.
     */

    // Make room in the file for the id -> file offset map so we can come back
    // after and lay it down.
    
    uint32_t num_seen =  cache.seen(cache.cache);

    struct disk_store *ds = disk_store_init(num_seen + 1, &f, file_name);

    struct fifo_q *node_q = NULL, *leaf_q = NULL;
    uint32_t *id;
    id = (uint32_t *)malloc(sizeof(uint32_t));
    *id = root_id;

    struct bpt_node *to_write_node = (struct bpt_node *)
            malloc(sizeof(struct bpt_node));
    to_write_node->data = (uint32_t *)calloc(BPT_NODE_NUM_ELEMENTS,
                                             sizeof(uint32_t));

    // We run through the code an renumber the nodes so that are laid our a
    // nicely

    /*
     * Use old_id_to_new_id_os to maintain the mapping between the IDs that
     * are in memory and those that will be written to disk.
     */
    struct ordered_set *old_id_to_new_id_os =
            ordered_set_init(cache.seen(cache.cache),
                             uint_pair_sort_by_first_element_cmp,
                             uint_pair_search_by_first_element_cmp,
                             uint_pair_search_by_first_key_cmp);

    struct uint_pair *p, *r;

    // put root into a map between the current id and the on-disk id
    p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
    p->first = root_id;
    p->second = old_id_to_new_id_os->num + 1;
    r = ordered_set_add(old_id_to_new_id_os, p);

    fifo_q_push(&node_q, id);

    long node_start_offset = ftell(f);

    while (fifo_q_peek(node_q) != NULL) {
        // Zero out the node that we will write to disk
        memset(to_write_node->data, 0, BPT_NODE_SIZE);

        // Get the current node's id from the queue and data from the cache
        uint32_t *curr_idp = fifo_q_pop(&node_q);
        uint32_t curr_id = *curr_idp;
        free(curr_idp);
        struct bpt_node *curr_node = cache.get(cache.cache, curr_id);

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

        uint32_t i;
        for (i = 0; i <= BPT_NUM_KEYS(curr_node); ++i)
            BPT_KEYS(to_write_node)[i] = BPT_KEYS(curr_node)[i];


        // If the node is a leaf we need to deal with the leading values
        if (BPT_IS_LEAF(curr_node)) {
            if (BPT_LEADING(curr_node) != 0) {
                // put a map between the current id and the to disk id
                p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
                p->first = BPT_LEADING(curr_node);
                p->second = old_id_to_new_id_os->num + 1;
                r = ordered_set_add(old_id_to_new_id_os, p);

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                BPT_LEADING(to_write_node) =  p->second;
            }
        }

        // loop over all the pointers and if the are nodes put them on the q
        uint32_t max_pointer;
        if (BPT_IS_LEAF(curr_node)) 
            max_pointer = BPT_NUM_KEYS(curr_node) - 1;
        else 
            max_pointer = BPT_NUM_KEYS(curr_node);

        for (i = 0; i <= max_pointer; ++i) {
            if (BPT_POINTERS(curr_node)[i] != 0) {
                // put a map between the current id and the to disk id
                p = (struct uint_pair *) malloc(sizeof(struct uint_pair));
                p->first = BPT_POINTERS(curr_node)[i];
                p->second = old_id_to_new_id_os->num + 1;
                r = ordered_set_add(old_id_to_new_id_os, p);

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                // if the node is a leaf, then its pointers are to data not
                // other nodes.  that data will be handled later
                if (! BPT_IS_LEAF(curr_node)) {
                    id = (uint32_t *)malloc(sizeof(uint32_t));
                    *id = BPT_POINTERS(curr_node)[i];
                    fifo_q_push(&node_q, id);
                }

                if (r->second != p->second)
                    errx(1, "%u has already been seen at %u\n",
                            p->first, r->first);

                BPT_POINTERS(to_write_node)[i] =  p->second;
            }
        }

        // we need to loop back over the leafs so we can write out the 
        // data in the pointers and leading
        if (BPT_IS_LEAF(curr_node)) {
            id = (uint32_t *)malloc(sizeof(uint32_t));
            *id = BPT_ID(curr_node);
            fifo_q_push(&leaf_q, id);
        }

        uint32_t ret = disk_store_append(ds,
                                         to_write_node->data,
                                         BPT_NODE_SIZE);

        if (ret + 1 != BPT_ID(to_write_node))
                errx(1,
                     "Disk write is out of sync.  Saw %u.  Expected %u.",
                     ret + 1, 
                     BPT_ID(to_write_node));
    }

    long node_end_offset = ftell(f);
    long data_start_offset = node_end_offset;

    // Write out the data to disk
    while (fifo_q_peek(leaf_q) != NULL) {
        // Get the current node's id from the queue and data from the cache
        uint32_t *curr_idp = fifo_q_pop(&leaf_q);
        uint32_t curr_id = *curr_idp;
        free(curr_idp);
        struct bpt_node *curr_node = cache.get(cache.cache, curr_id);

        //Write the leading value
        if (BPT_LEADING(curr_node) != 0) {
            // Get the on-disk id
            r = ordered_set_get(old_id_to_new_id_os, &(BPT_LEADING(curr_node)));
            if (r == NULL)
                errx(1,
                     "Node %u has not been seen yet.",
                     BPT_LEADING(curr_node));
            uint32_t on_disk_id = r->second;

            // Get the data
            void *curr_pointer = cache.get(cache.cache, BPT_LEADING(curr_node));

            uint8_t *serialized_data;
            uint64_t serialized_size = serialize_leading(curr_pointer,
                                                         &serialized_data);
            uint32_t ret = disk_store_append(ds,
                                             serialized_data,
                                             serialized_size);
            free(serialized_data);

            if (ret + 1 != on_disk_id)
                errx(1,
                     "Disk write is out of sync.  Saw %u.  Expected %u.",
                     ret + 1, 
                     on_disk_id);
        }

        //Write the pointer values
        uint32_t i;
        for (i = 0; i < BPT_NUM_KEYS(curr_node); ++i) {
            if (BPT_POINTERS(curr_node)[i] != 0) {
                // Get the on-disk id
                r = ordered_set_get(old_id_to_new_id_os,
                                    &(BPT_POINTERS(curr_node)[i]));
                if (r == NULL)
                    errx(1,
                         "Node %u has not been seen yet.",
                         BPT_POINTERS(curr_node)[i]);
                uint32_t on_disk_id = r->second;

                // get the pointer from cache
                void *curr_pointer = cache.get(cache.cache,
                                               BPT_POINTERS(curr_node)[i]);
                if (curr_pointer == NULL)
                    errx(1,
                         "Pointer data %u not in cache.", 
                         BPT_POINTERS(curr_node)[i]);

                uint8_t *serialized_data;
                uint64_t serialized_size = serialize_pointer(curr_pointer,
                                                             &serialized_data);
                uint32_t ret = disk_store_append(ds,
                                                 serialized_data,
                                                 serialized_size);
                free(serialized_data);

                if (ret + 1 != on_disk_id)
                    errx(1,
                         "Disk write is out of sync.  Saw %u.  Expected %u.",
                         ret + 1, 
                         on_disk_id);
            }
        }

    }

    disk_store_sync(ds);
    free(ds->file_name);
    free(ds->offsets);
    free(ds);
    ds = NULL;

    // Move back to the end of the file before we return
    fseek(f, 0, SEEK_END);

    ordered_set_destroy(&old_id_to_new_id_os, free_wrapper);
    free(to_write_node->data);
    free(to_write_node);
}
//}}}

//{{{ uint64_t serialize_uint32_t(void *deserialized, uint8_t **serialized)
uint64_t serialize_uint32_t(void *deserialized, uint8_t **serialized)
{
    uint32_t *data = (uint32_t *)deserialized;
    uint64_t size = sizeof(uint32_t);

    *serialized = (uint8_t *)calloc(1,size);
    memcpy(*serialized, data, size);

    return size;
}
//}}}

#if 0
//{{{void bpt_write_tree(struct bpt_node *root,
void bpt_write_tree(uint32_t root_id,
                    FILE *f,
                    struct ordered_set **addr_to_id,
                    struct indexed_list **id_to_offset_size,
                    long *table_offset)
{
    /*
     * Two ordered sets are used to help managage the serialization.  One is a
     * mapping from memory address to id (addr_to_id), and the other is from id
     * to file offset (id_to_offset).  First the tree is processed using BFS,
     * and the memory address of each not, its pointers, and its leading value
     * are added to the addr_to_id table.  The tree is then processed again
     * using BFS to store the values to file.  Within each node the pointers
     * are replaced with ids from the addr_to_id before saving.  Leaves of the
     * tree contain information that is not saved to disk with this function.
     * These extra values are in the addr_to_id set that need to be added to 
     * id_to_offset table then written to the file at the table_offset offset.
     *
     * The first value in the file is the order, then number of values in the
     * addr_to_id set, and then followed by the nodes
     *
     * Each node is stored in the following order:
     *  is_leaf     -> uin32_t
     *  parent      -> uint32_t
     *  num_keys    -> uin32_t
     *  keys        -> ORDER * uint32_t
     *  pointers    -> (ORDER+1) * uint32_t
     *  next        -> uint32_t
     *  leading     -> uint32_t
     *
     *  Total size is:
     *  (7+ORDER+ORDER+1) * uint32_t -> 8+2*ORDER
     */

    // make table of memory addr -> id
    struct pointer_uint_pair *pu_p, *pu_r;
    *addr_to_id = ordered_set_init(50,
                                   pointer_uint_pair_sort_element_cmp,
                                   pointer_uint_pair_search_element_cmp,
                                   pointer_uint_pair_search_key_cmp);

    uint32_t i, count = 1;

    // Do a BFS search of the tree to make a table of memaddr to ID
    struct fifo_q *q = NULL;
    fifo_q_push(&q, root);
    while (fifo_q_peek(q) != NULL) {
        // Add the current node to the table
        struct bpt_node *curr = (struct bpt_node *)fifo_q_pop(&q);

        pu_p = (struct pointer_uint_pair *)
                malloc(sizeof(struct pointer_uint_pair));
        pu_p->pointer = curr;
        pu_p->uint = count;
        //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
        pu_r = (struct pointer_uint_pair *)
                ordered_set_add(*addr_to_id, pu_p);

        count += 1;

        if (curr->is_leaf == 0) {
            // If the node is not a leaf, put all of its children on the q
            for (i = 0; i <= curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL)
                    fifo_q_push(&q, curr->pointers[i]);
            }
        } else {
            // If it is not a leaf, put the data elements on the q
            for (i = 0; i < curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL) {
                    pu_p = (struct pointer_uint_pair *)
                        malloc(sizeof(struct pointer_uint_pair));
                    pu_p->pointer = curr->pointers[i];
                    pu_p->uint = count;
                    //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
                    pu_r = (struct pointer_uint_pair *)
                            ordered_set_add(*addr_to_id, pu_p);
                    count += 1;
                }
            }
            // If it is not a leaf, put the leading data on the que
            if (curr->leading != NULL) {
                pu_p = (struct pointer_uint_pair *)
                        malloc(sizeof(struct pointer_uint_pair));
                pu_p->pointer = curr->leading;
                pu_p->uint = count;
                //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
                pu_r = (struct pointer_uint_pair *)
                        ordered_set_add(*addr_to_id, pu_p);
                count += 1;
            }
        }
    }

    long top_offset = ftell(f);

    //write ORDER
    if (fwrite(&(ORDER), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing id to offset table size to file");

    //write number of elements in the table
    if (fwrite(&((*addr_to_id)->num), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing id to offset table size to file");

    //make room in the file for the id -> offset table
    char *zero = (char *)calloc(
            (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)),
            sizeof(char));
    *table_offset = ftell(f);
    if (fwrite(zero,
               sizeof(char),
               (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)),
               f) != (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)))
        err(EX_IOERR, "Error writing id to offset table to file");
    free(zero);

    // make table of id -> file offset
    struct uint_offset_size_pair *uo_p, *uo_r;

    /*
    *id_to_offset_size = ordered_set_init(50,
                                     uint_offset_size_pair_sort_element_cmp,
                                     uint_offset_size_pair_search_element_cmp,
                                     uint_offset_size_pair_search_key_cmp);
    */
    *id_to_offset_size = indexed_list_init(50, sizeof(struct offset_size_pair));

    struct byte_array *ba = byte_array_init((8+2*ORDER) * sizeof(uint32_t));

    // Use BFS to then write the nodes to disk, replacing pointers with ids
    // from the pointer/id table
    q = NULL;
    fifo_q_push(&q, root);
    while (fifo_q_peek(q) != NULL) {
        // "clear" out the byte_array
        ba->num = 0;
        struct bpt_node *curr = (struct bpt_node *)fifo_q_pop(&q);

        // get id from pointer/is table
        pu_r = (struct pointer_uint_pair *)ordered_set_get(*addr_to_id, curr);
        uint32_t curr_id = pu_r->uint;

        //serialize
        // is_leaf
        byte_array_append(ba, &(curr->is_leaf), sizeof(uint32_t));
        //fprintf(stderr, "is_leaf:%u\t%u\n", curr_id, ba->num);
   
        // converted parent
        // get parent id 
        if (curr->parent != NULL) {
            pu_r = (struct pointer_uint_pair *)
                ordered_set_get(*addr_to_id, curr->parent);
            uint32_t parent_id = pu_r->uint;
            byte_array_append(ba, &(parent_id), sizeof(uint32_t));
            //fprintf(stderr, "parent:%u\t%u\n", curr_id, ba->num);
        } else {
            byte_array_append_zeros(ba, sizeof(uint32_t));
            //fprintf(stderr, "parent:%u\t%u\n", curr_id, ba->num);
        }

        // num_keys
        byte_array_append(ba, &(curr->num_keys), sizeof(uint32_t));
        //fprintf(stderr, "num_keys:%u\t%u\n", curr_id, ba->num);
       
        // keys
        byte_array_append(ba,
                          curr->keys,
                          curr->num_keys*sizeof(uint32_t));
        //fprintf(stderr, "keys:%u\t%u\n", curr_id, ba->num);

        //fprintf(stderr, "curr->num_keys:%u\n", curr->num_keys);
        
        // converted pointers values
        for (i = 0;
             i <= (curr->is_leaf ? curr->num_keys - 1 : curr->num_keys);
             ++i) {

            if (curr->pointers[i] != NULL) {
                pu_r = (struct pointer_uint_pair *)
                        ordered_set_get(*addr_to_id, curr->pointers[i]);

                if (pu_r == NULL)
                    errx(1, "%u %p not in table", i, curr->pointers[i]);

                uint32_t pointer_id = pu_r->uint;
                byte_array_append(ba, &(pointer_id), sizeof(uint32_t));
                //fprintf(stderr, "pointers:%u\t%u\n", curr_id, ba->num);

            } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "pointers:%u\t%u\n", curr_id, ba->num);
            }
        }

        // converted next value
        if ( (curr->is_leaf == 1) && (curr->next != NULL)) {
            pu_r = (struct pointer_uint_pair *)
                    ordered_set_get(*addr_to_id, curr->next);
            uint32_t next_id = pu_r->uint;
            byte_array_append(ba, &(next_id), sizeof(uint32_t));
            //fprintf(stderr, "next:%u\t%u\n", curr_id, ba->num);
        } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "next:%u\t%u\n", curr_id, ba->num);
        }

        // converted leading
        if (curr->is_leaf == 0) {
            if (curr->leading != NULL) {
               pu_r = (struct pointer_uint_pair *)
                        ordered_set_get(*addr_to_id, curr->leading);

                if (pu_r == NULL)
                    errx(1, "%u %p not in table", i, curr->leading);

                uint32_t leading_id = pu_r->uint;
                byte_array_append(ba, &(leading_id), sizeof(uint32_t));
                //fprintf(stderr, "leading:%u\t%u\n", curr_id, ba->num);
            } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "leading:%u\t%u\n", curr_id, ba->num);
            }
        } 

        // put in id to offset table
        /*
        uo_p = (struct uint_offset_size_pair *)
                malloc(sizeof(struct uint_offset_size_pair));
        uo_p->uint = curr_id;
        uo_p->offset = ftell(f);
        uo_p->size = ba->num;
        uo_r = (struct uint_offset_size_pair *)
                ordered_set_add(*id_to_offset_size, uo_p);
        */
        struct offset_size_pair os_p;
        os_p.offset = ftell(f);
        os_p.size = ba->num;
        int os_p_r = indexed_list_add(*id_to_offset_size, curr_id, &os_p);

        //fprintf(stderr, "num:%u\n", ba->num);
        if (fwrite(ba->data, sizeof(char), ba->num, f) != ba->num)
            err(EX_IOERR, "Error writing node to file");

        if (curr->is_leaf == 0) {
            // If the node is not a leaf, put all of its children on the q
            for (i = 0; i <= curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL)
                    fifo_q_push(&q, curr->pointers[i]);
            }
        }
    }
}
//}}}
#endif

#if 0
//{{{ struct bpt_node *bpt_to_node(void *n)
struct bpt_node *bpt_to_node(void *n)
{
    return (struct bpt_node*)n;
}
//}}}
//{{{void bpt_print_tree(struct bpt_node *curr, int level)
void bpt_print_tree(struct bpt_node *curr, int level)
{
    int i;
    for (i = 0; i < level; ++i)
        printf(" ");

    printf("keys %p(%d)(%d):", curr,curr->num_keys, curr->is_leaf);

    for (i = 0; i < curr->num_keys; ++i)
        printf(" %d", curr->keys[i]);
    printf("\n");

    for (i = 0; i < level; ++i)
        printf(" ");
    printf("values:");

    if (curr->is_leaf == 0) {
        for (i = 0; i <= curr->num_keys; ++i)
            printf(" %p", (struct bpt_node *)curr->pointers[i]);
    } else {
        for (i = 0; i < curr->num_keys; ++i) {
            int *v = (int *)curr->pointers[i];
            printf(" %d", *v);
        }
    }
    printf("\n");

    if (curr->is_leaf == 0) {
        for (i = 0; i <= curr->num_keys; ++i)
            bpt_print_tree((struct bpt_node *)curr->pointers[i], level+1);
    }
}
//}}}
//{{{ void bpt_print_node(struct bpt_node *n)
void bpt_print_node(struct bpt_node *n)
{
    int i;
    if (n->is_leaf)
        printf("+");
    else
        printf("-");
    printf("\t%p\t%p\t",n, n->parent);

    for (i = 0; i < n->num_keys; ++i) {
        if (i!=0)
            printf(" ");
        printf("%u", n->keys[i]);
    }
    printf("\n");
}
//}}}
//{{{void print_values(struct bpt_node *root)
void print_values(struct bpt_node *root)
{
    struct bpt_node *curr = root;
    while (curr->is_leaf != 1) {
        curr = (struct bpt_node *)curr->pointers[0];
    }

    printf("values: ");
    int i, *v;
    while (1) {
        for (i = 0; i < curr->num_keys; ++i) {
            v = (int *)curr->pointers[i];
            printf(" %d", *v);
        }
        if (curr->next == NULL)
            break;
        else
            curr = curr->next;
    }
    printf("\n");
}
//}}}
//{{{ void bpt_destroy_tree(struct bpt_node **curr)
void bpt_destroy_tree(struct bpt_node **curr)
{
#if 1
    if (*curr == NULL) {
        return;
    } else if ((*curr)->is_leaf == 1) {
        free((*curr)->keys);
        free((*curr)->pointers);
        free(*curr);
        *curr = NULL;
        return;
    } else {
        uint32_t i;
        for (i = 0; i <= (*curr)->num_keys; ++i) 
            bpt_destroy_tree((struct bpt_node **)&((*curr)->pointers[i]));
        free((*curr)->keys);
        free((*curr)->pointers);
        free(*curr);
        *curr = NULL;
        return;
    } 
#endif
}
//}}}
#endif

#if 0
//{{{void store(struct bpt_node *root, char *file_name)
void store(struct bpt_node *root, char *file_name)
{
    FILE *f = fopne(file_name, "wb");

    if (!f)
        err(EX_IOERR, "Could not write to '%s'.", file_name);
}
//}}}
//{{{struct bpt_node *bpt_split_node(struct bpt_node *root, struct bpt_node *bpt_node)

/*
struct bpt_node *bpt_split_node(struct bpt_node *root,
                                struct bpt_node *bpt_node,
                                struct bpt_node **lo_result_bpt_node,
                                struct bpt_node **hi_result_bpt_node,
                                int *lo_hi_split_point,
                                void (*repair)(struct bpt_node *,
                                               struct bpt_node *))
*/
uint32_t bpt_split_node(struct bpt_node *root,
                        struct bpt_node *bpt_node,
                        struct bpt_node **lo_result_bpt_node,
                        struct bpt_node **hi_result_bpt_node,
                        int *lo_hi_split_point,
                        void (*repair)(struct bpt_node *, struct bpt_node *))

{

#if DEBUG
    fprintf(stderr, "bpt_split_node():\n");
#endif

    struct bpt_node *n = bpt_new_node();

#if DEBUG
    fprintf(stderr, "bpt_split_node():\tbpt_new_node:%p\n", n);
#endif

    // set the split location
    int split_point = (ORDER + 1)/2;

    *lo_result_bpt_node = bpt_node;
    *hi_result_bpt_node = n;
    *lo_hi_split_point = split_point;

    // copy the 2nd 1/2 of the values over to the new bpt_node
    int bpt_node_i, new_bpt_node_i = 0;
    for (bpt_node_i = split_point;
         bpt_node_i < BPT_NUM_KEYS(bpt_node);
         ++bpt_node_i) {

        BPT_KEYS(n)[new_bpt_node_i] = BPT_KEYS(bpt_node)[bpt_node_i];
        BPT_POINTERS(n)[new_bpt_node_i] = BPT_POINTERS(bpt_node)[bpt_node_i];
        BPT_NUM_KEYS(n) += 1;
        new_bpt_node_i += 1;
    }

    // if the bpt_node is not a leaf, the far right pointer must be coppied too
    if (BPT_IS_LEAF(bpt_node) == 0) {
        BPT_POINTERS(n)[new_bpt_node_i] = BPT_POINTERS(bpt_node)[bpt_node_i];
        BPT_POINTERS(n)[0] = 0;
    }

    // set status of new bpt_node
    BPT_IS_LEAF(n) = BPT_IS_LEAF(bpt_node);
    BPT_PARENT(n) = BPT_PARENT(bpt_node);

    BPT_NUM_KEYS(bpt_node) = split_point;

    if (BPT_IS_LEAF(bpt_node) == 0) {
#if DEBUG
        fprintf(stderr, "bpt_split_node():\tsplit non-leaf\n");
#endif
        // if the bpt_node is not a leaf, then update the parent pointer of the 
        // children
        for (bpt_node_i = 1; bpt_node_i <= BPT_NUM_KEYS(n); ++bpt_node_i) {
            //( (struct bpt_node *)n->pointers[bpt_node_i])->parent = n;
            struct bpt_node *child = lru_cache_get(cache,
                                                   BPT_POINTERS(v)[bpt_node_i]);
            BPT_PARENT(child) = BPT_ID(n);
        }
    } else {
        // if the bpt_node is a leaf, then connect the two
        BPT_NEXT(n)= BPT_NEXT(bpt_node);
        BPT_NEXT(bpt_node) = BPT_ID(n);

#if DEBUG
        fprintf(stderr,
                "bpt_split_node():\tsplit leaf old->next:%p new->next:%p\n",
                BPT_NEXT(bpt_node),
                BPT_NEXT(n));
#endif
    }

    if (repair != NULL) {
        repair(bpt_node, n);
    }

    if (bpt_node == root) {
#if DEBUG
            fprintf(stderr,
                    "bpt_split_node():\tsplit root\tk:%d\n",
                    BPT_KEYS(n)[0]);
#endif
        // if we just split the root, create a new root witha single value
        struct bpt_node *new_root = bpt_new_node();
        BPT_IS_LEAF(new_root) = 0;
        BPT_NUM_KEYS(new_root) += 1;
        BPT_KEYS(new_root)[0] = BPT_KEYS(n)[0];
        BPT_POINTERS(new_root)[0] = BPT_ID(bpt_node); 
        BPT_POINTERS(new_root)[1] = BPT_ID(n); 
        BPT_PARENT(bpt_node) = BPT_ID(new_root);
        BPT_PARENT(n) = BPT_ID(new_root);
        return new_root;
    } else {
#if DEBUG
            fprintf(stderr, "bpt_split_node():\tsplit non-root\n");
#endif
        // if we didnt split the root, place the new value in the parent
        // bpt_node
        return bpt_place_new_key_value(root,
                                       BPT_PARENT(bpt_node),
                                       NULL,
                                       BPT_KEYS(n)[0],
                                       BPT_ID(n));
    }
}
//}}}
//{{{struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
                                         struct bpt_node **target_bpt_node,
                                         int *target_key_pos,
                                         uint32_t key,
                                         void *value)
{
    int bpt_insert_key_pos = bpt_find_insert_pos(*target_bpt_node, key);

    int bpt_insert_value_pos = bpt_insert_key_pos;

    if ((*target_bpt_node)->is_leaf == 0)
        bpt_insert_value_pos += 1;

    if ((*target_bpt_node)->is_leaf == 1)
        *target_key_pos = bpt_insert_key_pos;


    if (((*target_bpt_node)->is_leaf == 1) &&
         (*target_key_pos < ((*target_bpt_node)->num_keys)) &&
        ((*target_bpt_node)->keys[*target_key_pos] == key )) {

        if (append != NULL)
            append(value, &((*target_bpt_node)->pointers[*target_key_pos]));
        else
            (*target_bpt_node)->pointers[*target_key_pos] = value;

        return root;
    }

    // move everything over
    int i;


    for (i = (*target_bpt_node)->num_keys; i > bpt_insert_key_pos; --i) {
        (*target_bpt_node)->keys[i] = (*target_bpt_node)->keys[i-1];
    }

    if ((*target_bpt_node)->is_leaf == 1) {
        for (i = (*target_bpt_node)->num_keys; i > bpt_insert_value_pos; --i) 
            (*target_bpt_node)->pointers[i] = (*target_bpt_node)->pointers[i-1];
    } else {
        for (i = (*target_bpt_node)->num_keys+1;
             i > bpt_insert_value_pos;
             --i) {
            (*target_bpt_node)->pointers[i] = (*target_bpt_node)->pointers[i-1];
        }
    }

    (*target_bpt_node)->keys[bpt_insert_key_pos] = key;
    (*target_bpt_node)->pointers[bpt_insert_value_pos] = value;

    (*target_bpt_node)->num_keys += 1;

    // If there are now too many values in the bpt_node, split it
    if ((*target_bpt_node)->num_keys > ORDER) {
        struct bpt_node *lo_result_bpt_node = NULL, *hi_result_bpt_node = NULL;
        int lo_hi_split_point = 0;
        struct bpt_node *r = bpt_split_node(root,
                                            *target_bpt_node,
                                            &lo_result_bpt_node,
                                            &hi_result_bpt_node,
                                            &lo_hi_split_point,
                                            repair);
        if ((*target_bpt_node)->is_leaf) {
            if (bpt_insert_key_pos < lo_hi_split_point)
                *target_bpt_node = lo_result_bpt_node;
            else {
                *target_bpt_node = hi_result_bpt_node;
                *target_key_pos = bpt_insert_key_pos - lo_hi_split_point;
            }
        }

        return r;
    } else {
        return root;
    }
}
//}}}
#endif
