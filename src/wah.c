#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>


#include "wah.h"
#include "util.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

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

    WAH_I(w,8,i) =  1 << ( bits_per_word - val);
    

    return w;
}

uint8_t *wah_init(uint32_t word_size,
                  uint32_t val)
{
    if (word_size == 8)
        return wah_init_8(val);
    return NULL;
}

void wah_or_8(uint8_t *X, uint8_t *Y, uint8_t **R)
{
    uint32_t R_i = 0, X_i = 0, Y_i = 0;
    uint8_t x, y;
    uint32_t x_size, y_size, r_size, y_done = 0, x_done = 0;
    uint32_t X_len = WAH_LEN(X), Y_len = WAH_LEN(Y);
    uint32_t R_len = X_len + Y_len;

    *R = (uint8_t *)malloc(sizeof(uint32_t) + (R_len*sizeof(uint8_t)));
    memset(*R, 0, sizeof(uint32_t) + (R_len*sizeof(uint8_t)));

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
        if (R_i == R_len) {
            uint32_t old_len = R_len;
            R_len = R_len * 2;
            *R = (uint8_t *)
                    realloc(*R, sizeof(uint32_t) + (R_len*sizeof(uint8_t)));
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
    *R = (uint8_t *) realloc(*R, sizeof(uint32_t) + (R_len*sizeof(uint8_t)));
    WAH_LEN(*R) = R_len;
}
