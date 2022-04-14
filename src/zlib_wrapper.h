#ifndef __ZLIB_WRAPPER_H__
#define __ZLIB_WRAPPER_H__

#include <zlib.h>

/**
 * @brief level must be between 0 and 9: 
 * 1 gives best speed, 9 gives best compression, 0 gives no compression at all.
 */
void* zlib_compress(void *data, uLong uncompressed_size, int level, uLong *compressed_size);
void* zlib_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size);

#endif
