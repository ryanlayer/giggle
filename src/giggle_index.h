#ifndef __GIGGLE_H__
#define __GIGGLE_H__

#include <stdint.h>
#include <htslib/khash.h>
#include "bpt.h"
//#include "ll.h"
#include "cache.h"

#define PROGRAM_NAME  "giggle"
#define MAJOR_VERSION "0"
#define MINOR_VERSION "0"
#define REVISION_VERSION "1"
#define BUILD_VERSION "0"
#define VERSION MAJOR_VERSION "." MINOR_VERSION "." REVISION_VERSION

#define LEAF_DATA_STARTS_START(node, i) \
    ( i >= BPT_NUM_KEYS(node) \
      ? BPT_POINTERS(node)[BPT_NUM_KEYS(node)-1] >> 16 \
      : i == 0 ? 0 : BPT_POINTERS(node)[i-1] >> 16)

#define LEAF_DATA_STARTS_END(node, i) \
    ( i == BPT_NUM_KEYS(node) ? BPT_POINTERS(node)[i-1] >> 16 : \
        i == -1 ? 0 : BPT_POINTERS(node)[i] >> 16)

#define LEAF_DATA_ENDS_START(node, i) \
    (i >= BPT_NUM_KEYS(node) \
      ? BPT_POINTERS(node)[BPT_NUM_KEYS(node)-1] & 65535 \
      : i == 0 ? 0:BPT_POINTERS(node)[i-1] & 65535)

#define LEAF_DATA_ENDS_END(node, i) \
    ( i == BPT_NUM_KEYS(node) ? BPT_POINTERS(node)[i-1] & 65535 : \
    i == -1 ? 0 : BPT_POINTERS(node)[i] & 65535)

#define LEAF_DATA_LEADING_START(node) (0)
#define LEAF_DATA_LEADING_END(node) (node->num_leading)

struct file_id_offset_pair
{
    uint32_t file_id;
    long offset;
};
void *file_id_offset_pair_load(FILE *f, char *file_name);
void file_id_offset_pair_store(void *v, FILE *f, char *file_name);

struct file_id_offset_pairs
{
    uint64_t num,size;
    struct file_id_offset_pair *vals;
};

void c_str_store(void *v, FILE *f, char *file_name);
void *c_str_load(FILE *f, char *file_name);

struct giggle_index
{
    uint32_t *root_ids;
    uint32_t len, num;
    struct ordered_set *chrm_index;
    struct unordered_list *file_index;
    //struct unordered_list *offset_index;
    struct file_id_offset_pairs *offset_index;

    char *data_dir,
         *chrm_index_file_name,
         *file_index_file_name,
         *offset_index_file_name,
         *root_ids_file_name;
};

struct gigle_query_result
{
    struct giggle_index *gi;
    uint32_t num_files;
    struct long_ll **offsets;
};

struct gigle_query_result *giggle_query(struct giggle_index *gi,
                                        char *chrm,
                                        uint32_t start,
                                        uint32_t end,
                                        struct gigle_query_result *gqr);

void gigle_query_result_destroy(struct gigle_query_result **gqr);

struct giggle_query_iter
{
    struct giggle_index *gi;
    uint32_t file_id, curr, num;
    struct input_file *ipf;
    long *sorted_offsets;
};

uint32_t giggle_get_query_len(struct gigle_query_result *gqr,
                              uint32_t file_id);
                      
struct giggle_query_iter *giggle_get_query_itr(struct gigle_query_result *gqr,
                                               uint32_t file_id);

int giggle_query_next(struct giggle_query_iter *gqi,
                      char **result);

void giggle_iter_destroy(struct giggle_query_iter **gqi);

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
    void (*write_tree)(void *arg);
    void *(*giggle_collect_intersection)(uint32_t leaf_start_id,
                                         int pos_start_id,
                                         uint32_t leaf_end_id,
                                         int pos_end_id,
                                         uint32_t domain,
                                         void **r);
    void (*map_intersection_to_offset_list)(struct giggle_index *gi,
                                            struct gigle_query_result *gqr,
                                            void *_R);
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
struct giggle_index *giggle_init(uint32_t num_chrms,
                                 char *output_dir,
                                 uint32_t force,
                                 void (*giggle_set_data_handler)());
uint32_t giggle_store(struct giggle_index *gi);

struct giggle_index *giggle_load(char *data_dir,
                                 void (*giggle_set_data_handler)(void));

// LEAF DATA
struct leaf_data {
    uint32_t num_leading, num_starts, num_ends;
    uint32_t *leading, *starts, *ends, *data;
};

struct leaf_data_result {
    uint32_t len;
    uint32_t *data;
    struct leaf_data_result *next;
};

struct cache_handler leaf_data_cache_handler;
void leaf_data_free_mem(void **deserialized);
uint64_t leaf_data_deserialize(void *serialized,
                               uint64_t serialized_size,
                               void **deserialized);
uint64_t leaf_data_serialize(void *deserialized, void **serialized);

uint32_t giggle_get_leaf_data(struct giggle_index *gi,
                              uint32_t domain,
                              uint32_t leaf_id,
                              struct leaf_data **lf,
                              uint16_t **starts_ends_offsets);

struct cache_handler leaf_data_cache_handler;

uint32_t giggle_leaf_data_get_intersection_size(uint32_t leaf_start_id,
                                                int pos_start_id,
                                                uint32_t leaf_end_id,
                                                int pos_end_id,
                                                uint32_t domain);

uint32_t giggle_leaf_data_get_intersection(uint32_t leaf_start_id,
                                           int pos_start_id,
                                           uint32_t leaf_end_id,
                                           int pos_end_id,
                                           uint32_t domain);

void leaf_data_map_intersection_to_offset_list(struct giggle_index *gi,
                                               struct gigle_query_result *gqr,
                                               void *_R);

//void giggle_write_tree(struct giggle_index *gi);
void giggle_write_tree_cache_dump(void *giggle_index);
void giggle_write_tree_leaf_data(void *giggle_index);


void *giggle_collect_intersection_data_in_pointers(uint32_t leaf_start_id,
                                                   int pos_start_id,
                                                   uint32_t leaf_end_id,
                                                   int pos_end_id,
                                                   uint32_t domain,
                                                   void **r);

void *giggle_collect_intersection_data_in_block(uint32_t leaf_start_id,
                                                int pos_start_id,
                                                uint32_t leaf_end_id,
                                                int pos_end_id,
                                                uint32_t domain,
                                                void **r);

#endif
