#define _GNU_SOURCE

#include <stdlib.h>
#include <err.h>
#include "zlib_wrapper.h"

void* zlib_compress(void *data, uLong uncompressed_size, uLong *compressed_size) {

    *compressed_size = compressBound(uncompressed_size);

    void *compressed_data = (void *) calloc(1, *compressed_size);
    if (compressed_data == NULL)
        err(1, "calloc error in zlib_compress().");

    // Deflate
    compress((Bytef *)compressed_data, compressed_size, (Bytef *)data, uncompressed_size);

    return compressed_data;
}

void* zlib_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size) {

    void *uncompressed_data = (void *) calloc(1, uncompressed_size);
    if (uncompressed_data == NULL)
        err(1, "calloc error in zlib_uncompress().");

    // Inflate
    uncompress((Bytef *)uncompressed_data, &uncompressed_size, (Bytef *)compressed_data, compressed_size);

    return uncompressed_data;
}