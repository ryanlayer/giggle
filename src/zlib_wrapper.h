#ifndef __ZLIB_WRAPPER_H__
#define __ZLIB_WRAPPER_H__

#include <zlib.h>

void* zlib_compress(void *data, uLong uncompressed_size, uLong *compressed_size);
void* zlib_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size);

#endif
