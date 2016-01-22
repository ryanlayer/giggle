#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>
#include <sysexits.h>
#include <string.h>

#include "lists.h"
#include "util.h"

//{{{ bit_map

struct bit_map *bit_map_init(uint32_t bits)
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

    size_t fr = fread(&(b->num_bits), sizeof(uint32_t), 1, f);
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
    if (fwrite(&(b->num_bits), sizeof(uint32_t), 1, f) != 1)
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

void bit_map_set(struct bit_map *b, uint32_t i)
{
    while (i >= b->num_bits) {
        uint32_t new_bits = (b->num_bits)*2;
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

uint32_t bit_map_get(struct bit_map *b, uint32_t q)
{
    if (q > b->num_bits)
        return 0;

    return (( b->bm[q/32]) >> (31 - (q%32)) & 1);
}

//}}}

//{{{ indexed_list
//{{{ struct indexed_list *indexed_list_init(uint32_t init_size,
struct indexed_list *indexed_list_init(uint32_t init_size,
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
                          uint32_t index,
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
void *indexed_list_get(struct indexed_list *il, uint32_t index)
{
    if (bit_map_get(il->bm, index))
        return il->data + (index * il->element_size);
    else
        return NULL;
}
//}}}

//{{{void indexed_list_write(struct indexed_list *il, FILE *f, char *file_name)
void indexed_list_write(struct indexed_list *il, FILE *f, char *file_name)
{


    if (fwrite(&(il->size), sizeof(uint32_t), 1, f) != 1)
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

    size_t fr = fread(&(il->size), sizeof(uint32_t), 1, f);
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
    //os->data = (void **) malloc(init_size * sizeof(void *));
    os->data = (void **) calloc(init_size, sizeof(void *));
    os->sort_element_cmp = sort_element_cmp;
    os->search_element_cmp = search_element_cmp;
    os->search_key_cmp = search_key_cmp;

    return os;
}
//}}}

//{{{
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

//{{{ cc_hash

uint32_t hash_A(uint32_t x, uint32_t limit)
{
    x = x ^ (x>>4);
    x = (x^0xdeadbeef) + (x<<5);
    x = x ^ (x>>11);
    return x % limit;
}

uint32_t hash_B( uint32_t x, uint32_t limit)
{
    x = (x+0x7ed55d16) + (x<<12);
    x = (x^0xc761c23c) ^ (x>>19);
    x = (x+0x165667b1) + (x<<5);
    x = (x+0xd3a2646c) ^ (x<<9);
    x = (x+0xfd7046c5) + (x<<3);
    x = (x^0xb55a4f09) ^ (x>>16);
    return x % limit;
}

struct cc_hash *cc_hash_init(uint32_t size)
{
    struct cc_hash *hash = (struct cc_hash *)malloc(sizeof(struct cc_hash));
    hash->num = 0;
    hash->sizes = size / 2;
    hash->keys[0] = (uint32_t *) calloc(hash->sizes, sizeof(uint32_t));
    hash->keys[1] = (uint32_t *) calloc(hash->sizes, sizeof(uint32_t));
    hash->values[0] = (void **) calloc(hash->sizes, sizeof(void *));
    hash->values[1] = (void **) calloc(hash->sizes, sizeof(void *));
    
    hash->hashes[0] = hash_A;
    hash->hashes[1] = hash_B;

    return hash;
}

int cc_hash_add(struct cc_hash *hash, uint32_t key, void *value)
{
    uint32_t pos_0 = hash->hashes[0](key, hash->sizes);
    uint32_t pos_1 = hash->hashes[1](key, hash->sizes);

    if ( (hash->keys[0][pos_0] == key) || (hash->keys[1][pos_1] == key))
        return 1;

    uint32_t h_i = 0;
    uint32_t i, pos_i;

    for (i = 0; i < hash->sizes; ++i) {
        pos_i = hash->hashes[h_i](key, hash->sizes);
        if ( hash->values[h_i][pos_i] == NULL ) {
            hash->keys[h_i][pos_i] = key;
            hash->values[h_i][pos_i] = value;
            return 0;
        } else {
            uint32_t t_key = hash->keys[h_i][pos_i];
            void *t_value = hash->values[h_i][pos_i];
    
            hash->keys[h_i][pos_i] = key;
            hash->values[h_i][pos_i] = value;
    
            key = t_key;
            value = t_value;
            h_i = (h_i + 1) % 2;
        }
    }

    errx(1, "Could not place item\n");
}

void *cc_hash_get(struct cc_hash *hash, uint32_t key)
{
    uint32_t i, pos_i;
    for (i = 0; i < 2; ++i) {
        pos_i = hash->hashes[i](key, hash->sizes);
        if ((hash->keys[i][pos_i] == key) && (hash->values[i][pos_i] != NULL))
            return hash->values[i][pos_i];
    }

    return NULL;
}

void *cc_hash_remove(struct cc_hash *hash, uint32_t key)
{
    uint32_t i, pos_i;
    for (i = 0; i < 2; ++i) {
        pos_i = hash->hashes[i](key, hash->sizes);
        if ((hash->keys[i][pos_i] == key) && 
            (hash->values[i][pos_i] != NULL)) {
            void *r = hash->values[i][pos_i];
            hash->values[i][pos_i] = NULL;
            return r;
        }
    }

    return NULL;
}

void cc_hash_destroy(struct cc_hash **hash)
{
    free((*hash)->keys[0]);
    free((*hash)->keys[1]);
    free((*hash)->values[0]);
    free((*hash)->values[1]);
    free(*hash);
    *hash = NULL;
}
//}}}

//{{{ lru_cache

struct cache_def lru_cache_def = {
    NULL,
    lru_cache_init,
    lru_cache_seen,
    lru_cache_add,
    lru_cache_get,
    lru_cache_remove,
    lru_cache_destroy
};

//{{{struct lru_cache *lru_cache_init(uint32_t init_size)
void *lru_cache_init(uint32_t init_size, FILE *fp)
{
    struct lru_cache *lruc = (struct lru_cache *)
            malloc(sizeof(struct lru_cache));
    lruc->size = init_size;
    lruc->num = 0;
    lruc->seen = 0;
    lruc->hash_table = cc_hash_init(init_size * 2.5);
    lruc->head = NULL;
    lruc->tail = NULL;
    return lruc;
}
//}}}

//{{{uint32_t lru_cache_seen(struct lru_cache *lruc)
//uint32_t lru_cache_seen(struct lru_cache *lruc)
uint32_t lru_cache_seen(void *_lruc)
{
    struct lru_cache *lruc = (struct lru_cache *)_lruc;
    return lruc->seen;
}
//}}}

//{{{void lru_cache_add(struct lru_cache *lruc, uint32_t key, void *value)
void lru_cache_add(void *_lruc,
                   uint32_t key,
                   void *value,
                   void (*free_value)(void **data))
{
    struct lru_cache *lruc = (struct lru_cache *)_lruc;
    if (cc_hash_get(lruc->hash_table, key) != NULL)
        return;

    if (lruc->num == lruc->size) {
        // the head node is the lru
        struct linked_list_node *to_rem_l = lruc->head;
        lruc->head = to_rem_l->next;
        lruc->head->prev = NULL;

        struct linked_list_node *to_rem_h = 
                (struct linked_list_node *)
                cc_hash_remove(lruc->hash_table, to_rem_l->key);

        if (to_rem_h != to_rem_l)
            errx(1, "Inconsistency in LRU cache");

        if (to_rem_l->free_value != NULL)
            to_rem_l->free_value(&(to_rem_l->value));

        free(to_rem_l);
        lruc->num -= 1;
    }

    struct linked_list_node *ll = (struct linked_list_node *)
            malloc(sizeof(struct linked_list_node));
    ll->key = key;
    ll->free_value = free_value;
    ll->prev = NULL;
    ll->next = NULL;
    ll->value = value;

    if (lruc->head == NULL) {
        lruc->head = ll;
    } else {
        ll->prev = lruc->tail;
        lruc->tail->next = ll;
    }

    lruc->tail = ll;

    int r = cc_hash_add(lruc->hash_table, key, ll);

    lruc->num += 1;
    lruc->seen += 1;
}
//}}}

//{{{void *lru_cache_get(struct lru_cache *lruc, uint32_t key)
void *lru_cache_get(void *_lruc, uint32_t key)
{
    struct lru_cache *lruc = (struct lru_cache *)_lruc;
    struct linked_list_node *ll =
        (struct linked_list_node *) cc_hash_get(lruc->hash_table, key);

    if (ll == NULL)
        return NULL;

    // move this to the tail
    if (lruc->tail != ll) {

        // take ll out of the list
        if (lruc->head == ll) 
            lruc->head = ll->next;
        else {
            ll->next->prev = ll->prev;
            ll->prev->next = ll->next;
        }

        ll->prev = lruc->tail;
        lruc->tail->next = ll;
        lruc->tail = ll;
        lruc->tail->next = NULL;
    }
        
    return ll->value;
}
//}}}

//{{{void lru_cache_remove(struct lru_cache *lruc, uint32_t key)
void lru_cache_remove(void *_lruc, uint32_t key)
{
    struct lru_cache *lruc = (struct lru_cache *)_lruc;
    struct linked_list_node *to_rem = 
                (struct linked_list_node *)
                cc_hash_remove(lruc->hash_table, key);

    if (to_rem == NULL)
        return;

    // Take it out of the list
    if (to_rem == lruc->head) {
        lruc->head = NULL;
        lruc->tail = NULL;
    } else if (to_rem == lruc->tail) {
        lruc->tail->prev->next = NULL;
        lruc->tail = lruc->tail->prev;
    } else {
        to_rem->prev->next = to_rem->next;
    }


    if (to_rem->free_value != NULL)
        to_rem->free_value(&(to_rem->value));

    free(to_rem);
    lruc->num -= 1;
}
//}}}

//{{{void lru_cache_destroy(struct lru_cache **lruc)
void lru_cache_destroy(void **_lruc)
{
    struct lru_cache **lruc = (struct lru_cache **)_lruc;
    cc_hash_destroy(&((*lruc)->hash_table));

    struct linked_list_node *curr, *tmp;
    curr = (*lruc)->head;

    while (curr != NULL) {
        tmp = curr->next;;
        /*
        if ( (*lruc)->free_value != NULL)
            (*lruc)->free_value(&(curr->value));
        */
        if (curr->free_value != NULL)
            curr->free_value(&(curr->value));
        free(curr);
        curr = tmp;
    }
    free(*lruc);
    *lruc = NULL;
}
//}}}
//}}}

//{{{ simple_cache
//{{{void *simple_cache_init(uint32_t init_size, FILE *fp)
void *simple_cache_init(uint32_t init_size, FILE *fp)
{
    struct simple_cache *sc = (struct simple_cache *)
            malloc(sizeof(struct simple_cache));

    if (fp != NULL) {
        sc->ds = disk_store_load(&fp, "NULL");
    } else {
        sc->ds = NULL;
    }

    if (sc->ds != NULL) {
        while (init_size < sc->ds->num)
            init_size = init_size * 2;
    }

    sc->size = init_size;

    if (sc->ds != NULL) {
        sc->num = sc->ds->num;
        sc->seen = sc->ds->num;
    } else {
        sc->num = 0;
        sc->seen = 0;
    }

    sc->il = indexed_list_init(init_size, sizeof(struct value_free_value_pair));

    return sc;
}
//}}}

//{{{uint32_t simple_cache_seen(void *_sc)
uint32_t simple_cache_seen(void *_sc)
{
    struct simple_cache *sc = (struct simple_cache *)_sc;
    return sc->seen;
}
//}}}

//{{{void simple_cache_add(void *_sc,
void simple_cache_add(void *_sc,
                      uint32_t key,
                      void *value,
                      void (*free_value)(void **data))
{
    struct simple_cache *sc = (struct simple_cache *)_sc;

    struct value_free_value_pair vf;
    vf.value = value;
    vf.free_value = free_value;
    indexed_list_add(sc->il, key, &vf);
    sc->num += 1;
    sc->seen += 1;
}
//}}}

//{{{void *simple_cache_get(void *_sc, uint32_t key)
void *simple_cache_get(void *_sc, uint32_t key)
{
    struct simple_cache *sc = (struct simple_cache *)_sc;
    struct value_free_value_pair *vf = indexed_list_get(sc->il, key);

    if (vf == NULL) {
        if (sc->ds != NULL) {
            uint64_t size;
            void *v = disk_store_get(sc->ds, key, &size);
            if (v == NULL)
                return NULL;
        
            simple_cache_add(_sc, key, v, NULL);
            return v;
        } else {
            return NULL;
        }
    } else 
        return vf->value;
}
//}}}

//{{{void simple_cache_destroy(void **_sc)
void simple_cache_destroy(void **_sc)
{
    struct simple_cache **sc = (struct simple_cache **)_sc;

    uint32_t i;
    for (i = 0; i <= (*sc)->seen; ++i) {
        struct value_free_value_pair *vf = indexed_list_get((*sc)->il, i);
        if (vf != NULL) {
            if (vf->free_value != NULL)
                vf->free_value(&(vf->value));
        }
    }

    indexed_list_destroy(&((*sc)->il));
    free(*sc);
    *sc = NULL;
}
//}}}
//}}}

//{{{void free_wrapper(void **v)
void free_wrapper(void **v)
{
    free(*v);
    *v = NULL;
}
//}}}
