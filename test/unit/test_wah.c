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
    uint8_t v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),v);
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));
    free(w);

    w = wah_init(8, 8);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("10000001"),v);
    TEST_ASSERT_EQUAL(0, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));
    v = WAH_I(w,8,1);
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),v);
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"), WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));
    free(w);

    w = wah_init(8, 15);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("10000010"),v);
    TEST_ASSERT_EQUAL(0, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(2, WAH_NUM_WORDS(v,8));
    v = WAH_I(w,8,1);
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"),v);
    TEST_ASSERT_EQUAL(bin_char_to_int("01000000"), WAH_VAL(v,8));
    free(w);

    w = wah_init(8, 17);
    TEST_ASSERT_EQUAL(2,WAH_LEN(w));
    v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("10000010"),v);
    TEST_ASSERT_EQUAL(0, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(2, WAH_NUM_WORDS(v,8));
    v = WAH_I(w,8,1);
    TEST_ASSERT_EQUAL(bin_char_to_int("00010000"),v);
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));
    free(w);

    w = wah_init(8, 1000);
    TEST_ASSERT_EQUAL(3,WAH_LEN(w));
    v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("11111111"),v);
    TEST_ASSERT_EQUAL(0, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(127, WAH_NUM_WORDS(v,8));
    v = WAH_I(w,8,1);
    TEST_ASSERT_EQUAL(bin_char_to_int("10001111"),v);
    TEST_ASSERT_EQUAL(0, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(15, WAH_NUM_WORDS(v,8));
    v = WAH_I(w,8,2);
    TEST_ASSERT_EQUAL(bin_char_to_int("00000010"),v);
    TEST_ASSERT_EQUAL(2, WAH_VAL(v,8));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));

    v = 14;
    WAH_I(w,8,0) = (uint8_t)v;
    v++;
    WAH_I(w,8,1) = (uint8_t)v;
    v++;
    WAH_I(w,8,2) = (uint8_t)v;
    uint8_t u = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(14, u);
    u = WAH_I(w,8,1);
    TEST_ASSERT_EQUAL(15, u);
    u = WAH_I(w,8,2);
    TEST_ASSERT_EQUAL(16, u);

    free(w);
}

void test_wah_or_8(void)
{
    /*
     * 150: 2 10010101 00010000
     *        149      16
     * 100: 2 10001110 00100000
     *        142      32
     *
     * 150 | 100
     *
     * 10001110 00100000 10000110 00010000
     * 142      32       134      16
     *
     */

    uint8_t *w = wah_init(8, 150);
    uint8_t *u = wah_init(8, 100);
    uint8_t *x = wah_init(8, 1);

    uint8_t *r = NULL;
    wah_or_8(w, u, &r);
    TEST_ASSERT_EQUAL(4,WAH_LEN(r));

    uint8_t v = WAH_I(r,8,0);
    TEST_ASSERT_EQUAL(142, v);
    v = WAH_I(r,8,1);
    TEST_ASSERT_EQUAL(32, v);
    v = WAH_I(r,8,2);
    TEST_ASSERT_EQUAL(134, v);
    v = WAH_I(r,8,3);
    TEST_ASSERT_EQUAL(16, v);

    uint8_t *r_2 = NULL;
    wah_or_8(r, x, &r_2);
    /*
     * 10001110 00100000 10000110 00010000
     * 142      32       134      16
     * 01000000
     * 64
     *
     * 01000000 10001101 00100000 10000110 00010000
     * 64       141      32       134      16
     */
    TEST_ASSERT_EQUAL(5,WAH_LEN(r_2));
    v = WAH_I(r_2,8,0);
    TEST_ASSERT_EQUAL(64, v);
    v = WAH_I(r_2,8,1);
    TEST_ASSERT_EQUAL(141, v);
    v = WAH_I(r_2,8,2);
    TEST_ASSERT_EQUAL(32, v);
    v = WAH_I(r_2,8,3);
    TEST_ASSERT_EQUAL(134, v);
    v = WAH_I(r_2,8,4);
    TEST_ASSERT_EQUAL(16, v);

    uint8_t *r_3 = NULL;
    wah_or_8(r_2, r_2, &r_3);
    TEST_ASSERT_EQUAL(5,WAH_LEN(r_3));
    v = WAH_I(r_3,8,0);
    TEST_ASSERT_EQUAL(64, v);
    v = WAH_I(r_3,8,1);
    TEST_ASSERT_EQUAL(141, v);
    v = WAH_I(r_3,8,2);
    TEST_ASSERT_EQUAL(32, v);
    v = WAH_I(r_3,8,3);
    TEST_ASSERT_EQUAL(134, v);
    v = WAH_I(r_3,8,4);
    TEST_ASSERT_EQUAL(16, v);


}
