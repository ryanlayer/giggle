#ifndef __GIGGLE_H__
#define __GIGGLE_H__

#include <stdint.h>
#include <htslib/khash.h>
#include "bpt.h"
#include "ll.h"
#include "cache.h"

struct file_id_offset_pair
{
    uint32_t file_id;
    long offset;
};

struct giggle_index
{
    uint32_t *root_ids;
    uint32_t len, num;
    struct ordered_set *chrm_index;
    struct unordered_list *file_index;
    struct unordered_list *offset_index;
};

uint32_t giggle_insert(uint32_t domain,
                       uint32_t *root_id,
                       uint32_t start,
                       uint32_t end,
                       uint32_t id);

void *giggle_search(uint32_t domain,
                    uint32_t root_id,
                    uint32_t start,
                    uint32_t end);

struct giggle_def 
{
    struct cache_handler non_leading_cache_handler;
    struct cache_handler leading_cache_handler;
    void *(*new_non_leading)(uint32_t domain);
    void *(*new_leading)(uint32_t domain);
    void (*non_leading_SA_add_scalar)(uint32_t domain,
                                      void *non_leading,
                                      void *scalar);
    void (*non_leading_SE_add_scalar)(uint32_t domain, 
                                      void *non_leading,
                                      void *scalar);
    void (*leading_B_add_scalar)(uint32_t domain,
                                 void *leading,
                                 void *scalar);
    void (*leading_union_with_B)(uint32_t domain,
                                 void **result,
                                 void *leading);
    void (*non_leading_union_with_SA)(uint32_t domain,
                                      void **result,
                                      void *non_leading);
    void (*non_leading_union_with_SA_subtract_SE)(uint32_t domain,
                                                  void **result,
                                                  void *non_leading);
};

struct giggle_def giggle_data_handler;


struct giggle_index *giggle_init_index(uint32_t init_size);
void giggle_index_destroy(struct giggle_index **gi);
uint32_t giggle_get_chrm_id(struct giggle_index *gi, char *chrm);
uint32_t giggle_get_file_id(struct giggle_index *gi, char *path);
uint32_t giggle_index_file(struct giggle_index *gi,
                           char *file_name); 
uint32_t giggle_index_directory(struct giggle_index *gi,
                                char *path_name,
                                int verbose);
void *giggle_query_region(struct giggle_index *gi,
                          char *chrm,
                          uint32_t start,
                          uint32_t end);
#endif
