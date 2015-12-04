#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bpt.h"
#include "giggle.h"

//{{{void uint32_t_ll_append(struct uint32_t_ll **ll, uint32_t val)
void uint32_t_ll_append(struct uint32_t_ll **ll, uint32_t val)
{
    struct uint32_t_ll_node *n = (struct uint32_t_ll_node *)
        malloc(sizeof(struct uint32_t_ll_node));
    n->val = val;
    n->next = NULL;

    if (*ll == NULL) {
        *ll = (struct uint32_t_ll *)malloc(sizeof(struct uint32_t_ll));
        (*ll)->head = n;
        (*ll)->len = 1;
    } else {
        (*ll)->tail->next = n;
        (*ll)->len = (*ll)->len + 1;
    }

    (*ll)->tail = n;
}
//}}}

//{{{void uint32_t_ll_uniq_append(struct uint32_t_ll **ll, uint32_t val)
void uint32_t_ll_uniq_append(struct uint32_t_ll **ll, uint32_t val)
{
    struct uint32_t_ll_node *n = (struct uint32_t_ll_node *)
        malloc(sizeof(struct uint32_t_ll_node));
    n->val = val;
    n->next = NULL;

    if (*ll == NULL) {
        *ll = (struct uint32_t_ll *)malloc(sizeof(struct uint32_t_ll));
        (*ll)->head = n;
        (*ll)->len = 1;
    } else {

        struct uint32_t_ll_node *curr = (*ll)->head;
        while (curr != NULL) {
            if (curr->val == val) {
                free(n);
                return;
            }
            curr = curr->next;
        }

        (*ll)->tail->next = n;
        (*ll)->len = (*ll)->len + 1;
    }

    (*ll)->tail = n;
}
//}}}

//{{{void uint32_t_ll_remove(struct uint32_t_ll **ll, uint32_t val)
void uint32_t_ll_remove(struct uint32_t_ll **ll, uint32_t val)
{
    if (*ll != NULL) {
        struct uint32_t_ll_node *tmp, *last, *curr = (*ll)->head;
        while (curr != NULL) {
            if (curr->val == val) {
                if ((curr == (*ll)->head) && (curr == (*ll)->tail)) {
                    free(curr);
                    free(*ll);
                    *ll = NULL;
                    return;
                } else if (curr == (*ll)->head) {
                    tmp = curr->next;
                    free(curr);
                    (*ll)->head = tmp;
                    curr = tmp;
                    (*ll)->len = (*ll)->len - 1;
                } else if (curr == (*ll)->tail) {
                    free(curr);
                    curr = NULL;
                    (*ll)->tail = last;
                    last->next = NULL;
                    (*ll)->len = (*ll)->len - 1;
                } else {
                    tmp = curr;
                    last->next = tmp->next;
                    curr = tmp->next;
                    free(tmp);
                    (*ll)->len = (*ll)->len - 1;
                }
            } else {
                last = curr;
                curr = curr->next;
            }
        }
    }
}
//}}}

//{{{int uint32_t_ll_contains(struct uint32_t_ll *ll, uint32_t val)
uint32_t uint32_t_ll_contains(struct uint32_t_ll *ll, uint32_t val)
{
    if (ll == NULL)
       return 0; 

    uint32_t r = 0;
    
    struct uint32_t_ll_node *curr = ll->head;
    while (curr != NULL) {
        if (curr->val == val)
            r += 1;
        curr = curr->next;
    }

    return r;
}
//}}}

//{{{void uint32_t_ll_free(struct uint32_t_ll **ll)
void uint32_t_ll_free(struct uint32_t_ll **ll)
{
    struct uint32_t_ll_node *curr, *tmp;

    if (*ll != NULL) {
        curr = (*ll)->head;
        while (curr != NULL) {
            tmp = curr->next;
            free(curr);
            curr = tmp;
        }

        free(*ll);
        *ll = NULL;
    }
}
//}}}

//{{{void leading_repair(struct bpt_node *a, struct bpt_node *b)
void leading_repair(struct bpt_node *a, struct bpt_node *b)
{
#if DEBUG
    fprintf(stderr, "leading_repair\n");
#endif

    if ( (a->is_leaf == 1) && (b->is_leaf == 1) ) {
        struct giggle_bpt_leading_data *d = 
            (struct giggle_bpt_leading_data *)
            malloc(sizeof(struct giggle_bpt_leading_data));

        d->B = NULL;

        if (a->leading != NULL) {
            struct giggle_bpt_leading_data *l =  
                    (struct giggle_bpt_leading_data *)a->leading;

            struct uint32_t_ll_node *curr = l->B->head;

            while (curr != NULL) {
#if DEBUG
                fprintf(stderr, "+ %u\n", curr->val);
#endif
                uint32_t_ll_uniq_append(&(d->B), curr->val);
                curr = curr->next;
            }
        }

        uint32_t i;
        for (i = 0 ; i < a->num_keys; ++i) {
            struct giggle_bpt_non_leading_data *nl = 
                (struct giggle_bpt_non_leading_data *)a->pointers[i];

            if (nl->SA != NULL) {
                struct uint32_t_ll_node *curr = nl->SA->head;
                while (curr != NULL) {
                    uint32_t_ll_uniq_append(&(d->B), curr->val);
#if DEBUG
                    fprintf(stderr, "+ %u\n", curr->val);
#endif
                    curr = curr->next;
                }
            }

            if (nl->SE != NULL) {
                struct uint32_t_ll_node *curr = nl->SE->head;
                while (curr != NULL) {
                    uint32_t_ll_remove(&(d->B), curr->val);
#if DEBUG
                    fprintf(stderr, "- %u\n", curr->val);
#endif
                    curr = curr->next;
                }
            }
        }

        if (d->B != NULL)
            b->leading = d;
    }
}
//}}}

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
    repair_func = leading_repair;

    struct bpt_node *start_leaf_r = NULL;
    int start_pos_r;

    struct giggle_bpt_non_leading_data *start_r = 
        (struct giggle_bpt_non_leading_data*)bpt_find(*root,
                                                      &start_leaf_r,
                                                      &start_pos_r,
                                                      start);
    if (start_r == NULL) {
        struct giggle_bpt_non_leading_data *d = 
            (struct giggle_bpt_non_leading_data *)
            malloc(sizeof( struct giggle_bpt_non_leading_data));
        d->SA = NULL;
        d->SE = NULL;

        uint32_t_ll_uniq_append(&(d->SA), id);
        *root = bpt_insert(*root, start, d, &start_leaf_r, &start_pos_r);
    } else {
        uint32_t_ll_uniq_append(&(start_r->SA), id);
    }

    struct bpt_node *end_leaf_r = NULL;
    int end_pos_r;

    struct giggle_bpt_non_leading_data *end_r = 
        (struct giggle_bpt_non_leading_data*)bpt_find(*root,
                                                      &end_leaf_r,
                                                      &end_pos_r,
                                                      end + 1);
    if (end_r == NULL) {
        struct giggle_bpt_non_leading_data *d = 
            (struct giggle_bpt_non_leading_data *)
            malloc(sizeof( struct giggle_bpt_non_leading_data));
        d->SA = NULL;
        d->SE = NULL;

        uint32_t_ll_uniq_append(&(d->SE), id);
        *root = bpt_insert(*root, end + 1, d, &end_leaf_r, &end_pos_r);
    } else {
        uint32_t_ll_uniq_append(&(end_r->SE), id);
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
                struct giggle_bpt_leading_data *d = 
                        (struct giggle_bpt_leading_data *)
                        malloc(sizeof( struct giggle_bpt_leading_data));
                d->B = NULL;
                uint32_t_ll_uniq_append(&(d->B), id);
            } else {
                struct giggle_bpt_leading_data *d = 
                    (struct giggle_bpt_leading_data *)(curr_leaf->leading);
                uint32_t_ll_uniq_append(&(d->B), id);
            }
        } while (curr_leaf != end_leaf_r);
    }

    return 0;
}
//}}}

//{{{struct uint32_t_ll *giggle_search(struct bpt_node *root,
struct uint32_t_ll *giggle_search(struct bpt_node *root,
                                  uint32_t start,
                                  uint32_t end)
{
#if DEBUG
    fprintf(stderr, "giggle_search\n");
    fprintf(stderr, "start:%u\tend:%u\n", start, end);
#endif

    if (root == NULL)
        return NULL;

    struct uint32_t_ll *r = NULL;


    struct giggle_bpt_non_leading_data *nld_start_r, *nld_end_r;
    struct bpt_node *leaf_start_r, *leaf_end_r;
    int pos_start_r, pos_end_r;

    nld_start_r = (struct giggle_bpt_non_leading_data *)bpt_find(root,
                                                                 &leaf_start_r, 
                                                                 &pos_start_r,
                                                                 start);
#if DEBUG
    fprintf(stderr, "pos_start_r:%d\t", pos_start_r);
#endif


    nld_end_r = (struct giggle_bpt_non_leading_data *)bpt_find(root,
                                                               &leaf_end_r, 
                                                               &pos_end_r,
                                                               end);
#if DEBUG
    fprintf(stderr, "pos_end_r:%d\n", pos_end_r);
#endif


    // get everything in the leading value
    if (leaf_start_r->leading != NULL) {
        struct uint32_t_ll_node *curr = ((struct giggle_bpt_leading_data *)
            (leaf_start_r->leading))->B->head;
        while (curr != NULL) {
#if DEBUG
            fprintf(stderr, "+%u\n", curr->val);
#endif
            uint32_t_ll_uniq_append(&r, curr->val);
            curr = curr->next;
        }
    }

    // add any SA and remove any that are an SE up to and including this point
    uint32_t i;
    struct giggle_bpt_non_leading_data *nld;
    for (i = 0; (i < leaf_start_r->num_keys) && (i <= pos_start_r); ++i) {
        nld = (struct giggle_bpt_non_leading_data *)
                (leaf_start_r->pointers[i]);
        if (nld != NULL) {
            if ((nld->SA != NULL)) {
                struct uint32_t_ll_node *curr = nld->SA->head;
                while (curr != NULL) {
#if DEBUG
                    fprintf(stderr, "+%u\n", curr->val);
#endif
                    uint32_t_ll_uniq_append(&r, curr->val);
                    curr = curr->next;
                }
            }
            if ((nld->SE != NULL)) {
                struct uint32_t_ll_node *curr = nld->SE->head;
                while (curr != NULL) {
#if DEBUG
                    fprintf(stderr, "-%u\n", curr->val);
#endif
                    uint32_t_ll_remove(&r, curr->val);
                    curr = curr->next;
                }
            }
        }
    }

    // now process everything in between the start and end
    struct bpt_node *leaf_curr = leaf_start_r;
    int pos_curr = pos_start_r + 1;

    // any intermediate leaves
    while (leaf_curr != leaf_end_r) {

        // do from pos_curr to the last key
        for (i = pos_curr; i < leaf_curr->num_keys; ++i) {
            nld = (struct giggle_bpt_non_leading_data *)
                    (leaf_curr->pointers[i]);
            if ((nld != NULL) && (nld->SA != NULL)) {
                struct uint32_t_ll_node *curr = nld->SA->head;
                while (curr != NULL) {
#if DEBUG
                    fprintf(stderr, "+%u\n", curr->val);
#endif
                    uint32_t_ll_uniq_append(&r, curr->val);
                    curr = curr->next;
                }
            }
        }

        leaf_curr = leaf_curr->next;
        pos_curr = 0;
    }

    if (leaf_curr == leaf_end_r) {
        // add all SA's from here to either the end point
        for ( i = pos_curr;
             (i < leaf_curr->num_keys) && (i <= pos_end_r); 
              ++i) {
            nld = (struct giggle_bpt_non_leading_data *)
                    (leaf_curr->pointers[i]);
            if ((nld != NULL) && (nld->SA != NULL)) {
                struct uint32_t_ll_node *curr = nld->SA->head;
                while (curr != NULL) {
#if DEBUG
                    fprintf(stderr, "+%u\n", curr->val);
#endif
                    uint32_t_ll_uniq_append(&r, curr->val);
                    curr = curr->next;
                }
            }

        }
    }

    return r;
}
//}}}
