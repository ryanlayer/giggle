#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <htslib/bgzf.h>
#include <htslib/tbx.h>
#include <htslib/kstring.h>

#include "unity.h"
#include "bpt.h"
#include "giggle_index.h"
#include "file_read.h"
#include "lists.h"
#include "util.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_init_file(void)
void test_safe_subtract(void)
{
    uint32_t a, b, r;

    a = 10;
    b = 5;

    r = safe_subtract(a, b);
    TEST_ASSERT_EQUAL(5, r);

    r = safe_subtract(b, a);
    TEST_ASSERT_EQUAL(0, r);

    r = safe_subtract(a, a);
    TEST_ASSERT_EQUAL(0, r);
}
//}}}

