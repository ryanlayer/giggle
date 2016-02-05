#ifndef __WAH_H__
#define __WAH_H__

#define WAH_LEN(W) (((uint32_t *)W)[0])
#define WAH_VAL(W,S,I) ((W + sizeof(uint32_t) + (S/8)*I)[0])


uint8_t *wah_init(uint32_t word_size,
                  uint32_t val);

uint8_t *wah_init_8(uint32_t val);

#endif
