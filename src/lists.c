#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>
#include <string.h>

#include "lists.h"

//{{{struct unordered_list *unordered_list_init(uint32_t init_size)
struct unordered_list *unordered_list_init(uint32_t init_size)
{
    struct unordered_list *us = (struct unordered_list *)
            malloc(sizeof(struct unordered_list));
    us->num = 0;
    us->size = init_size;

    us->data = (void **) malloc(init_size * sizeof(void *));

    return us;
}
//}}}

//{{{void unordered_list_destroy(struct unordered_list **ul)
void unordered_list_destroy(struct unordered_list **ul)
{
    free((*ul)->data);
    (*ul)->data = NULL;

    free(*ul);
    *ul = NULL;
}
//}}}

//{{{uint32_t unordered_list_add(struct unordered_list *ul,
uint32_t unordered_list_add(struct unordered_list *ul,
                      void *data)
{
    uint32_t id = ul->num;

    ul->num = ul->num + 1;

    // check to see if there is enough space, if not grow by double
    if ( ul->size == ul->num ) {
        ul->size = ul->size * 2;
        ul->data = (void *)realloc(ul->data, 
                                   ul->size * sizeof(void *));
    }

    ul->data[id] = data;
    
    return id;
}
//}}}

//{{{void *unordered_list_get(struct unordered_list *ul, uint32_t i)
void *unordered_list_get(struct unordered_list *ul, uint32_t i)
{
    if (i >= ul->num)
        return NULL;

    return ul->data[i];
}
//}}}

//{{{ struct ordered_set *ordered_set_init(uint32_t init_size,
struct ordered_set 
        *ordered_set_init(
                uint32_t init_size,
                int (*sort_element_cmp)(const void *a, const void *b),
                int (*search_element_cmp)(const void *a, const void *b),
                int (*search_key_cmp)(const void *a, const void *b))
{
    struct ordered_set *os = (struct ordered_set *)
                                malloc(sizeof(struct ordered_set));
    os->num = 0;
    os->size = init_size;
    os->data = (void **) malloc(init_size * sizeof(void *));
    os->sort_element_cmp = sort_element_cmp;
    os->search_element_cmp = search_element_cmp;
    os->search_key_cmp = search_key_cmp;

    return os;
}
//}}}

//{{{void *ordered_set_add(struct ordered_set *os,
void *ordered_set_add(struct ordered_set *os,
                      void *data)
{
    void **p  = NULL;

    if (os->num != 0)
        p = bsearch(data,
                    os->data,
                    os->num,
                    sizeof(void *),
                    os->search_element_cmp); 

    if (p != NULL) {
        return *p;
    } else {
        // make space if needed
        if (os->num == os->size - 1) {
            os->size = os->size * 2;
            os->data = realloc(os->data, os->size * sizeof(void *));
        }

        os->data[os->num] = data;
        os->num = os->num + 1;

        qsort(os->data, os->num, sizeof(void *), os->sort_element_cmp);

        return data;
    }
}
//}}}

//{{{ void *ordered_set_get(struct unordered_list *os, void *key);
void *ordered_set_get(struct ordered_set *os, void *key)
{

    if (os->num != 0) {
        void **p = bsearch(key,
                           os->data,
                           os->num,
                           sizeof(void *),
                           os->search_key_cmp); 
        if (p == NULL)
            return p;
        else
            return *p;
    } else {
        return NULL;
    }
}
//}}}

//{{{ str_uint_pair
//{{{ int str_uint_pair_sort_element_cmp(const void *a, const void *b)
int str_uint_pair_sort_element_cmp(const void *a, const void *b)
{
    struct str_uint_pair **pa = (struct str_uint_pair **)a;
    struct str_uint_pair **pb = (struct str_uint_pair **)b;
    return strcmp((*pa)->str, (*pb)->str);
}
//}}}

//{{{int str_uint_pair_search_element_cmp(const void *a, const void *b)
int str_uint_pair_search_element_cmp(const void *a, const void *b)
{
    struct str_uint_pair *key = (struct str_uint_pair *)a;
    struct str_uint_pair **arg = (struct str_uint_pair **)b;
    return strcmp(key->str, (*arg)->str);
}
//}}}

//{{{int str_uint_pair_search_key_cmp(const void *a, const void *b)
int str_uint_pair_search_key_cmp(const void *a, const void *b)
{
    char *key = (char *)a;
    struct str_uint_pair **arg = (struct str_uint_pair **)b;
    return strcmp(key, (*arg)->str);
}
//}}}
//}}}

//{{{ pointer_uint_pair
//{{{ int pointer_uint_pair_sort_element_cmp(const void *a, const void *b)
int pointer_uint_pair_sort_element_cmp(const void *a, const void *b)
{
    /*
    fprintf(stderr,
            "pointer_uint_pair_sort_element_cmp:%p %p\n",
            (*pa)->pointer,
            (*pb)->pointer,
            */


    struct pointer_uint_pair **pa = (struct pointer_uint_pair **)a;
    struct pointer_uint_pair **pb = (struct pointer_uint_pair **)b;
    if ((*pa)->pointer < (*pb)->pointer)
        return -1;
    else if ((*pa)->pointer > (*pb)->pointer)
        return 1;
    else
        return 0;
}
//}}}

//{{{int pointer_uint_pair_search_element_cmp(const void *a, const void *b)
int pointer_uint_pair_search_element_cmp(const void *a, const void *b)
{
    struct pointer_uint_pair *key = (struct pointer_uint_pair *)a;
    struct pointer_uint_pair **arg = (struct pointer_uint_pair **)b;

    if (key->pointer < (*arg)->pointer)
        return -1;
    else if (key->pointer > (*arg)->pointer)
        return 1;
    else
        return 0;
}
//}}}

//{{{int pointer_uint_pair_search_key_cmp(const void *a, const void *b)
int pointer_uint_pair_search_key_cmp(const void *a, const void *b)
{
    struct pointer_uint_pair **arg = (struct pointer_uint_pair **)b;

    if (a < (*arg)->pointer)
        return -1;
    else if (a > (*arg)->pointer)
        return 1;
    else
        return 0;

}
//}}}
//}}}

//{{{ uint_offset_pair
//{{{ int uint_offset_pair_sort_element_cmp(const void *a, const void *b)
int uint_offset_pair_sort_element_cmp(const void *a, const void *b)
{
    struct uint_offset_pair **pa = (struct uint_offset_pair **)a;
    struct uint_offset_pair **pb = (struct uint_offset_pair **)b;

    return (*pa)->uint - (*pb)->uint;
}
//}}}

//{{{int uint_offset_pair_search_element_cmp(const void *a, const void *b)
int uint_offset_pair_search_element_cmp(const void *a, const void *b)
{
    struct uint_offset_pair *key = (struct uint_offset_pair *)a;
    struct uint_offset_pair **arg = (struct uint_offset_pair **)b;

    return key->uint - (*arg)->uint;

}
//}}}

//{{{int uint_offset_pair_search_key_cmp(const void *a, const void *b)
int uint_offset_pair_search_key_cmp(const void *a, const void *b)
{
    uint32_t *key = (uint32_t *)a;
    struct uint_offset_pair **arg = (struct uint_offset_pair **)b;

    return *key - (*arg)->uint;
}
//}}}
//}}}

//{{{void fifo_q_push(struct fifo_q **q, void *val)
void fifo_q_push(struct fifo_q **q, void *val)
{
    int *v = (int *)val;
    struct fifo_q *n = (struct fifo_q *)malloc(sizeof(struct fifo_q));
    n->val = val;
    n->next = *q;

    *q = n;
}
//}}}

//{{{void *fifo_q_pop(struct fifo_q **q)
void *fifo_q_pop(struct fifo_q **q)
{
    if (*q == NULL)
        return NULL;

    struct fifo_q *t = *q;
    void *r = t->val;
    *q = (*q)->next;
    free(t);
    return r;
}
//}}}

//{{{void *fifo_q_peek(struct fifo_q *q)
void *fifo_q_peek(struct fifo_q *q)
{
    if (q == NULL)
        return NULL;
    else
        return q->val;
}
//}}}

