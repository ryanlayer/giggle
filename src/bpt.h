#ifndef __BPT_H__
#define __BPT_H__
#include <stdint.h>
#include <stdio.h>
#include "lists.h"

uint32_t ORDER;

struct ordered_set *id_to_offset_map;

struct bpt_node 
{
    struct bpt_node *parent;
    uint32_t *keys, num_keys, is_leaf;
    uint8_t flags;
    void **pointers;
    void *leading;
    struct bpt_node *next;
};

void (*repair)(struct bpt_node *, struct bpt_node *);

void (*append)(void *, void **);

struct bpt_node *bpt_to_node(void *n);

struct bpt_node *bpt_insert(struct bpt_node *root,
                            uint32_t key,
                            void *value,
                            struct bpt_node **leaf,
                            int *pos);

struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
                                         struct bpt_node **target_bpt_node,
                                         int *target_key_pos,
                                         uint32_t key,
                                         void *value);

struct bpt_node *bpt_new_node();

void bpt_print_tree(struct bpt_node *curr, int level);

void pbt_print_node(struct bpt_node *bpt_node);

int b_search(uint32_t key, uint32_t *D, uint32_t D_size);

struct bpt_node *bpt_find_leaf(struct bpt_node *curr, uint32_t key);

int bpt_find_insert_pos(struct bpt_node *leaf, uint32_t key);

struct bpt_node *bpt_split_node(struct bpt_node *root,
                                struct bpt_node *bpt_node,
                                struct bpt_node **lo_result_bpt_node,
                                struct bpt_node **hi_result_bpt_node,
                                int *lo_hi_split_point,
                                void (*repair)(struct bpt_node *,
                                               struct bpt_node *));

void *bpt_find(struct bpt_node *root,
               struct bpt_node **leaf,
               int *pos,
               uint32_t key);

void bpt_destroy_tree(struct bpt_node **root);

void bpt_write_tree(struct bpt_node *root,
                    FILE *f,
                    struct ordered_set **addr_to_id,
                    struct indexed_list **id_to_offset_size,
                    long *table_offset);
#endif
