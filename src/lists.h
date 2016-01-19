#ifndef __LISTS_H__
#define __LISTS_H__

#include <stdint.h>
#include "disk_store.h"

// BITMAP
struct bit_map
{
    uint32_t num_bits, num_ints;
    uint32_t *bm;
};

struct bit_map *bit_map_init(uint32_t bits);
struct bit_map *bit_map_load(FILE *f, char *file_name);
void bit_map_store(struct bit_map *b, FILE *f, char *file_name);
void bit_map_destroy(struct bit_map **b);
void bit_map_set(struct bit_map *b, uint32_t i);
uint32_t bit_map_get(struct bit_map *b, uint32_t q);

// INDEXED LIST
struct indexed_list
{
    char *data;
    uint32_t size, element_size;
    struct bit_map *bm;
};
struct indexed_list *indexed_list_init(uint32_t init_size,
                                       uint32_t element_size);
void indexed_list_destroy(struct indexed_list **il);
uint32_t indexed_list_add(struct indexed_list *il,
                          uint32_t index,
                          void *data);
void *indexed_list_get(struct indexed_list *il, uint32_t index);
void indexed_list_write(struct indexed_list *il, FILE *f, char *file_name);
struct indexed_list *indexed_list_load(FILE *f, char *file_name);

struct offset_size_pair
{
    long offset;
    uint32_t size;
};

// UNORDERD LIST
struct unordered_list
{
    void **data;
    uint32_t num, size;
};

struct unordered_list *unordered_list_init(uint32_t init_size);
void unordered_list_destroy(struct unordered_list **ul,
                            void (*free_data)(void **data));
uint32_t unordered_list_add(struct unordered_list *ul,
                            void *data);
void *unordered_list_get(struct unordered_list *ul, uint32_t i);

// ORDERD SET
struct ordered_set
{
    void **data;
    uint32_t num, size;
    int (*sort_element_cmp)(const void *a, const void *b);
    int (*search_element_cmp)(const void *a, const void *b);
    int (*search_key_cmp)(const void *a, const void *b);
};

struct ordered_set 
        *ordered_set_init(
                uint32_t init_size,
                int (*sort_element_cmp)(const void *a, const void *b),
                int (*search_element_cmp)(const void *a, const void *b),
                int (*search_key_cmp)(const void *a, const void *b));
void ordered_set_destroy(struct ordered_set **os,
                         void (*free_data)(void **data));
void *ordered_set_add(struct ordered_set *os,
                       void *data);
void *ordered_set_get(struct ordered_set *os, void *key);

struct str_uint_pair
{
    char *str;
    uint32_t uint;
};

int str_uint_pair_sort_element_cmp(const void *a, const void *b);
int str_uint_pair_search_element_cmp(const void *a, const void *b);
int str_uint_pair_search_key_cmp(const void *a, const void *b);
void str_uint_pair_free(void **v);

struct pointer_uint_pair
{
    void *pointer;
    uint32_t uint;
};
int pointer_uint_pair_sort_element_cmp(const void *a, const void *b);
int pointer_uint_pair_search_element_cmp(const void *a, const void *b);
int pointer_uint_pair_search_key_cmp(const void *a, const void *b);

struct uint_offset_size_pair
{
    uint32_t uint,size;
    long offset;
};
int uint_offset_size_pair_sort_element_cmp(const void *a, const void *b);
int uint_offset_size_pair_search_element_cmp(const void *a, const void *b);
int uint_offset_size_pair_search_key_cmp(const void *a, const void *b);

struct uint_pair
{
    uint32_t first,second;
};
int uint_pair_sort_by_first_element_cmp(const void *a, const void *b);
int uint_pair_search_by_first_element_cmp(const void *a, const void *b);
int uint_pair_search_by_first_key_cmp(const void *a, const void *b);


// FIFO Q
struct fifo_q_element
{
    void *val;
    struct fifo_q_element *next;
};

struct fifo_q
{
    struct fifo_q_element *head, *tail;
};

void fifo_q_push(struct fifo_q **q, void *val);
void *fifo_q_pop(struct fifo_q **q);
void *fifo_q_peek(struct fifo_q *q);

// BYTE ARRAY
struct byte_array
{
    char *data;
    uint32_t num, size;
};

struct byte_array *byte_array_init(uint32_t init_size);
void byte_array_destory(struct byte_array **ba);
void byte_array_append(struct byte_array *ba, void *data, uint32_t size);
void byte_array_append_zeros(struct byte_array *ba, uint32_t size);


// CACHES
struct cache_def {
    void *cache;
    void *(*init)(uint32_t size, FILE *fp);
    uint32_t (*seen)(void *cache);
    void (*add)(void *cache,
                uint32_t key,
                void *data,
                void (*free_value)(void **data));
    void *(*get)(void *cache, uint32_t key);
    void (*remove)(void *cache, uint32_t key);
    void (*destroy)(void **cache);
};

struct cc_hash
{
    uint32_t num, sizes, *keys[2];
    void **values[2];
    uint32_t (*hashes[2])(uint32_t x, uint32_t limit);
};

uint32_t hash_A(uint32_t x, uint32_t limit);
uint32_t hash_B(uint32_t x, uint32_t limit);

struct cc_hash *cc_hash_init(uint32_t size);
int cc_hash_add(struct cc_hash *hash, uint32_t key, void *value);
void *cc_hash_get(struct cc_hash *hash, uint32_t key);
void *cc_hash_remove(struct cc_hash *hash, uint32_t key);
void cc_hash_destroy(struct cc_hash **hash);

struct linked_list_node
{
    void *value; 
    uint32_t key;
    struct linked_list_node *prev, *next;
    void (*free_value)(void **data);
};

struct lru_cache
{
    struct cc_hash *hash_table;
    struct linked_list_node *head, *tail;
    uint32_t size, num, seen;
};

void *lru_cache_init(uint32_t init_size, FILE *fp);
uint32_t lru_cache_seen(void *lruc); 
void *lru_cache_get(void *cache, uint32_t key);
void lru_cache_remove(void *cache, uint32_t key);
void lru_cache_add(void *cache,
                   uint32_t key,
                   void *value,
                   void (*free_value)(void **data));
void lru_cache_destroy(void **lruc);

struct cache_def lru_cache_def;


struct value_free_value_pair
{
    void *value;
    void (*free_value)(void **value);
};

struct simple_cache
{
    struct indexed_list *il;
    uint32_t size, num, seen;
    struct disk_store *ds;
};

void *simple_cache_init(uint32_t init_size, FILE *fp);
uint32_t simple_cache_seen(void *cache); 
void *simple_cache_get(void *cache, uint32_t key);
void simple_cache_remove(void *cache, uint32_t key);
void simple_cache_add(void *cache,
                      uint32_t key,
                      void *value,
                      void (*free_value)(void **data));
void simple_cache_destroy(void **cache);

void free_wrapper(void **v);

#endif

