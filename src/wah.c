#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include <err.h>


#include "wah.h"
#include "util.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

//{{{uint8_t *wah_init_8(uint32_t val)
uint8_t *wah_init_8(uint32_t val)
{
    uint32_t bits_per_word = 7;
    uint32_t num_words = (val + bits_per_word - 1) / bits_per_word;
    // the max number of words 8-bit fill word and represent is 
    // 2**7 - 1 = 127
    uint32_t len = 1 + (num_words > 1 ? (num_words + 127 - 1)/127 : 0);
    uint8_t *w = (uint8_t *)malloc(sizeof(uint32_t) + (len*sizeof(uint8_t)));

    WAH_LEN(w) = len;

    uint32_t i = 0;
    uint32_t saved_words;
    while (val > bits_per_word) {
        saved_words = MIN(num_words - 1, 127);
        WAH_I(w,8,i) = (1 << bits_per_word) + (saved_words);
        val -= saved_words * bits_per_word;
        num_words -= saved_words;
        i+=1;
    }

    if (val > 0)
        WAH_I(w,8,i) =  1 << ( bits_per_word - val);
    else
        WAH_I(w,8,i) =  0;

    return w;
}
//}}}

//{{{uint8_t *wah_init(uint32_t word_size,
uint8_t *wah_init(uint32_t word_size,
                  uint32_t val)
{
    if (word_size == 8)
        return wah_init_8(val);
    return NULL;
}
//}}}

uint8_t *wah_8_copy(uint8_t *w)
{
    if (w == NULL)
        return NULL;

    if (WAH_LEN(w) == 0)
        return NULL;

    uint32_t R_size = sizeof(uint32_t) + (WAH_LEN(w)*sizeof(uint8_t));
    uint8_t *R = (uint8_t *)malloc(R_size);
    memcpy(R, w, R_size);

    return R;
}

//{{{ uint32_t wah_or_8(uint8_t *X, uint8_t *Y, uint8_t **R, uint32_t *R_size)
uint32_t wah_or_8(uint8_t *X, uint8_t *Y, uint8_t **R, uint32_t *R_size)
{
    uint32_t R_i = 0, X_i = 0, Y_i = 0;
    uint8_t x, y;
    uint32_t x_size, y_size, r_size, y_done = 0, x_done = 0;
    uint32_t X_len = WAH_LEN(X), Y_len = WAH_LEN(Y);
    uint32_t R_len = X_len + Y_len;
    uint32_t reset_R = 0;

    if (*R == NULL) {
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)malloc(*R_size);
        memset(*R, 0, *R_size);
        reset_R = 1;
    } else if (*R_size < sizeof(uint32_t) + (R_len*sizeof(uint8_t))) {
        free(*R);
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)malloc(*R_size);
        memset(*R, 0, *R_size);
        reset_R = 1;
    }

    x = WAH_I(X, 8, X_i);
    y = WAH_I(Y, 8, Y_i);

    x_size = WAH_NUM_WORDS(x, 8);
    y_size = WAH_NUM_WORDS(y, 8);

    uint8_t v;
    while (1) {
        r_size = MIN(x_size, y_size);

        if (r_size > 1) 
            v = (uint8_t) ((1<< 7) + r_size);
        else
            v = (uint8_t) (WAH_VAL(x,8) | WAH_VAL(y, 8));

        // Grow R if we need to
        if (sizeof(uint32_t) + R_i*sizeof(uint8_t) == *R_size) {
            uint32_t old_len = R_len;
            reset_R = 1;
            R_len = R_len * 2;
            *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
            *R = (uint8_t *) realloc(*R, *R_size);
            memset(*R + sizeof(uint32_t) + (old_len * sizeof(uint8_t)),
                   0,
                   old_len * sizeof(uint8_t) );
        }

        WAH_I(*R, 8, R_i) = (uint8_t) v;
        R_i += 1;

        //fprintf(stderr, "%" PRIu8 "\n", v);

        x_size -= r_size;
        y_size -= r_size;

        if ((x_size == 0) && (x_done == 0)) {
            X_i += 1;
            if (X_i == X_len) {
                x_done = 1;
                x = 0;
            } else {
                x = WAH_I(X, 8, X_i);
                x_size = WAH_NUM_WORDS(x, 8);
            }
        }

        if ((y_size == 0) && (y_done == 0)) {
            Y_i += 1;
            if (Y_i == Y_len) {
                y_done = 1;
                y = 0;
            } else {
                y = WAH_I(Y, 8, Y_i);
                y_size = WAH_NUM_WORDS(y, 8);
            }
        }

        if ((x_done == 1) && (y_done == 1))
            break;
        else if (x_done == 1)
            x_size = y_size;
        else if (y_done == 1)
            y_size = x_size;
    }

    R_len = R_i;
    WAH_LEN(*R) = R_len;
    if (reset_R == 1) {
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)realloc(*R, *R_size);
    }

    return reset_R;
}
//}}}

//{{{ uint32_t wah_or_8(uint8_t *X, uint8_t *Y, uint8_t **R, uint32_t *R_size)
uint32_t wah_nand_8(uint8_t *X, uint8_t *Y, uint8_t **R, uint32_t *R_size)
{
    uint32_t R_i = 0, X_i = 0, Y_i = 0;
    uint8_t x, y;
    uint32_t x_size, y_size, r_size, y_done = 0, x_done = 0;
    uint32_t X_len = WAH_LEN(X), Y_len = WAH_LEN(Y);
    uint32_t R_len = X_len + Y_len;
    uint32_t reset_R = 0;

    if (*R == NULL) {
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)malloc(*R_size);
        memset(*R, 0, *R_size);
        reset_R = 1;
    } else if (*R_size < sizeof(uint32_t) + (R_len*sizeof(uint8_t))) {
        free(*R);
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)malloc(*R_size);
        memset(*R, 0, *R_size);
        reset_R = 1;
    }

    x = WAH_I(X, 8, X_i);
    y = WAH_I(Y, 8, Y_i);

    x_size = WAH_NUM_WORDS(x, 8);
    y_size = WAH_NUM_WORDS(y, 8);

    uint8_t v;
    while (1) {
        r_size = MIN(x_size, y_size);

        if (r_size > 1) 
            v = (uint8_t) ((1<< 7) + r_size);
        else
            v = (uint8_t) (WAH_VAL(x,8) & ~(WAH_VAL(y, 8)));

        // Grow R if we need to
        if (sizeof(uint32_t) + R_i*sizeof(uint8_t) == *R_size) {
            uint32_t old_len = R_len;
            reset_R = 1;
            R_len = R_len * 2;
            *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
            *R = (uint8_t *) realloc(*R, *R_size);
            memset(*R + sizeof(uint32_t) + (old_len * sizeof(uint8_t)),
                   0,
                   old_len * sizeof(uint8_t) );
        }

        WAH_I(*R, 8, R_i) = (uint8_t) v;
        R_i += 1;

        //fprintf(stderr, "%" PRIu8 "\n", v);

        x_size -= r_size;
        y_size -= r_size;

        if ((x_size == 0) && (x_done == 0)) {
            X_i += 1;
            if (X_i == X_len) {
                x_done = 1;
                x = 0;
            } else {
                x = WAH_I(X, 8, X_i);
                x_size = WAH_NUM_WORDS(x, 8);
            }
        }

        if ((y_size == 0) && (y_done == 0)) {
            Y_i += 1;
            if (Y_i == Y_len) {
                y_done = 1;
                y = 0;
            } else {
                y = WAH_I(Y, 8, Y_i);
                y_size = WAH_NUM_WORDS(y, 8);
            }
        }

        if ((x_done == 1) && (y_done == 1))
            break;
        else if (x_done == 1)
            x_size = y_size;
        else if (y_done == 1)
            y_size = x_size;
    }

    R_len = R_i;
    WAH_LEN(*R) = R_len;
    if (reset_R == 1) {
        *R_size = sizeof(uint32_t) + (R_len*sizeof(uint8_t));
        *R = (uint8_t *)realloc(*R, *R_size);
    }

    return reset_R;
}
//}}}

//{{{ uint32_t wah_get_ints_8(uint8_t *X, uint32_t **R)
uint32_t wah_get_ints_8(uint8_t *X, uint32_t **R)
{
    uint8_t x;
    uint32_t x_i_size, x_size = 0;
    uint32_t X_len = WAH_LEN(X);
    uint32_t R_len = 0;

    uint32_t i;
    for (i = 0; i < X_len; ++i) {
        x = WAH_I(X, 8, i);
        x_i_size = WAH_NUM_WORDS(x, 8);

        if (x_i_size == 1)
            R_len +=  __builtin_popcount(x);

        x_size += x_i_size * 7;
    }

    uint32_t diff = (sizeof(unsigned int)/sizeof(uint8_t) - 1)*8;
    uint32_t offset = 0;

    *R = (uint32_t*)calloc(R_len, sizeof(uint32_t));
    uint32_t R_i = 0;
    x_size = 0;
    for (i = 0; i < X_len; ++i) {
        x = WAH_I(X, 8, i);
        x_i_size = WAH_NUM_WORDS(x, 8);
        if ( x_i_size == 1 ) {
            while (x != 0) {
                offset = __builtin_clz(x) - diff;
                (*R)[R_i] = offset + x_size;
                R_i += 1;
                x &= ~(1 << (8-1-offset));
            }
        }
        x_size += x_i_size * 7;
    }
    return R_len;
}
//}}}

//{{{ uint32_t wah_get_ints_count_8(uint8_t *X)
uint32_t wah_get_ints_count_8(uint8_t *X)
{
    uint8_t x;
    uint32_t x_i_size;
    uint32_t X_len = WAH_LEN(X);
    uint32_t R_len = 0;

    uint32_t i;
    for (i = 0; i < X_len; ++i) {
        x = WAH_I(X, 8, i);
        x_i_size = WAH_NUM_WORDS(x, 8);

        if (x_i_size == 1)
            R_len +=  __builtin_popcount(x);
    }

    return R_len;
}
//}}}

//{{{void wah_8_uniq_append(uint8_t **w, uint32_t id)
void wah_8_uniq_append(uint8_t **w, uint32_t id)
{
    uint8_t *w_id = wah_init(8, id);

    if (*w == NULL) {
        *w = w_id;
    } else {
        uint8_t *r = NULL;
        uint32_t r_size = 0;
        uint32_t resize = wah_or_8(*w, w_id, &r, &r_size);
        free(*w);
        free(w_id);
        *w = r;
    }
}
//}}}

// NOTE:  this function could easily be rewritten using the general funtions
//{{{ void wah_8_leading_repair(uint32_t domain, 
void wah_8_leading_repair(uint32_t domain, 
                          struct bpt_node *a,
                          struct bpt_node *b)
{
#if DEBUG
    fprintf(stderr, "leading_repair\n");
#endif

    if ( (BPT_IS_LEAF(a) == 1) && (BPT_IS_LEAF(b) == 1) ) {
        // make a new leading value for the new right node
        struct wah_8_bpt_leading_data *d = 
            (struct wah_8_bpt_leading_data *)
            malloc(sizeof(struct wah_8_bpt_leading_data));

        d->B = NULL;

        // if the left node had leading data, grab all of it
        if (BPT_LEADING(a) != 0) {
            // get it from cache
            struct wah_8_bpt_leading_data *l =  
                    cache.get(domain,
                              BPT_LEADING(a) - 1,
                              &wah_8_leading_cache_handler);

            d->B = wah_8_copy(l->B);
        }

        uint32_t i;
        for (i = 0 ; i < BPT_NUM_KEYS(a); ++i) {
            struct wah_8_bpt_non_leading_data *nl = 
                    cache.get(domain,
                              BPT_POINTERS(a)[i] - 1,
                              &wah_8_non_leading_cache_handler);

            wah_8_non_leading_union_with_SA_subtract_SE(domain,
                                                        (void **)(&d->B),
                                                        nl);


        }

        if (d->B != NULL) {
            uint32_t v_id = cache.seen(domain) + 1;
            cache.add(domain, v_id - 1, d, &wah_8_leading_cache_handler);
            BPT_LEADING(b) = v_id;
        } else {
            free(d);
        }
    }
}
//}}}

//{{{ giggle_data_handler :: wah_8_giggle_data_handler
void wah_8_giggle_set_data_handler()
{
    bpt_node_repair = wah_8_leading_repair;

    wah_8_giggle_data_handler.non_leading_cache_handler =
        wah_8_non_leading_cache_handler;
    wah_8_giggle_data_handler.leading_cache_handler = 
        wah_8_leading_cache_handler;
    wah_8_giggle_data_handler.new_non_leading = 
        wah_8_new_non_leading;
    wah_8_giggle_data_handler.new_leading = 
        wah_8_new_leading;
    wah_8_giggle_data_handler.non_leading_SA_add_scalar = 
        wah_8_non_leading_SA_add_scalar;
    wah_8_giggle_data_handler.non_leading_SE_add_scalar = 
        wah_8_non_leading_SE_add_scalar;
    wah_8_giggle_data_handler.leading_B_add_scalar = 
        wah_8_leading_B_add_scalar;
    wah_8_giggle_data_handler.leading_union_with_B = 
        wah_8_leading_union_with_B;
    wah_8_giggle_data_handler.non_leading_union_with_SA = 
        wah_8_non_leading_union_with_SA;
    wah_8_giggle_data_handler.non_leading_union_with_SA_subtract_SE = 
        wah_8_non_leading_union_with_SA_subtract_SE;

    giggle_data_handler = wah_8_giggle_data_handler;
}

//{{{void *wah_8_new_non_leading()
void *wah_8_new_non_leading(uint32_t domain)
{
    struct wah_8_bpt_non_leading_data *d = 
            (struct wah_8_bpt_non_leading_data *)
            malloc(sizeof( struct wah_8_bpt_non_leading_data));
    d->SA = NULL;
    d->SE = NULL;

    return (void *)d;
}
//}}}

//{{{void *wah_8_new_leading()
void *wah_8_new_leading(uint32_t domain)
{
    struct wah_8_bpt_leading_data *d = 
            (struct wah_8_bpt_leading_data *)
            malloc(sizeof( struct wah_8_bpt_leading_data));
    d->B = NULL;

    return (void *)d;
}
//}}}

//{{{void wah_8_non_leading_SA_add_scalar(void *d, void *v)
void wah_8_non_leading_SA_add_scalar(uint32_t domain,
                                     void *_nld,
                                     void *_id)
{
#if DEBUG
    fprintf(stderr, "uint32_t_ll_non_leading_SA_add_scalar\n");
#endif
    struct wah_8_bpt_non_leading_data *nld =
            (struct wah_8_bpt_non_leading_data *)_nld;
    uint32_t *id = (uint32_t *)_id;

#if DEBUG
    fprintf(stderr, "id:%u\n", *id);
#endif

    wah_8_uniq_append(&(nld->SA), *id);
}
//}}}

//{{{void wah_8_non_leading_SE_add_scalar(void *d, void *v)
void wah_8_non_leading_SE_add_scalar(uint32_t domain,
                                     void *_nld,
                                     void *_id)
{
#if DEBUG
    fprintf(stderr, "uint32_t_ll_non_leading_SA_add_scalar\n");
#endif
    struct wah_8_bpt_non_leading_data *nld =
            (struct wah_8_bpt_non_leading_data *)_nld;
    uint32_t *id = (uint32_t *)_id;

#if DEBUG
    fprintf(stderr, "id:%u\n", *id);
#endif

    wah_8_uniq_append(&(nld->SE), *id);
}
//}}}

//{{{void wah_8_leading_B_add_scalar(void *d, void *v)
void wah_8_leading_B_add_scalar(uint32_t domain,
                                void *_ld,
                                void *_id)
{
#if DEBUG
    fprintf(stderr, "uint32_t_ll_non_leading_SA_add_scalar\n");
#endif
    struct wah_8_bpt_leading_data *ld =
            (struct wah_8_bpt_leading_data *)_ld;
    uint32_t *id = (uint32_t *)_id;

#if DEBUG
    fprintf(stderr, "id:%u\n", *id);
#endif

    wah_8_uniq_append(&(ld->B), *id);
}
//}}}

//{{{void wah_8_leading_union_with_B(void **R, void *leading)
void wah_8_leading_union_with_B(uint32_t domain,
                                void **R,
                                void *leading)
{
    struct wah_8_bpt_leading_data *ld = 
            (struct wah_8_bpt_leading_data *)leading;

    if ((ld != NULL) && (ld->B != NULL)) {
        uint8_t **w = (uint8_t **)R;

        if (*w == NULL)
            *w = wah_init_8(0);

        uint8_t *r = NULL;
        uint32_t r_size = 0;
        uint32_t resize = wah_or_8(*w, ld->B, &r, &r_size);

        free(*w);
        *w = r;
    }
}
//}}}

//{{{void wah_8_non_leading_union_with_SA(void **R, void *d)
void wah_8_non_leading_union_with_SA(uint32_t domain, void **R, void *d)
{
    struct wah_8_bpt_non_leading_data *nld = 
            (struct wah_8_bpt_non_leading_data *) d;
    if (nld != NULL) {
        if ((nld->SA != NULL)) {
            uint8_t **w = (uint8_t **)R;

            if (*w == NULL)
                *w = wah_init_8(0);

            uint8_t *r = NULL;
            uint32_t r_size = 0;
            uint32_t resize = wah_or_8(*w, nld->SA, &r, &r_size);

            free(*w);
            *w = r;
        }
    }
}
//}}}

//{{{void wah_8_non_leading_union_with_SA_subtract_SE(uint32_t domain,
void wah_8_non_leading_union_with_SA_subtract_SE(uint32_t domain,
                                                 void **R,
                                                 void *d)
{
    struct wah_8_bpt_non_leading_data *nld = 
            (struct wah_8_bpt_non_leading_data *) d;

    if (nld != NULL) {
        if ((nld->SA != NULL)) {
            uint8_t **w = (uint8_t **)R;

            if (*w == NULL)
                *w = wah_init_8(0);

            uint8_t *r = NULL;
            uint32_t r_size = 0;
            uint32_t resize = wah_or_8(*w, nld->SA, &r, &r_size);

            free(*w);
            *w = r;
        }
        if ((nld->SE != NULL)) {
            uint8_t **w = (uint8_t **)R;

            if (*w == NULL)
                *w = wah_init_8(0);

            uint8_t *r = NULL;
            uint32_t r_size = 0;
            uint32_t resize = wah_nand_8(*w, nld->SE, &r, &r_size);

            free(*w);
            *w = r;
        }
    }
}
//}}}
//}}}

//{{{ wah_8_non_leading_cache_handler
struct cache_handler wah_8_non_leading_cache_handler = {
        wah_8_non_leading_serialize,
        wah_8_non_leading_deserialize,
        wah_8_non_leading_free
};

//{{{uint64_t wah_8_non_leading_serialize(void *deserialized,
uint64_t wah_8_non_leading_serialize(void *deserialized,
                                     void **serialized)
{
    if (deserialized == NULL) {
        *serialized = NULL;
        return 0;
    }

    struct wah_8_bpt_non_leading_data *d =  
            (struct wah_8_bpt_non_leading_data *)deserialized;

    uint32_t SA_len = 0, SE_len = 0, serialized_len;

    if (d->SA != NULL)
        SA_len = sizeof(uint32_t) + WAH_LEN(d->SA)*sizeof(uint8_t);

    if (d->SE != NULL)
        SE_len = sizeof(uint32_t) + WAH_LEN(d->SE)*sizeof(uint8_t);

    serialized_len = 2*sizeof(uint32_t) + SA_len + SE_len;

    uint8_t *data = (uint8_t *)malloc(serialized_len);

    uint32_t *data_u = (uint32_t *)data;
    data_u[0] = SA_len;
    data_u[1] = SE_len;

    uint32_t data_i = 2*sizeof(uint32_t);


    if (d->SA != NULL) 
        memcpy(data + data_i, d->SA, SA_len);

    data_i += SA_len;


    if (d->SE != NULL) 
        memcpy(data + data_i, d->SE, SE_len);

    data_i += SE_len;

    if (data_i != serialized_len)
        errx(1,
             "Issue with wah_8_non_leading_serlize lengths. "
             "Expected:%u observed:%u.",
             serialized_len,
             data_i);

    *serialized = data;

    return serialized_len; 
}
//}}}

//{{{uint64_t wah_8_non_leading_deserialize(void *serialized,
uint64_t wah_8_non_leading_deserialize(void *serialized,
                                       uint64_t serialized_size,
                                       void **deserialized)
{
    if ((serialized_size == 0) || (serialized == NULL)) {
        *deserialized = NULL;
        return 0;
    }

    if (serialized_size < sizeof(uint32_t)*2)
        errx(1,
             "Malformed wah_8_non_leading serialized value. "
             "Too short");

    uint32_t *data_u32 = (uint32_t *)serialized;

    if ( 2*sizeof(uint32_t) + 
         (data_u32[0]+data_u32[1])*sizeof(uint8_t) != serialized_size)
        errx(1,
             "Malformed wah_8_non_leading serialized value. "
             "Incorrect serialized_size.");

    struct wah_8_bpt_non_leading_data *d = 
            (struct wah_8_bpt_non_leading_data *)
                    calloc(1, sizeof(struct wah_8_bpt_non_leading_data));
    d->SA = NULL;
    d->SE = NULL;

    uint8_t *data_8 = (uint8_t *)(data_u32 + 2);

    if (data_u32[0] > 0)
        d->SA = wah_8_copy(data_8);

    data_8 = data_8 + data_u32[0];

    if (data_u32[1] > 0)
        d->SE = wah_8_copy(data_8);


    *deserialized = d;

    return sizeof(struct wah_8_bpt_non_leading_data);
}
//}}}

//{{{void wah_8_non_leading_free(void **deserialized)
void wah_8_non_leading_free(void **deserialized)
{
    struct wah_8_bpt_non_leading_data **d = 
            (struct wah_8_bpt_non_leading_data **)deserialized;
    if ((*d)->SA != NULL)
        free((*d)->SA);
    if ((*d)->SE != NULL)
        free((*d)->SE);
    free(*d);
    *d = NULL;
}
//}}}
//}}}

//{{{ wah_8_leading_cache_handler 
struct cache_handler wah_8_leading_cache_handler = {
        wah_8_leading_serialize,
        wah_8_leading_deserialize,
        wah_8_leading_free
};


//{{{uint64_t wah_8_leading_serialize(void *deserialized,
uint64_t wah_8_leading_serialize(void *deserialized,
                                 void **serialized)
{
    if (deserialized == NULL) {
        *serialized = NULL;
        return 0;
    }

    struct wah_8_bpt_leading_data *d =  
            (struct wah_8_bpt_leading_data *)deserialized;

    uint32_t B_len = 0, serialized_len;

    if (d->B != NULL)
        B_len = sizeof(uint32_t) + WAH_LEN(d->B)*sizeof(uint8_t);

    serialized_len = sizeof(uint32_t) + B_len;

    uint8_t *data = (uint8_t *)malloc(serialized_len);

    uint32_t *data_u = (uint32_t *)data;
    data_u[0] = B_len;

    uint32_t data_i = sizeof(uint32_t);

    if (d->B != NULL) 
        memcpy(data + data_i, d->B, B_len);

    data_i += B_len;

    if (data_i != serialized_len)
        errx(1,
             "Issue with wah_8_leading_serlize lengths. "
             "Expected:%u observed:%u.",
             serialized_len,
             data_i);

    *serialized = data;

    return serialized_len; 
}
//}}}


//{{{uint64_t wah_8_leading_deserialize(void *serialized,
uint64_t wah_8_leading_deserialize(void *serialized,
                                   uint64_t serialized_size,
                                   void **deserialized)
{
    if ((serialized_size == 0) || (serialized == NULL)) {
        *deserialized = NULL;
        return 0;
    }

    if (serialized_size < sizeof(uint32_t))
        errx(1,
             "Malformed wah_8_leading serialized value. "
             "Too short");

    uint32_t *data_u32 = (uint32_t *)serialized;

    if ( sizeof(uint32_t) + 
         (data_u32[0])*sizeof(uint8_t) != serialized_size)
        errx(1,
             "Malformed wah_8_leading serialized value. "
             "Incorrect serialized_size.");

    struct wah_8_bpt_leading_data *d = 
            (struct wah_8_bpt_leading_data *)
                    calloc(1, sizeof(struct wah_8_bpt_non_leading_data));
    d->B = NULL;

    uint8_t *data_8 = (uint8_t *)(data_u32 + 1);

    if (data_u32[0] > 0)
        d->B = wah_8_copy(data_8);

    *deserialized = d;

    return sizeof(struct wah_8_bpt_leading_data);
}
//}}}

//{{{void wah_8_leading_free(void **deserialized)
void wah_8_leading_free(void **deserialized)
{
    struct wah_8_bpt_leading_data **d = 
            (struct wah_8_bpt_leading_data **)deserialized;
    if ( (*d)->B != NULL )
        free ((*d)->B);
    free(*d);
    *d = NULL;
}
//}}}
//}}}


