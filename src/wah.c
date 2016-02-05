#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "wah.h"
#include "util.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

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
        WAH_VAL(w,8,i) = (1 << bits_per_word) + (saved_words);
        val -= saved_words * bits_per_word;
        num_words -= saved_words;
        i+=1;
    }

    WAH_VAL(w,8,i) =  1 << ( bits_per_word - val);
    

    return w;
}

uint8_t *wah_init(uint32_t word_size,
                  uint32_t val)
{
    if (word_size == 8)
        return wah_init_8(val);
    return NULL;
}

void wah_or_8(uint8_t *a, uint8_t *b)
{
}
