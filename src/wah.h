#ifndef __WAH_H__
#define __WAH_H__

#define WAH_LEN(W) ( ((uint32_t *)W)[0] )
#define WAH_I(W,S,I) ( (W + sizeof(uint32_t) + (S/8)*I)[0] )
#define WAH_VAL(W,S) ( (W >> (S-1)) == 1 ?  0 : W)
#define WAH_NUM_WORDS(W,S) ( (W >> (S-1)) == 1 ?  W & ~(1<< (S-1)) : 1)

uint8_t *wah_init(uint32_t word_size,
                  uint32_t val);

uint8_t *wah_init_8(uint32_t val);
void wah_or_8(uint8_t *X, uint8_t *Y, uint8_t **R);

uint32_t wah_get_ints(uint8_t *X, uint32_t **R);

#endif
