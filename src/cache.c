#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>
#include <sysexits.h>
#include <string.h>
#include <unistd.h>

#include "cache.h"
#include "util.h"
#include "lists.h"

static void *_cache = NULL;
struct cache_def cache;


struct cache_handler uint32_t_cache_handler = {uint32_t_serialize, 
                                               uint32_t_deserialize,
                                               uint32_t_free_mem};

//{{{ uint32_t_cache_handler
//{{{uint64_t uint32_t_serialize(void *deserialized, void **serialized)
uint64_t uint32_t_serialize(void *deserialized, void **serialized)
{
    uint32_t *de = (uint32_t *)deserialized;

    //uint8_t *data = (uint8_t *)calloc(1, sizeof(uint32_t));
    uint8_t *data = (uint8_t *)calloc(sizeof(uint32_t), sizeof(uint8_t));

    memcpy(data, de, sizeof(uint32_t));

    *serialized = (void *)data;
    return sizeof(uint32_t);
}
//}}}

//{{{ uint64_t uint32_t_deserialize(void *serialized,
uint64_t uint32_t_deserialize(void *serialized,
                             uint64_t serialized_size,
                             void **deserialized)
{
    uint8_t *data = (uint8_t *)serialized;

    uint32_t *u = (uint32_t *)malloc(sizeof(uint32_t));

    memcpy(u, data, sizeof(uint32_t));

    *deserialized = (void *)u;

    return sizeof(uint32_t);
}
//}}}

//{{{void uint32_t_free_mem(void **deserialized)
void uint32_t_free_mem(void **deserialized)
{
    uint32_t **de = (uint32_t **)deserialized;
    free(*de);
    *de = NULL;
}
//}}}
//}}}

//{{{ simple_cache
struct cache_def simple_cache_def = {
    simple_cache_init,
    simple_cache_seen,
    simple_cache_add,
    simple_cache_get,
    simple_cache_store,
    NULL,
    simple_cache_destroy
};

//{{{ void *simple_cache_init(uint32_t size,
void *simple_cache_init(uint32_t size,
                        uint32_t num_domains,
                        char **file_names) 
{
    struct simple_cache *sc = (struct simple_cache *)
            malloc(sizeof(struct simple_cache));

    _cache = sc;
    cache = simple_cache_def;

    sc->num_domains = num_domains;

    sc->ils = (struct indexed_list **)calloc(num_domains,
                                             sizeof(struct indexed_list *));
    sc->nums = (uint32_t *)calloc(num_domains, sizeof(uint32_t));
    sc->seens = (uint32_t *)calloc(num_domains, sizeof(uint32_t));

    sc->dss = NULL;

    uint32_t i;
    // opend attached files
    if (file_names != NULL) {
        sc->dss = (struct disk_store **)calloc(num_domains, 
                                               sizeof(struct disk_store *));
        sc->index_file_names = (char **)calloc(num_domains, 
                                               sizeof(char *));
        sc->data_file_names = (char **)calloc(num_domains, 
                                              sizeof(char *));
        uint32_t ret;
        char *index_file_name, *data_file_name;
        for ( i = 0; i < num_domains; ++i) {
            ret = asprintf(&index_file_name, "%s.idx", file_names[i]);
            ret = asprintf(&data_file_name, "%s.dat", file_names[i]);
            sc->index_file_names[i] = index_file_name;
            sc->data_file_names[i] = data_file_name;
            // test to see if these files are in place
            if( (access(index_file_name, F_OK) != -1 ) && 
                (access(data_file_name, F_OK) != -1) )
                sc->dss[i] = disk_store_load(NULL,
                                             index_file_name,
                                             NULL,
                                             data_file_name);
            else 
                sc->dss[i] = NULL;
                /*
                sc->dss[i] = disk_store_init(size,
                                             NULL,
                                             index_file_name,
                                             NULL,
                                             data_file_name);
                */
        }
    }

    // set size
    sc->sizes = (uint32_t *)calloc(num_domains, sizeof(uint32_t));
    for ( i = 0; i < num_domains; ++i) {
        sc->sizes[i] = size;
        sc->nums[i] = 0;
        sc->seens[i] = 0;

        if ((sc->dss != NULL) && (sc->dss[i] != NULL)) {
            sc->nums[i] = sc->dss[i]->num;
            sc->seens[i] = sc->dss[i]->num;

            while (sc->sizes[i] < sc->dss[i]->num)
                sc->sizes[i] = sc->sizes[i] * 2;
        }

        sc->ils[i] = indexed_list_init(sc->sizes[i],
                                       sizeof(struct value_cache_handler_pair));
    }

    return sc;
}
//}}}

//{{{uint32_t simple_cache_seen(void *_sc)
uint32_t simple_cache_seen(uint32_t domain)
{
    if (_cache == NULL)
        errx(1, "Cache has not been initialized.");

    struct simple_cache *sc = (struct simple_cache *)_cache;
    return sc->seens[domain];
}
//}}}

//{{{void simple_cache_add(void *_sc,
void simple_cache_add(uint32_t domain,
                      uint32_t key,
                      void *value,
                      struct cache_handler *handler)
{
    if (_cache == NULL)
        errx(1, "Cache has not been initialized.");
    struct simple_cache *sc = (struct simple_cache *)_cache;

    struct value_cache_handler_pair vh;
    vh.value = value;
    vh.handler = handler;
    indexed_list_add(sc->ils[domain], key, &vh);
    sc->nums[domain] += 1;
    sc->seens[domain] += 1;
}
//}}}

//{{{void *simple_cache_get(void *_sc, uint32_t key)
void *simple_cache_get(uint32_t domain,
                       uint32_t key,
                       struct cache_handler *handler)
{
    if (_cache == NULL)
        errx(1, "Cache has not been initialized.");
    struct simple_cache *sc = (struct simple_cache *)_cache;
    struct value_cache_handler_pair *vh = indexed_list_get(sc->ils[domain],
                                                           key);

    if (vh == NULL) {
        if ((sc->dss != NULL) && (sc->dss[domain] != NULL)) {
            uint64_t size;
            void *raw = disk_store_get(sc->dss[domain], key, &size);
            if (raw == NULL)
                return NULL;

            void *v;
            uint64_t deserialized_size = handler->deserialize(raw,
                                                              size,
                                                              &v);


            simple_cache_add(domain, key, v, handler);
            return v;
        } else {
            return NULL;
        }
    } else 
        return vh->value;
}
//}}}

//{{{void simple_cache_destroy(void **_sc)
void simple_cache_destroy()
{
    if (_cache == NULL)
        errx(1, "Cache has not been initialized.");
    struct simple_cache *sc = (struct simple_cache *)_cache;

    uint32_t i,j;

    for (i = 0; i < sc->num_domains; ++i) {
        for (j = 0; j <= sc->seens[i]; ++j) {
            struct value_cache_handler_pair *vh =
                    indexed_list_get(sc->ils[i], j);
            if (vh != NULL) {
                if ((vh->handler != NULL) && (vh->handler->free_mem != NULL))
                    vh->handler->free_mem(&(vh->value));
            }
        }

        indexed_list_destroy(&(sc->ils[i]));
    }


    free(sc->ils);
    free(sc->nums);
    free(sc->seens);
    if ( sc->dss != NULL) {
        for (i = 0; i < sc->num_domains; ++i)
            if ( sc->dss[i] != NULL)
                disk_store_destroy(&(sc->dss[i]));
        free(sc->dss);
    }
    free(sc->sizes);
    free(sc);
    _cache = NULL;
}
//}}}

//{{{void simple_cache_store(uint32_t domain,
void simple_cache_store(uint32_t domain,
                        uint32_t *disk_id_order)
{
    if (_cache == NULL)
        errx(1, "Cache has not been initialized.");
    struct simple_cache *sc = (struct simple_cache *)_cache;

    if (sc->dss[domain] != NULL)
        errx(1, "Modifying and existing bpt is not currently supported.");

    fprintf(stderr, "%s %s\n", 
                                      sc->index_file_names[domain],
                                      sc->data_file_names[domain]);
    sc->dss[domain] = disk_store_init(sc->seens[domain],
                                      NULL,
                                      sc->index_file_names[domain],
                                      NULL,
                                      sc->data_file_names[domain]);

    struct value_cache_handler_pair *vh;
    uint32_t mem_i, disk_i, ds_id;
    uint64_t serialized_size;
    void *v;
    for (disk_i = 0 ; disk_i < sc->seens[domain]; ++disk_i) {

        if (disk_id_order != NULL)
            mem_i = disk_id_order[disk_i];
        else
            mem_i = disk_i;

        vh = indexed_list_get(sc->ils[domain], mem_i);
        if (vh == NULL)
            errx(1, "Value missing from cache.");

        if ((vh->handler == NULL) || (vh->handler->serialize == NULL))
            errx(1, "Cannot serialize given data without a valid handler.");

        serialized_size = vh->handler->serialize(vh->value,
                                                          &v);
        ds_id = disk_store_append(sc->dss[domain], v, serialized_size);

        if (disk_i != ds_id)
            errx(1, "Cache and disk are out of sync");
        free(v);
    }
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
