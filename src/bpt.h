#ifndef __BPT_H__
#define __BPT_H__
#include <stdint.h>
#include <stdio.h>
#include "lists.h"

uint32_t ORDER;
uint32_t CACHE_SIZE;

#define BPT_NODE_SIZE ((2*ORDER+8)*sizeof(uint32_t))

#define BPT_ID(node) ((node)->data[0])
#define BPT_PARENT(node) ((node)->data[1])
#define BPT_IS_LEAF(node) ((node)->data[2])
#define BPT_LEADING(node) ((node)->data[3])
#define BPT_NEXT(node) ((node)->data[4])
#define BPT_NUM_KEYS(node) ((node)->data[5])
#define BPT_KEYS(node) ((node)->data + 6)
#define BPT_POINTERS(node) ((node)->data + (6+ORDER+1))

struct ordered_set *id_to_offset_map;

/*
struct cache_def {
    void *cache;
    void *(*init)(uint32_t size);
    uint32_t (*seen)(void *cache);
    void (*add)(void *cache,
                uint32_t key,
                void *data,
                void (*free_value)(void **data));
    void *(*get)(void *cache, uint32_t key);
    void (*remove)(void *cache, uint32_t key);
    void (*destroy)(void **cache);
};
*/

struct cache_def cache;


struct bpt_node 
{
    // 0 parent
    // 1 is_leaf
    // 2 leading
    // 3 next
    // 4 num_keys
    // [5 ... 5+(ORDER+1)]
    // [(5+(ORDER+1)) ... ((5+ORDER+1)+1)+(ORDER+2))
    //
    // Total size is 5+(ORDER+1)+(ORDER+2) = 2*ORDER + 8
    
    uint32_t *data;
};

struct bpt_node *bpt_new_node();

struct bpt_node *bpt_copy_node(uint32_t *data);

uint32_t bpt_find_leaf(uint32_t curr, uint32_t key);

void bpt_free_node(void **v);

uint32_t bpt_place_new_key_value(uint32_t root_id,
                                 uint32_t *target_id,
                                 int *target_key_pos,
                                 uint32_t key,
                                 uint32_t value_id);

uint32_t bpt_split_node(uint32_t root_id,
                        uint32_t bpt_node_id,
                        uint32_t *lo_result_id,
                        uint32_t *hi_result_id,
                        int *lo_hi_split_point,
                        void (*repair)(struct bpt_node *, struct bpt_node *));

void (*repair)(struct bpt_node *, struct bpt_node *);

void (*append)(uint32_t new_value_id, uint32_t existing_value_id);

struct bpt_node *bpt_to_node(void *n);

uint32_t bpt_insert(uint32_t root_id,
                    uint32_t key,
                    uint32_t value_id,
                    uint32_t *leaf_id,
                    int *pos);

uint32_t bpt_insert_new_value(uint32_t root_id,
                              uint32_t key,
                              void *value,
                              void (*free_value)(void **data),
                              uint32_t *value_id,
                              uint32_t *leaf_id,
                              int *pos);

void bpt_print_tree(struct bpt_node *curr, int level);

void pbt_print_node(struct bpt_node *bpt_node);

int b_search(uint32_t key, uint32_t *D, uint32_t D_size);

int bpt_find_insert_pos(struct bpt_node *leaf, uint32_t key);

uint32_t bpt_find(uint32_t root_id,
                  uint32_t *leaf_id,
                  int *pos,
                  uint32_t key);

void bpt_destroy_tree(struct bpt_node **root);

void bpt_write_tree(struct bpt_node *root,
                    FILE *f,
                    struct ordered_set **addr_to_id,
                    struct indexed_list **id_to_offset_size,
                    long *table_offset);

#endif
