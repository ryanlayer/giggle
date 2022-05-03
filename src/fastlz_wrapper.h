#ifndef __FASTLZ_WRAPPER_H__
#define __FASTLZ_WRAPPER_H__

#include "fastlz.h"
typedef unsigned long  uLong;
#define MINIMUM_BUFFER_SIZE 66

/**
 * @brief level must be 1 or 2: 
 * 1 is the fastest compression and generally useful for short data.
 * 2 is slightly slower but it gives better compression ratio.
 */
void* fastlz_wrapper_compress(void *data, uLong uncompressed_size, int level, uLong *compressed_size);
void* fastlz_wrapper_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size);

#endif
