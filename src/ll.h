#ifndef __LL_H__
#define __LL_H__

#include "bpt.h"

struct uint32_t_ll_bpt_non_leading_data
{
    struct uint32_t_ll *SA, *SE;
};

struct uint32_t_ll_bpt_leading_data
{
    struct uint32_t_ll *B;
};

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
void uint32_t_ll_leading_repair(struct bpt_node *a, struct bpt_node *b);
void *uint32_t_ll_new_non_leading();
void *uint32_t_ll_new_leading();
void uint32_t_ll_non_leading_SA_add_scalar(void *, void *);
void uint32_t_ll_non_leading_SE_add_scalar(void *, void *);
void int32_t_ll_append_non_leading_SE(void *, void *);
void uint32_t_ll_leading_B_add_scalar(void *, void *);
void uint32_t_ll_leading_union_with_B(void **, void *);
void uint32_t_ll_non_leading_union_with_SA_subtract_SE(void **R, void *d);
void uint32_t_ll_non_leading_union_with_SA(void **R, void *d);

#endif
