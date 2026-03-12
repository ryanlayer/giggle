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

//{{{void test_safe_subtract(void)
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

//{{{ test_parse_region
void test_parse_region_basic(void)
{
    char *chrm = NULL;
    uint32_t start, end;
    char region[] = "chr1:1000-2000";

    int result = parse_region(region, &chrm, &start, &end);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("chr1", chrm);
    TEST_ASSERT_EQUAL(1000, start);
    TEST_ASSERT_EQUAL(2000, end);

    free(chrm);
}

void test_parse_region_no_chr_prefix(void)
{
    char *chrm = NULL;
    uint32_t start, end;
    char region[] = "1:500-1500";

    int result = parse_region(region, &chrm, &start, &end);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("1", chrm);
    TEST_ASSERT_EQUAL(500, start);
    TEST_ASSERT_EQUAL(1500, end);

    free(chrm);
}

void test_parse_region_chrX(void)
{
    char *chrm = NULL;
    uint32_t start, end;
    char region[] = "chrX:100-200";

    int result = parse_region(region, &chrm, &start, &end);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("chrX", chrm);
    TEST_ASSERT_EQUAL(100, start);
    TEST_ASSERT_EQUAL(200, end);

    free(chrm);
}

void test_parse_region_large_coords(void)
{
    char *chrm = NULL;
    uint32_t start, end;
    char region[] = "chr1:1000000-2000000";

    int result = parse_region(region, &chrm, &start, &end);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("chr1", chrm);
    TEST_ASSERT_EQUAL(1000000, start);
    TEST_ASSERT_EQUAL(2000000, end);

    free(chrm);
}

void test_parse_region_zero_start(void)
{
    char *chrm = NULL;
    uint32_t start, end;
    char region[] = "chr1:0-100";

    int result = parse_region(region, &chrm, &start, &end);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("chr1", chrm);
    TEST_ASSERT_EQUAL(0, start);
    TEST_ASSERT_EQUAL(100, end);

    free(chrm);
}
//}}}

//{{{ test_safe_dirname_basename
void test_safe_dirname(void)
{
    char result[4096];

    safe_dirname("/path/to/file.txt", result);
    TEST_ASSERT_EQUAL_STRING("/path/to", result);

    safe_dirname("/path/to/", result);
    TEST_ASSERT_EQUAL_STRING("/path", result);

    safe_dirname("file.txt", result);
    TEST_ASSERT_EQUAL_STRING(".", result);

    safe_dirname("/", result);
    TEST_ASSERT_EQUAL_STRING("/", result);
}

void test_safe_basename(void)
{
    char result[4096];

    safe_basename("/path/to/file.txt", result);
    TEST_ASSERT_EQUAL_STRING("file.txt", result);

    safe_basename("/path/to/", result);
    TEST_ASSERT_EQUAL_STRING("to", result);

    safe_basename("file.txt", result);
    TEST_ASSERT_EQUAL_STRING("file.txt", result);
}
//}}}

//{{{ test_long_cmp
void test_long_cmp(void)
{
    long a = 10, b = 20, c = 10;

    // a < b should return negative
    TEST_ASSERT_TRUE(long_cmp(&a, &b) < 0);

    // b > a should return positive
    TEST_ASSERT_TRUE(long_cmp(&b, &a) > 0);

    // a == c should return 0
    TEST_ASSERT_EQUAL(0, long_cmp(&a, &c));
}
//}}}

