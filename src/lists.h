#ifndef __LISTS_H__
#define __LISTS_H__

struct unordered_list
{
    void **data;
    uint32_t num, size;
};

struct unordered_list *unordered_list_init(uint32_t init_size);
void unordered_list_destroy(struct unordered_list **ul);
uint32_t unordered_list_add(struct unordered_list *ul,
                            void *data);
void *unordered_list_get(struct unordered_list *ul, uint32_t i);

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
void ordered_set_destroy(struct ordered_set **os);
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

#endif
