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

//{{{
int str_uint_pair_sort_element_cmp(const void *a, const void *b)
{
    struct str_uint_pair **pa = (struct str_uint_pair **)a;
    struct str_uint_pair **pb = (struct str_uint_pair **)b;
    return strcmp((*pa)->str, (*pb)->str);
}

int str_uint_pair_search_element_cmp(const void *a, const void *b)
{
    struct str_uint_pair *key = (struct str_uint_pair *)a;
    struct str_uint_pair **arg = (struct str_uint_pair **)b;
    return strcmp(key->str, (*arg)->str);
}

int str_uint_pair_search_key_cmp(const void *a, const void *b)
{
    char *key = (char *)a;
    struct str_uint_pair **arg = (struct str_uint_pair **)b;
    return strcmp(key, (*arg)->str);
}
//}}}
