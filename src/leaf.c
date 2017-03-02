#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "leaf.h"

//{{{uint64_t leaf_data_serialize(void *deserialized, void **serialized)
uint64_t leaf_data_serialize(void *deserialized, void **serialized)
{
#if 0
    struct leaf_data *de = (struct leaf_data *)deserialized;

    uint32_t *data = (uint32_t *)malloc(
            3*sizeof(uint32_t) +
            ((de->num_leading + de->num_starts + de->num_ends)
             * sizeof(uint32_t))*2);
    
    data[0] = de->num_leading;
    data[1] = de->num_starts;
    data[2] = de->num_ends;

    uint8_t *output = (uint8_t *)(data + 3);
    int cs = fastlz_compress(de->data,
                             (de->num_leading + 
                             de->num_starts + 
                             de->num_ends) * sizeof(uint32_t),
                             output);
    //realloc(data, 3*sizeof(uint32_t) + cs*sizeof(int));
    *serialized = (void *)data;
    return 3*sizeof(uint32_t) + cs*sizeof(int);

#endif
#if 1
    struct leaf_data *de = (struct leaf_data *)deserialized;
    uint64_t *data = (uint64_t *)calloc((3 +
                                        de->num_leading +
                                        de->num_starts +
                                        de->num_ends),
                                        sizeof(uint64_t));
    if (data == NULL)
        err(1, "calloc error in leaf_data_serialize.\n");

    data[0] = de->num_leading;
    data[1] = de->num_starts;
    data[2] = de->num_ends;
    memcpy(data + 3,
           de->data,
           (de->num_leading + 
            de->num_starts + 
            de->num_ends)*sizeof(uint64_t));

    *serialized = (void *)data;
    return ((3 + de->num_leading + de->num_starts + de->num_ends) *
        sizeof(uint64_t));
#endif
}
//}}}

//{{{ uint64_t leaf_data_deserialize(void *serialized,
uint64_t leaf_data_deserialize(void *serialized,
                               uint64_t serialized_size,
                               void **deserialized)
{
    uint64_t *data = (uint64_t *)serialized;
    
    struct leaf_data *lf = (struct leaf_data *) 
            calloc(1, sizeof(struct leaf_data));
    if (lf == NULL)
        err(1, "calloc error in leaf_data_deserialize.\n");

    lf->num_leading = data[0];
    lf->num_starts = data[1];
    lf->num_ends = data[2];
    lf->data = (uint64_t *)calloc(lf->num_leading +
                                    lf->num_starts +
                                    lf->num_ends,
                                  sizeof(uint64_t));
    if (lf->data == NULL)
        err(1, "calloc error in leaf_data_deserialize.\n");

    lf->leading = lf->data;
    lf->starts = lf->data + lf->num_leading;
    lf->ends = lf->data + lf->num_leading + lf->num_starts;
    memcpy(lf->data,
           data + 3,
           (lf->num_leading + lf->num_starts + lf->num_ends)*sizeof(uint64_t));

    *deserialized = (void *)lf;

    return sizeof(struct leaf_data);
}
//}}}

//{{{void leaf_data_free_mem(void **deserialized)
void leaf_data_free_mem(void **deserialized)
{
    struct leaf_data **de = (struct leaf_data **)deserialized;
    free((*de)->data);
    free(*de);
    *de = NULL;
}
//}}}

//{{{uint32_t leaf_data_get_starts_ends(struct bpt_node *node,
uint32_t leaf_data_get_starts_ends(struct bpt_node *node,
                                   struct leaf_data *data,
                                   uint32_t from,
                                   uint32_t to,
                                   uint64_t **starts,
                                   uint64_t *starts_size,
                                   uint64_t **ends,
                                   uint64_t *ends_size)
{
    uint32_t total_size = 0;
    if (starts != NULL) {
        *starts_size = LEAF_DATA_STARTS_END(node, to) -
                LEAF_DATA_STARTS_START(node, from);
        if (*starts_size > 0) {
            *starts = (uint64_t *) malloc(*starts_size * sizeof(uint64_t));
            if (starts == NULL)
                err(1, "calloc error in leaf_data_get_starts_ends().\n");

            memcpy(*starts,
                   data->starts + 
                        LEAF_DATA_STARTS_START(node, from),
                    *starts_size * sizeof(uint64_t));
            total_size += *starts_size;
        }
    }

    if (ends != NULL) {
        *ends_size = LEAF_DATA_ENDS_END(node, to) -
                LEAF_DATA_ENDS_START(node, from);
        if (*ends_size > 0) {
            *ends = (uint64_t *) malloc(*ends_size * sizeof(uint64_t));
            if (ends == NULL)
                err(1, "calloc error in leaf_data_get_starts_ends().\n");

            memcpy(*ends,
                   data->ends + 
                        LEAF_DATA_ENDS_START(node, from),
                    *ends_size * sizeof(uint64_t));
            total_size += *ends_size;
        }
    }

    return total_size;
}
//}}}
