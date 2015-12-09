#include <stdlib.h>
#include <stdint.h>
#include "ll.h"
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
void uint32_t_ll_leading_repair(struct bpt_node *a, struct bpt_node *b)
{
#if DEBUG
    fprintf(stderr, "leading_repair\n");
#endif

    if ( (a->is_leaf == 1) && (b->is_leaf == 1) ) {
        struct uint32_t_ll_bpt_leading_data *d = 
            (struct uint32_t_ll_bpt_leading_data *)
            malloc(sizeof(struct uint32_t_ll_bpt_leading_data));

        d->B = NULL;

        if (a->leading != NULL) {
            struct uint32_t_ll_bpt_leading_data *l =  
                    (struct uint32_t_ll_bpt_leading_data *)a->leading;

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
            struct uint32_t_ll_bpt_non_leading_data *nl = 
                (struct uint32_t_ll_bpt_non_leading_data *)a->pointers[i];

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

//{{{void *uint32_t_ll_new_non_leading()
void *uint32_t_ll_new_non_leading()
{
    struct uint32_t_ll_bpt_non_leading_data *d = 
            (struct uint32_t_ll_bpt_non_leading_data *)
            malloc(sizeof( struct uint32_t_ll_bpt_non_leading_data));
    d->SA = NULL;
    d->SE = NULL;

    return (void *)d;
}
//}}}

//{{{void *uint32_t_ll_new_leading()
void *uint32_t_ll_new_leading()
{
    struct uint32_t_ll_bpt_leading_data *d = 
            (struct uint32_t_ll_bpt_leading_data *)
            malloc(sizeof( struct uint32_t_ll_bpt_leading_data));
    d->B = NULL;

    return (void *)d;
}
//}}}

//{{{void uint32_t_ll_non_leading_SA_add_scalar(void *d, void *v)
void uint32_t_ll_non_leading_SA_add_scalar(void *d, void *v)
{
    struct uint32_t_ll_bpt_non_leading_data *nld =
            (struct uint32_t_ll_bpt_non_leading_data *)d;
    uint32_t *id = (uint32_t *)v;

    uint32_t_ll_uniq_append(&(nld->SA), *id);
}
//}}}

//{{{void uint32_t_ll_non_leading_SE_add_scalar(void *d, void *v)
void uint32_t_ll_non_leading_SE_add_scalar(void *d, void *v)
{
    struct uint32_t_ll_bpt_non_leading_data *nld =
            (struct uint32_t_ll_bpt_non_leading_data *)d;
    uint32_t *id = (uint32_t *)v;

    uint32_t_ll_uniq_append(&(nld->SE), *id);
}
//}}}

//{{{void uint32_t_ll_leading_B_add_scalar(void *d, void *v)
void uint32_t_ll_leading_B_add_scalar(void *d, void *v)
{
    struct uint32_t_ll_bpt_leading_data *ld =
            (struct uint32_t_ll_bpt_leading_data *)d;
    uint32_t *id = (uint32_t *)v;

    uint32_t_ll_uniq_append(&(ld->B), *id);
}
//}}}

//{{{void uint32_t_ll_leading_union_with_B(void **R, void *leading)
void uint32_t_ll_leading_union_with_B(void **R, void *leading)
{
    struct uint32_t_ll_node *curr = 
        ((struct uint32_t_ll_bpt_leading_data *)(leading))->B->head;
    while (curr != NULL) {
#if DEBUG
        fprintf(stderr, "+%u\n", curr->val);
#endif
        uint32_t_ll_uniq_append((struct uint32_t_ll **)R, curr->val);
        curr = curr->next;
    }
}
//}}}

//{{{void uint32_t_ll_non_leading_union_with_SA_subtract_SE(void **R, void *d)
void uint32_t_ll_non_leading_union_with_SA_subtract_SE(void **R, void *d)
{
    struct uint32_t_ll_bpt_non_leading_data *nld = 
            (struct uint32_t_ll_bpt_non_leading_data *) d;
    if (nld != NULL) {
        if ((nld->SA != NULL)) {
            struct uint32_t_ll_node *curr = nld->SA->head;
            while (curr != NULL) {
#if DEBUG
                fprintf(stderr, "+%u\n", curr->val);
#endif
                uint32_t_ll_uniq_append((struct uint32_t_ll **)R, curr->val);
                curr = curr->next;
            }
        }
        if ((nld->SE != NULL)) {
            struct uint32_t_ll_node *curr = nld->SE->head;
            while (curr != NULL) {
#if DEBUG
                fprintf(stderr, "-%u\n", curr->val);
#endif
                uint32_t_ll_remove((struct uint32_t_ll **)R, curr->val);
                curr = curr->next;
            }
        }
    }

}
//}}}

//{{{void uint32_t_ll_non_leading_union_with_SA(void **R, void *d)
void uint32_t_ll_non_leading_union_with_SA(void **R, void *d)
{
    struct uint32_t_ll_bpt_non_leading_data *nld = 
            (struct uint32_t_ll_bpt_non_leading_data *) d;
    if (nld != NULL) {
        if ((nld->SA != NULL)) {
            struct uint32_t_ll_node *curr = nld->SA->head;
            while (curr != NULL) {
#if DEBUG
                fprintf(stderr, "+%u\n", curr->val);
#endif
                uint32_t_ll_uniq_append((struct uint32_t_ll **)R, curr->val);
                curr = curr->next;
            }
        }
    }
}
//}}}
