#define _GNU_SOURCE

#include <stdlib.h>
#include <err.h>
#include "fastlz_wrapper.h"

void* fastlz_wrapper_compress(void *data, uLong uncompressed_size, uLong *compressed_size) {

    *compressed_size = uncompressed_size * 1.05 + 1; // extra 1 in case of round down
    if (*compressed_size < MINIMUM_BUFFER_SIZE) {
        *compressed_size = MINIMUM_BUFFER_SIZE;
    }

    void *compressed_data = (void *) calloc(1, *compressed_size);
    if (compressed_data == NULL)
        err(1, "calloc error in fastlz_compress().");

    // Deflate
    *compressed_size = fastlz_compress_level(2, data, uncompressed_size, compressed_data);

    return compressed_data;
}

void* fastlz_wrapper_uncompress(void *compressed_data, uLong compressed_size, uLong uncompressed_size) {

    void *uncompressed_data = (void *) calloc(1, uncompressed_size);
    if (uncompressed_data == NULL)
        err(1, "calloc error in fastlz_uncompress().");

    // Inflate
    fastlz_decompress(compressed_data, compressed_size, uncompressed_data, uncompressed_size);

    return uncompressed_data;
}