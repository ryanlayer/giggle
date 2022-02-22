#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity/unity.h"
#include "zlib_wrapper.h"

void setUp(void) { }
void tearDown(void) { }

void test_zlib_wrapper(void)
{
    uint32_t x = 789;
    void *data = &x;
    uLong uncompressed_size = sizeof(x);
    uLong compressed_size;
    void *compressed_data = zlib_compress(data, uncompressed_size, &compressed_size);
    void *uncompressed_data = zlib_uncompress(compressed_data, compressed_size, uncompressed_size);
    uint32_t y = *((uint32_t *)uncompressed_data);
    TEST_ASSERT_EQUAL(x, y);
}