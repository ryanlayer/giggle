#ifndef __GIGGLE_H__
#define __GIGGLE_H__

#include <stdint.h>
#include "bpt.h"

struct uint32_t_ll_node
{
    uint32_t val;
    struct uint32_t_ll_node *next;
};

struct uint32_t_ll
{
    uint32_t len;
    struct uint32_t_ll_node *head, *tail;
};

void uint32_t_ll_append(struct uint32_t_ll **ll, uint32_t val);
void uint32_t_ll_uniq_append(struct uint32_t_ll **ll, uint32_t val);
void uint32_t_ll_remove(struct uint32_t_ll **ll, uint32_t val);
uint32_t uint32_t_ll_contains(struct uint32_t_ll *ll, uint32_t val);
void uint32_t_ll_free(struct uint32_t_ll **ll);

struct giggle_bpt_non_leading_data
{
    struct uint32_t_ll *SA, *SE;
};

struct giggle_bpt_leading_data
{
    struct uint32_t_ll *B;
};

void leading_repair(struct bpt_node *a, struct bpt_node *b);

uint32_t giggle_insert(struct bpt_node **root,
                       uint32_t start,
                       uint32_t end,
                       uint32_t id);

struct uint32_t_ll *giggle_search(struct bpt_node *root,
                                  uint32_t start,
                                  uint32_t end);


#if 0
struct file_data
{
    uint32_t file_idx;
    uint64_t file_offset;
};

struct giggle
{
    struct node *root;

    char **file_list;
    uint32_t num_files, file_list_size;

    struct file_data *interval_list;
    uint32_t num_intervals, interval_list_size;
};

struct giggle *init_giggle(uint32_t file_num_est,
                           uint32_t interval_num_est);

uint32_t giggle_insert(struct giggle *g,
                       uint32_t file_idx,
                       uint64_t offset,
                       uint32_t start,
                       uint32_t end);

uint32_t add_file_data(struct giggle *g,
                       uint32_t file_idx,
                       uint64_t offset);
#endif


#endif
