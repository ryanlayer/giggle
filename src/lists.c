#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>
#include <sysexits.h>
#include <string.h>

#include "lists.h"
#include "util.h"

//{{{ bit_map

struct bit_map *bit_map_init(uint64_t bits)
{
    struct bit_map *b = (struct bit_map *) malloc(sizeof(struct bit_map));
    b->num_bits = bits;
    b->num_ints = (bits + 32 - 1) / 32;
    b->bm = (uint32_t *) calloc(b->num_ints, sizeof(uint32_t));
    return b;
}

struct bit_map *bit_map_load(FILE *f, char *file_name)
{
    struct bit_map *b = (struct bit_map *) malloc(sizeof(struct bit_map));

    size_t fr = fread(&(b->num_bits), sizeof(uint64_t), 1, f);
    check_file_read(file_name, f, 1, fr);
    fr = fread(&(b->num_ints), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);
    b->bm = (uint32_t *) calloc(b->num_ints, sizeof(uint32_t));
    fr = fread(b->bm, sizeof(uint32_t), b->num_ints, f);
    check_file_read(file_name, f, b->num_ints, fr);

    return b;
}

void bit_map_store(struct bit_map *b, FILE *f, char *file_name)
{
    if (fwrite(&(b->num_bits), sizeof(uint64_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing number of bits to '%s'.",
            file_name);

    if (fwrite(&(b->num_ints), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing number of ints to '%s'.",
            file_name);

    if (fwrite(b->bm, sizeof(uint32_t), b->num_ints, f) != b->num_ints)
        err(EX_IOERR,
            "Error writing number bitmap to '%s'.",
            file_name);
}

void bit_map_destroy(struct bit_map **b)
{
    free((*b)->bm);
    free(*b);
    *b = NULL;
}

void bit_map_set(struct bit_map *b, uint64_t i)
{
    while (i >= b->num_bits) {
        uint64_t new_bits = (b->num_bits)*2;
        uint32_t new_ints = (new_bits + 32 - 1) / 32;
        uint32_t *new_bm = (uint32_t *)calloc(new_ints, sizeof(uint32_t));
        memcpy(new_bm, b->bm, (b->num_ints)*sizeof(uint32_t));

        free(b->bm);
        b->num_bits = new_bits;
        b->num_ints = new_ints;
        b->bm = new_bm;
    }


    b->bm[i/32] |= 1 << (31 - (i%32));
}

uint32_t bit_map_get(struct bit_map *b, uint64_t q)
{
    if (q > b->num_bits)
        return 0;

    return (( b->bm[q/32]) >> (31 - (q%32)) & 1);
}

//}}}

//{{{ indexed_list
//{{{ struct indexed_list *indexed_list_init(uint32_t init_size,
struct indexed_list *indexed_list_init(uint64_t init_size,
                                       uint32_t element_size)
{
    struct indexed_list *il = (struct indexed_list *)
            calloc(1,sizeof(struct indexed_list));
    il->size = init_size;
    il->element_size = element_size;
    il->data = (char *) calloc(init_size, element_size);
    il->bm = bit_map_init(init_size);
    return il;
}
//}}}

//{{{void indexed_list_destroy(struct indexed_list **il)
void indexed_list_destroy(struct indexed_list **il)
{
    if (*il != NULL) {
        bit_map_destroy(&((*il)->bm));
        free((*il)->data);
        free(*il);
        *il = NULL;
    }
}
//}}}

//{{{uint32_t indexed_list_add(struct indexed_list *il,
uint32_t indexed_list_add(struct indexed_list *il,
                          uint64_t index,
                          void *data)
{
    uint32_t r = 0;
    while (index >= il->size) {
        uint32_t old_size = il->size;
        il->size = il->size * 2;
        il->data = (char *)realloc(il->data, il->size * il->element_size);
        memset(il->data + old_size * il->element_size,
               0,
               old_size * il->element_size);
        r = 1;
    }

    bit_map_set(il->bm, index);

    memcpy(il->data + (index * il->element_size), data, il->element_size);

    return r;
}
//}}}

//{{{void *indexed_list_get(struct indexed_list *il, uint32_t index)
void *indexed_list_get(struct indexed_list *il, uint64_t index)
{
    //fprintf(stderr, "%d\n", bit_map_get(il->bm, index));
    if (bit_map_get(il->bm, index) != 0)
        return il->data + (index * il->element_size);
    else
        return NULL;
}
//}}}

//{{{void indexed_list_write(struct indexed_list *il, FILE *f, char *file_name)
void indexed_list_write(struct indexed_list *il, FILE *f, char *file_name)
{
    if (fwrite(&(il->size), sizeof(uint64_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing indexed list size to '%s'.",
            file_name);

    if (fwrite(&(il->element_size), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing indexed list element size to '%s'.",
            file_name);

    if (fwrite(il->data, il->element_size, il->size, f) != il->size)
        err(EX_IOERR,
            "Error writing indexed list data to '%s'.",
            file_name);

    bit_map_store(il->bm, f, file_name);
}
//}}}

//{{{struct indexed_list *indexed_list_load(FILE *f, char *file_name)
struct indexed_list *indexed_list_load(FILE *f, char *file_name)
{
    struct indexed_list *il = (struct indexed_list *)
            malloc(sizeof(struct indexed_list));

    size_t fr = fread(&(il->size), sizeof(uint64_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    fr = fread(&(il->element_size), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    il->data = (char *) calloc(il->size, il->element_size);
    fr = fread(il->data, il->element_size, il->size, f);
    check_file_read(file_name, f, il->size, fr);

    il->bm = bit_map_load(f, file_name);

    return il;
}
//}}}

//}}}

//{{{unordered_list
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
void unordered_list_destroy(struct unordered_list **ul,
                            void (*free_data)(void **data))
{
    if (free_data != NULL) {
        uint32_t i;
        for (i = 0; i < (*ul)->num; ++i) {
            free_data(&((*ul)->data[i]));
        }
    }

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

void unordered_list_store(struct unordered_list *ul, 
                          FILE *f,
                          char *file_name,
                          void (*ul_store)(void *v, FILE *f, char *file_name))
{
    if (fwrite(&(ul->num), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing unordered_list num '%s'.", file_name);

    if (fwrite(&(ul->size), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing unordered_list num '%s'.", file_name);
    uint32_t i;
    for (i = 0; i < ul->num; ++i) {
        ul_store(ul->data[i], f, file_name);
    }
}

struct unordered_list *unordered_list_load(
                FILE *f,
                char *file_name,
                void *(*ul_load)(FILE *f, char *file_name))
{

    struct unordered_list *ul = (struct unordered_list *)
                                malloc(sizeof(struct unordered_list));
    size_t fr = fread(&(ul->num), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    fr = fread(&(ul->size), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    ul->data = (void **) calloc(ul->size, sizeof(void *));

    uint32_t i;
    for (i = 0; i < ul->num; ++i) {
        ul->data[i] = ul_load(f, file_name);
    }

    return ul;
}
//}}}

//{{{ordered_set
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
    os->data = (void **) calloc(init_size, sizeof(void *));
    os->sort_element_cmp = sort_element_cmp;
    os->search_element_cmp = search_element_cmp;
    os->search_key_cmp = search_key_cmp;

    return os;
}
//}}}

//{{{ void ordered_set_destroy(struct ordered_set **os,
void ordered_set_destroy(struct ordered_set **os,
                         void (*free_data)(void **data))
{
    if (free_data != NULL) {
        uint32_t i;
        for (i = 0; i < (*os)->num; ++i) {
            free_data(&((*os)->data[i]));
        }
    }

    free((*os)->data);
    free(*os);
    *os = NULL;
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
       // only qsort if we have to.
       if ((os->num > 1) && (((*(os->sort_element_cmp))(&(os->data[os->num-2]),
                                                        &data))) != -1) {
                       qsort(os->data, os->num, sizeof(void *), os->sort_element_cmp);
       }
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

//{{{void ordered_set_store(struct ordered_set *os, 
void ordered_set_store(struct ordered_set *os, 
                       FILE *f,
                       char *file_name,
                       void (*os_store)(void *v, FILE *f, char *file_name))
{
    if (fwrite(&(os->num), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing ordered_set num '%s'.", file_name);

    if (fwrite(&(os->size), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing ordered_set num '%s'.", file_name);
    uint32_t i;
    for (i = 0; i < os->num; ++i) {
        os_store(os->data[i], f, file_name);
    }
}
//}}}

//{{{struct ordered_set *ordered_set_load(
struct ordered_set 
        *ordered_set_load(
                FILE *f,
                char *file_name,
                void *(*os_load)(FILE *f, char *file_name),
                int (*sort_element_cmp)(const void *a, const void *b),
                int (*search_element_cmp)(const void *a, const void *b),
                int (*search_key_cmp)(const void *a, const void *b))
{

    struct ordered_set *os = (struct ordered_set *)
                                malloc(sizeof(struct ordered_set));
    size_t fr = fread(&(os->num), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    fr = fread(&(os->size), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    os->data = (void **) calloc(os->size, sizeof(void *));

    uint32_t i;
    for (i = 0; i < os->num; ++i) {
        os->data[i] = os_load(f, file_name);
    }

    os->sort_element_cmp = sort_element_cmp;
    os->search_element_cmp = search_element_cmp;
    os->search_key_cmp = search_key_cmp;
    return os;
}
//}}}
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

//{{{void str_uint_pair_store(void *v, FILE *f, char *file_name)
void str_uint_pair_store(void *v, FILE *f, char *file_name)
{
    struct str_uint_pair *p = (struct str_uint_pair *)v;

    if (fwrite(&(p->uint), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing uint of str_uint_pair in '%s'.", file_name);

    uint32_t str_len = strlen(p->str);

    if (fwrite(&str_len, sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR,
            "Error writing str len of str_uint_pair in '%s'.", file_name);

    if (fwrite(p->str, sizeof(char), strlen(p->str), f) != strlen(p->str))
        err(EX_IOERR,
            "Error writing str of str_uint_pair in '%s'.", file_name);
}
//}}}

//{{{void *str_uint_pair_load(FILE *f, char *file_name)
void *str_uint_pair_load(FILE *f, char *file_name)
{
    struct str_uint_pair *p = (struct str_uint_pair *)
            calloc(1,sizeof(struct str_uint_pair));

    size_t fr = fread(&(p->uint), sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    uint32_t str_len;

    fr = fread(&str_len, sizeof(uint32_t), 1, f);
    check_file_read(file_name, f, 1, fr);

    p->str = (char *)calloc(str_len + 1, sizeof(char));

    fr = fread(p->str, sizeof(char), str_len, f);
    check_file_read(file_name, f, str_len, fr);

    return (void *)p;
}
//}}}

//{{{void str_uint_pair_free(void **p)
void str_uint_pair_free(void **_p)
{
    struct str_uint_pair **p = (struct str_uint_pair **)_p;
    if (*p != NULL) {
        free((*p)->str);
        free(*p);
        *p = NULL;
    }
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

//{{{ uint_offset_size_pair
//{{{ int uint_offset_size_pair_sort_element_cmp(const void *a, const void *b)
int uint_offset_size_pair_sort_element_cmp(const void *a, const void *b)
{
    struct uint_offset_size_pair **pa = (struct uint_offset_size_pair **)a;
    struct uint_offset_size_pair **pb = (struct uint_offset_size_pair **)b;

    return (*pa)->uint - (*pb)->uint;
}
//}}}

//{{{int uint_offset_size_pair_search_element_cmp(const void *a, const void *b)
int uint_offset_size_pair_search_element_cmp(const void *a, const void *b)
{
    struct uint_offset_size_pair *key = (struct uint_offset_size_pair *)a;
    struct uint_offset_size_pair **arg = (struct uint_offset_size_pair **)b;

    return key->uint - (*arg)->uint;
}
//}}}

//{{{int uint_offset_size_pair_search_key_cmp(const void *a, const void *b)
int uint_offset_size_pair_search_key_cmp(const void *a, const void *b)
{
    uint32_t *key = (uint32_t *)a;
    struct uint_offset_size_pair **arg = (struct uint_offset_size_pair **)b;

    return *key - (*arg)->uint;
}
//}}}
//}}}

//{{{ uint_pair
//{{{ int uint_pair_sort_by_first_element_cmp(const void *a, const void *b)
int uint_pair_sort_by_first_element_cmp(const void *a, const void *b)
{
    struct uint_pair **pa = (struct uint_pair **)a;
    struct uint_pair **pb = (struct uint_pair **)b;

    if ((*pa)->first < (*pb)->first) {
        return -1;
    } else if ((*pa)->first > (*pb)->first) {
        return 1;
    } else {
        return 0;
    }
}
//}}}

//{{{int uint_pair_search_by_first_element_cmp(const void *a, const void *b)
int uint_pair_search_by_first_element_cmp(const void *a, const void *b)
{
    struct uint_pair *key = (struct uint_pair *)a;
    struct uint_pair **arg = (struct uint_pair **)b;

    if (key->first < (*arg)->first) {
        return -1;
    } else if (key->first > (*arg)->first) {
        return 1;
    } else {
        return 0;
    }

}
//}}}

//{{{int uint_pair_search_by_first_key_cmp(const void *a, const void *b)
int uint_pair_search_by_first_key_cmp(const void *a, const void *b)
{
    uint32_t *key = (uint32_t *)a;
    struct uint_pair **arg = (struct uint_pair **)b;

    if (*key < (*arg)->first) {
        return -1;
    } else if (*key > (*arg)->first) {
        return 1;
    } else {
        return 0;
    }
}
//}}}
//}}}

//{{{ fifo_q
//{{{void fifo_q_push(struct fifo_q **q, void *val)
void fifo_q_push(struct fifo_q **q, void *val)
{
    int *v = (int *)val;
    struct fifo_q_element *n = (struct fifo_q_element *)
            malloc(sizeof(struct fifo_q_element));
    n->val = val;
    n->next = NULL;

    if (*q == NULL) {
        *q = (struct fifo_q *)malloc(sizeof(struct fifo_q));
        (*q)->head = n;
        (*q)->tail = n;
    } else {
        (*q)->tail->next = n;
        (*q)->tail = n;
    }
}
//}}}

//{{{void *fifo_q_pop(struct fifo_q **q)
void *fifo_q_pop(struct fifo_q **q)
{
    if (*q == NULL)
        return NULL;

    struct fifo_q_element *n = (*q)->head;
    void *r = n->val;
    (*q)->head = n->next;
    free(n);

    if ((*q)->head == NULL) {
        free(*q);
        *q = NULL;
    }

    return r;
}
//}}}

//{{{void *fifo_q_peek(struct fifo_q *q)
void *fifo_q_peek(struct fifo_q *q)
{
    if (q == NULL)
        return NULL;
    else
        return q->head->val;
}
//}}}
//}}}

//{{{ byte_array
//{{{ struct byte_array *byte_array_init(uint32_t init_size)
struct byte_array *byte_array_init(uint32_t init_size)
{
    struct byte_array *ba = (struct byte_array *)
            malloc(sizeof(struct byte_array));
    
    ba->data = (char *) malloc(init_size);
    ba->num = 0;
    ba->size = init_size;

    return ba;
}
//}}}

//{{{void byte_array_destory(struct byte_array **ba)
void byte_array_destory(struct byte_array **ba)
{
    free((*ba)->data);
    free(*ba);
    *ba = NULL;
}
//}}}

//{{{void byte_array_append(struct byte_array *ba, void *data, uint32_t size)
void byte_array_append(struct byte_array *ba, void *data, uint32_t size)
{
    while (ba->num + size >= ba->size) {
        ba->size = ba->size * 2;
        ba->data = realloc(ba->data, ba->size * sizeof(char *));
    }
    if (ba->num + size >= ba->size)
        errx(1, "Not enough room to append on byte array");

    void *r = memcpy(ba->data + ba->num, data, size);
    ba->num += size;
}
//}}}

//{{{void byte_array_append_zeros(struct byte_array *ba, uint32_t size)
void byte_array_append_zeros(struct byte_array *ba, uint32_t size)
{
    memset(ba->data + ba->num, 0, size);
    ba->num += size;
}
//}}}
//}}}
