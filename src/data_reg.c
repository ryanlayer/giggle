#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>

#include "data_reg.h"

//{{{struct data_reg *data_reg_init(uint32_t init_size)
struct data_reg *data_reg_init(uint32_t init_size)
{
    struct data_reg *dr = (struct data_reg *)malloc(sizeof(struct data_reg));
    dr->num_data = 0;
    dr->data_size = init_size;

    dr->data = (void **) malloc(init_size * sizeof(void *));

    return dr;
}
//}}}

//{{{void data_reg_destroy(struct data_reg **dr)
void data_reg_destroy(struct data_reg **dr)
{
    free((*dr)->data);
    (*dr)->data = NULL;

    free(*dr);
    *dr = NULL;
}
//}}}

//{{{uint32_t data_reg_add(struct data_reg *dr,
uint32_t data_reg_add(struct data_reg *dr,
                      void *data)
{
    uint32_t id = dr->num_data;

    dr->num_data = dr->num_data + 1;

    // check to see if there is enough space, if not grow by double
    if ( dr->data_size == dr->num_data ) {
        dr->data_size = dr->data_size * 2;
        dr->data = (void *)realloc(dr->data, 
                                   dr->data_size * sizeof(void *));
    }

    dr->data[id] = data;
    
    return id;
}
//}}}

//{{{void *data_reg_get(struct data_reg *dr, uint32_t i)
void *data_reg_get(struct data_reg *dr, uint32_t i)
{
    if (i > dr->num_data)
        return NULL;

    return dr->data[i];
}
//}}}
