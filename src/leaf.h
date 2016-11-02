#ifndef __LEAF_H__
#define __LEAF_H__

#include "bpt.h"

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

struct leaf_data {
    uint32_t num_leading, num_starts, num_ends;
    uint32_t *leading, *starts, *ends, *data;
};

struct leaf_data_result {
    uint32_t len;
    uint32_t *data;
    struct leaf_data_result *next;
};

void leaf_data_free_mem(void **deserialized);
uint64_t leaf_data_deserialize(void *serialized,
                               uint64_t serialized_size,
                               void **deserialized);
uint64_t leaf_data_serialize(void *deserialized, void **serialized);

uint32_t leaf_data_get_starts_ends(struct bpt_node *node,
                                   struct leaf_data *data,
                                   uint32_t from,
                                   uint32_t to,
                                   uint32_t **starts,
                                   uint32_t *starts_size,
                                   uint32_t **ends,
                                   uint32_t *ends_size);
#endif
