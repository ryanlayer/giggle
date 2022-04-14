#ifndef __FASTLZ_WRAPPER_H__
#define __FASTLZ_WRAPPER_H__

#include "fastlz.h"
typedef unsigned long  uLong;
#define MINIMUM_BUFFER_SIZE 66

void* fastlz_wrapper_compress(void *data, uLong uncompressed_size, uLong *compressed_size);
void* fastlz_wrapper_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size);

#endif
