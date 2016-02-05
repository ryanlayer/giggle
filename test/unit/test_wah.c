#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <inttypes.h>


#include "wah.h"
#include "unity.h"

void setUp(void) { }
void tearDown(void) { }

uint32_t bin_char_to_int(char *bin)
{
    uint32_t i = 0;
    int j = 0;

    while (bin[j] != '\0') {
        i = i << 1;
        if (bin[j] == '1')
            i += 1;
        j+=1;
    }

    return i;
}

void test_wah_init(void)
{
    uint8_t *w = wah_init(8, 1);
    TEST_ASSERT_EQUAL(1,WAH_LEN(w));
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),WAH_VAL(w,8,0));
    free(w);

    w = wah_init(8, 8);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    TEST_ASSERT_EQUAL(bin_char_to_int("10000001"),WAH_VAL(w,8,0));
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),WAH_VAL(w,8,1));
    free(w);

    w = wah_init(8, 15);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    TEST_ASSERT_EQUAL(bin_char_to_int("10000010"),WAH_VAL(w,8,0));
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),WAH_VAL(w,8,1));
    free(w);

    w = wah_init(8, 17);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    TEST_ASSERT_EQUAL(bin_char_to_int("10000010"),WAH_VAL(w,8,0));
    TEST_ASSERT_EQUAL(bin_char_to_int("00010000"),WAH_VAL(w,8,1));
    free(w);

    w = wah_init(8, 1000);
    TEST_ASSERT_EQUAL(3,WAH_LEN(w));
    TEST_ASSERT_EQUAL(bin_char_to_int("11111111"),WAH_VAL(w,8,0));
    TEST_ASSERT_EQUAL(bin_char_to_int("10001111"),WAH_VAL(w,8,1));
    TEST_ASSERT_EQUAL(bin_char_to_int("00000010"),WAH_VAL(w,8,2));

    free(w);
}
