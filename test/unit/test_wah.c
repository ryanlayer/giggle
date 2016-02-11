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

//{{{ uint32_t bin_char_to_int(char *bin)
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
//}}}

//{{{ void test_wah_init(void)
void test_wah_init(void)
{
    uint8_t *w = wah_init(8, 0);
    TEST_ASSERT_EQUAL(1,WAH_LEN(w));
    uint8_t v = WAH_I(w,8,0);
    TEST_ASSERT_EQUAL(bin_char_to_int("00000000"),v);
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(v,8));
    free(w);

    w = wah_init(8, 1);
    TEST_ASSERT_EQUAL(1,WAH_LEN(w));
    v = WAH_I(w,8,0);
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
//}}}

//{{{void test_wah_8_copy(void)
void test_wah_8_copy(void)
{
    uint8_t *o = NULL;
    wah_8_uniq_append(&o, rand());
    wah_8_uniq_append(&o, rand());
    wah_8_uniq_append(&o, rand());
    wah_8_uniq_append(&o, rand());

    uint8_t *c = wah_8_copy(o);
    TEST_ASSERT_EQUAL(WAH_LEN(o), WAH_LEN(c));

    uint32_t *R_o = NULL, *R_c = NULL;
    TEST_ASSERT_EQUAL(4, wah_get_ints_8(o, &R_o));
    TEST_ASSERT_EQUAL(4, wah_get_ints_8(c, &R_c));

    TEST_ASSERT_EQUAL(R_o[0], R_c[0]);
    TEST_ASSERT_EQUAL(R_o[1], R_c[1]);
    TEST_ASSERT_EQUAL(R_o[2], R_c[2]);
    TEST_ASSERT_EQUAL(R_o[3], R_c[3]);

    free(R_o);
    free(R_c);
    free(o);
    free(c);
}
//}}}

//{{{ void test_wah_or_8(void)
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
    uint32_t r_size = 0;
    uint32_t resize = wah_or_8(w, u, &r, &r_size);
    TEST_ASSERT_EQUAL(1,resize);
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
    resize = wah_or_8(r, x, &r_2, &r_size);
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
    resize = wah_or_8(r_2, r_2, &r_3, &r_size);
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

    free(w);
    free(u);
    free(x);
    free(r);
    free(r_2);
    free(r_3);
}
//}}}

//{{{ void test_wah_or_8(void)
void test_wah_nand_8(void)
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
    uint8_t *y = wah_init(8, 3);
    uint8_t *z = wah_init(8, 6);

    uint8_t *r = NULL;
    uint32_t r_size = 0;
    uint32_t resize = wah_or_8(w, u, &r, &r_size);
    uint8_t *r_2 = NULL;
    resize = wah_or_8(r, x, &r_2, &r_size);
    uint8_t *r_3 = NULL;
    resize = wah_or_8(r_2, r_2, &r_3, &r_size);
    resize = wah_or_8(r_3, y, &r_2, &r_size);
    resize = wah_or_8(r_2, z, &r_3, &r_size);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(5, wah_get_ints_8(r_3, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(3, R[1]);
    TEST_ASSERT_EQUAL(6, R[2]);
    TEST_ASSERT_EQUAL(100, R[3]);
    TEST_ASSERT_EQUAL(150, R[4]);

    free(R);
    R = NULL;

    resize = wah_nand_8(r_3, y, &r_2, &r_size);

    TEST_ASSERT_EQUAL(4, wah_get_ints_8(r_2, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(100, R[2]);
    TEST_ASSERT_EQUAL(150, R[3]);

    free(R);
    R = NULL;

    uint8_t *r_4 = NULL;
    resize = wah_or_8(x, w, &r_4, &r_size);

    resize = wah_nand_8(r_2, r_4, &r_3, &r_size);
    TEST_ASSERT_EQUAL(2, wah_get_ints_8(r_3, &R));
    TEST_ASSERT_EQUAL(6, R[0]);
    TEST_ASSERT_EQUAL(100, R[1]);

    free(w);
    free(u);
    free(x);
    free(y);
    free(z);
    free(r);
    free(r_2);
    free(r_3);
    free(R);
    free(r_4);
}
//}}}

//{{{void test_wah_get_ints_8(void)
void test_wah_get_ints_8(void)
{
    //150: 2 10010101 00010000
    //       149      16
    uint8_t *w_150 = wah_init(8, 150);
    // 100: 2 10001110 00100000
    //        142      32
    uint8_t *w_100 = wah_init(8, 100);
    //   1: 1 01000000
    //        64
    uint8_t *w_1 = wah_init(8, 1);
    //   1: 1 00010000
    //        16
    uint8_t *w_3 = wah_init(8, 3);

    uint8_t *w_8 = wah_init(8, 8);
    uint8_t *w_10 = wah_init(8, 10);


    uint32_t *R = NULL; 
    uint32_t r_size = 0;
    TEST_ASSERT_EQUAL(1, wah_get_ints_8(w_150, &R));
    TEST_ASSERT_EQUAL(150, R[0]);
    free(R);
    R = NULL;

    TEST_ASSERT_EQUAL(1, wah_get_ints_8(w_100, &R));
    TEST_ASSERT_EQUAL(100, R[0]);
    free(R);
    R = NULL;

    TEST_ASSERT_EQUAL(1, wah_get_ints_8(w_1, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    free(R);
    R = NULL;

    TEST_ASSERT_EQUAL(1, wah_get_ints_8(w_3, &R));
    TEST_ASSERT_EQUAL(3, R[0]);
    free(R);
    R = NULL;

    uint8_t *r_100_150 = NULL;
    uint32_t resize = wah_or_8(w_150, w_100, &r_100_150, &r_size);
    TEST_ASSERT_EQUAL(2, wah_get_ints_8(r_100_150, &R));
    TEST_ASSERT_EQUAL(100, R[0]);
    TEST_ASSERT_EQUAL(150, R[1]);
    free(R);
    R = NULL;

    uint8_t *r_1_100_150 = NULL;
    resize = wah_or_8(r_100_150, w_1, &r_1_100_150, &r_size);
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(r_1_100_150, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(100, R[1]);
    TEST_ASSERT_EQUAL(150, R[2]);
    free(R);
    R = NULL;

    uint8_t *r_1_100_150_B = NULL;
    resize = wah_or_8(r_1_100_150, r_1_100_150, &r_1_100_150_B, &r_size);
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(r_1_100_150_B, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(100, R[1]);
    TEST_ASSERT_EQUAL(150, R[2]);
    free(R);
    R = NULL;

    uint8_t *r_1_3_100_150 = NULL;
    resize = wah_or_8(r_1_100_150, w_3, &r_1_3_100_150, &r_size);
    TEST_ASSERT_EQUAL(4, wah_get_ints_8(r_1_3_100_150, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(3, R[1]);
    TEST_ASSERT_EQUAL(100, R[2]);
    TEST_ASSERT_EQUAL(150, R[3]);
    free(R);
    R = NULL;

    uint8_t *r_1_3_8_100_150 = NULL;
    resize = wah_or_8(r_1_3_100_150, w_8, &r_1_3_8_100_150, &r_size);
    uint8_t *r_1_3_8_10_100_150 = NULL;
    resize = wah_or_8(r_1_3_8_100_150, w_10, &r_1_3_8_10_100_150, &r_size);

    TEST_ASSERT_EQUAL(6, wah_get_ints_8(r_1_3_8_10_100_150, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(3, R[1]);
    TEST_ASSERT_EQUAL(8, R[2]);
    TEST_ASSERT_EQUAL(10, R[3]);
    TEST_ASSERT_EQUAL(100, R[4]);
    TEST_ASSERT_EQUAL(150, R[5]);
    free(R);
    R = NULL;

    free(w_8);
    free(w_10);
    free(w_3);
    free(w_1);
    free(w_100);
    free(w_150);
    free(r_100_150);
    free(r_1_100_150);
    free(r_1_100_150_B);
    free(r_1_3_100_150);
    free(r_1_3_8_100_150);
    free(r_1_3_8_10_100_150);
}
//}}}

//{{{void test_wah_or_8_resize(void)
void test_wah_or_8_resize(void)
{
    //150: 2 10010101 00010000
    //       149      16
    uint8_t *w_150 = wah_init(8, 150);
    // 100: 2 10001110 00100000
    //        142      32
    uint8_t *w_100 = wah_init(8, 100);
    //   1: 1 01000000
    //        64
    uint8_t *w_1 = wah_init(8, 1);
    //   1: 1 00010000
    //        16
    uint8_t *w_3 = wah_init(8, 3);

    uint8_t *w_8 = wah_init(8, 8);
    uint8_t *w_10 = wah_init(8, 10);


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

    // This one is too small
    uint32_t r_size = sizeof(uint32_t) + sizeof(uint8_t);
    uint8_t *r = (uint8_t *)malloc(r_size);

    uint32_t resize = wah_or_8(w_150, w_100, &r, &r_size);

    TEST_ASSERT_EQUAL(1, resize);
    TEST_ASSERT_EQUAL(4, WAH_LEN(r));
    TEST_ASSERT_EQUAL(sizeof(uint32_t) + 4*sizeof(uint8_t), r_size);
    TEST_ASSERT_EQUAL(142, WAH_I(r, 8, 0));
    TEST_ASSERT_EQUAL( 32, WAH_I(r, 8, 1));
    TEST_ASSERT_EQUAL(134, WAH_I(r, 8, 2));
    TEST_ASSERT_EQUAL( 16, WAH_I(r, 8, 3));

    uint32_t *R = NULL; 
    TEST_ASSERT_EQUAL(2, wah_get_ints_8(r, &R));
    TEST_ASSERT_EQUAL(100, R[0]);
    TEST_ASSERT_EQUAL(150, R[1]);

    free(R);
    R = NULL; 
    free(r);

    // Larger than needed
    r_size = sizeof(uint32_t) + 10*sizeof(uint8_t);
    r = (uint8_t *)malloc(r_size);

    resize = wah_or_8(w_150, w_100, &r, &r_size);
    TEST_ASSERT_EQUAL(0, resize);
    TEST_ASSERT_EQUAL(4, WAH_LEN(r));
    TEST_ASSERT_EQUAL(sizeof(uint32_t) + 10*sizeof(uint8_t), r_size);
    TEST_ASSERT_EQUAL(142, WAH_I(r, 8, 0));
    TEST_ASSERT_EQUAL( 32, WAH_I(r, 8, 1));
    TEST_ASSERT_EQUAL(134, WAH_I(r, 8, 2));
    TEST_ASSERT_EQUAL( 16, WAH_I(r, 8, 3));

    free(R);
    R = NULL; 
    free(r);

    // Reuse 
    uint32_t r_0_size = sizeof(uint32_t) + 10*sizeof(uint8_t);
    uint8_t *r_0 = (uint8_t *)malloc(r_0_size);
    memset(r_0, 0, r_0_size);

    uint32_t r_1_size = sizeof(uint32_t) + 10*sizeof(uint8_t);
    uint8_t *r_1 = (uint8_t *)malloc(r_1_size);
    memset(r_1, 0, r_1_size);

    resize = wah_or_8(w_150, w_100, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or_8(r_0, w_1, &r_1, &r_1_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or_8(r_1, w_3, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or_8(r_0, w_8, &r_1, &r_1_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or_8(r_1, w_10, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);

    TEST_ASSERT_EQUAL(6, wah_get_ints_8(r_0, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(3, R[1]);
    TEST_ASSERT_EQUAL(8, R[2]);
    TEST_ASSERT_EQUAL(10, R[3]);
    TEST_ASSERT_EQUAL(100, R[4]);
    TEST_ASSERT_EQUAL(150, R[5]);

    free(R);
    free(r_0);
    free(r_1);

    free(w_8);
    free(w_10);
    free(w_3);
    free(w_1);
    free(w_100);
    free(w_150);
}
//}}}

//{{{ void test_wah_8_uniq_append(void)
void test_wah_8_uniq_append(void)
{
    uint8_t *w = NULL;
    wah_8_uniq_append(&w, 5);

    uint32_t *R;
    TEST_ASSERT_EQUAL(1, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);

    wah_8_uniq_append(&w, 1);
    wah_8_uniq_append(&w, 100);
    wah_8_uniq_append(&w, 10);
    wah_8_uniq_append(&w, 50);
    wah_8_uniq_append(&w, 75);

    free(R);
    R = NULL;

    TEST_ASSERT_EQUAL(6, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(10, R[2]);
    TEST_ASSERT_EQUAL(50, R[3]);
    TEST_ASSERT_EQUAL(75, R[4]);
    TEST_ASSERT_EQUAL(100, R[5]);

    free(R);
    free(w);
}
//}}}

//{{{void test_wah_8_non_leading_SA_SE_add_scalar(void)
void test_wah_8_non_leading_SA_SE_add_scalar(void)
{
    struct wah_8_bpt_non_leading_data 
            *nld = wah_8_new_non_leading(0);

    uint32_t v = 5;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);
    v = 1;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);
    v = 10;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);
    v = 100;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);
    v = 50;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);
    v = 75;
    wah_8_non_leading_SA_add_scalar(0, nld, &v);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints_8(nld->SA, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(10, R[2]);
    TEST_ASSERT_EQUAL(50, R[3]);
    TEST_ASSERT_EQUAL(75, R[4]);
    TEST_ASSERT_EQUAL(100, R[5]);

    v = 1000;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);
    v = 700;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);
    v = 1;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);
    v = 99;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);
    v = 6;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);
    v = 8;
    wah_8_non_leading_SE_add_scalar(0, nld, &v);

    free(R);
    R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints_8(nld->SE, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(8, R[2]);
    TEST_ASSERT_EQUAL(99, R[3]);
    TEST_ASSERT_EQUAL(700, R[4]);
    TEST_ASSERT_EQUAL(1000, R[5]);

    wah_8_non_leading_free((void **)&nld);
    free(R);
}
//}}}

//{{{void test_wah_8_leading_B_add_scalar(void)
void test_wah_8_leading_B_add_scalar(void)
{
    struct wah_8_bpt_leading_data 
            *ld = wah_8_new_leading(0);

    uint32_t v = 5;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 1;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 10;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 100;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 50;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 75;
    wah_8_leading_B_add_scalar(0, ld, &v);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints_8(ld->B, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(10, R[2]);
    TEST_ASSERT_EQUAL(50, R[3]);
    TEST_ASSERT_EQUAL(75, R[4]);
    TEST_ASSERT_EQUAL(100, R[5]);

    free(R);
    wah_8_leading_free((void **)&ld);
}
//}}}

//{{{ void test_wah_8_leading_union_with_B(void)
void test_wah_8_leading_union_with_B(void)
{
    struct wah_8_bpt_leading_data 
            *ld = wah_8_new_leading(0);

    uint32_t v = 5;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 1;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 10;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 100;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 50;
    wah_8_leading_B_add_scalar(0, ld, &v);
    v = 75;
    wah_8_leading_B_add_scalar(0, ld, &v);

    uint8_t *r = NULL;
    wah_8_leading_union_with_B(0, (void **)&r, (void *)ld);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints_8(r, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(10, R[2]);
    TEST_ASSERT_EQUAL(50, R[3]);
    TEST_ASSERT_EQUAL(75, R[4]);
    TEST_ASSERT_EQUAL(100, R[5]);

    free(R);

    struct wah_8_bpt_leading_data 
            *ld_2 = wah_8_new_leading(0);

    v = 11;
    wah_8_leading_B_add_scalar(0, ld_2, &v);
    v = 17;
    wah_8_leading_B_add_scalar(0, ld_2, &v);

    wah_8_leading_union_with_B(0, (void **)&r, (void *)ld_2);

    R = NULL;
    TEST_ASSERT_EQUAL(8, wah_get_ints_8(r, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(10, R[2]);
    TEST_ASSERT_EQUAL(11, R[3]);
    TEST_ASSERT_EQUAL(17, R[4]);
    TEST_ASSERT_EQUAL(50, R[5]);
    TEST_ASSERT_EQUAL(75, R[6]);
    TEST_ASSERT_EQUAL(100, R[7]);

    free(R);

    free(r);
    wah_8_leading_free((void **)&ld);
    wah_8_leading_free((void **)&ld_2);
}
//}}}

//{{{void test_wah_8_non_leading_union_with_SA_subtract_SE(void)
void test_wah_8_non_leading_union_with_SA_subtract_SE(void)
{
    uint8_t *r = NULL;
    wah_8_uniq_append(&r, 5);
    wah_8_uniq_append(&r, 10);


    struct wah_8_bpt_non_leading_data *nld_a = wah_8_new_non_leading(0);

    wah_8_uniq_append(&(nld_a->SA), 5);
    wah_8_uniq_append(&(nld_a->SA), 50);
    wah_8_uniq_append(&(nld_a->SA), 6);

    wah_8_uniq_append(&(nld_a->SE), 10);

    struct wah_8_bpt_non_leading_data *nld_b = wah_8_new_non_leading(0);

    wah_8_uniq_append(&(nld_b->SA), 51);
    wah_8_uniq_append(&(nld_b->SA), 3);
    wah_8_uniq_append(&(nld_b->SA), 12);

    wah_8_uniq_append(&(nld_b->SE), 50);


    wah_8_non_leading_union_with_SA_subtract_SE(0, (void **)&r, (void *)nld_a);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(r, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);

    wah_8_non_leading_union_with_SA_subtract_SE(0, (void **)&r, (void *)nld_b);

    R = NULL;
    TEST_ASSERT_EQUAL(5, wah_get_ints_8(r, &R));
    TEST_ASSERT_EQUAL(3, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(6, R[2]);
    TEST_ASSERT_EQUAL(12, R[3]);
    TEST_ASSERT_EQUAL(51, R[4]);

    free(r);
    free(R);
    wah_8_non_leading_free((void **)&nld_a);
    wah_8_non_leading_free((void **)&nld_b);
}
//}}}

//{{{void test_wah_8_non_leading_serialize(void)
void test_wah_8_non_leading_serialize(void)
{


    // BOTH NULL
    struct wah_8_bpt_non_leading_data *nld_a = NULL;

    uint8_t *s = NULL;

    uint64_t size = wah_8_non_leading_serialize(nld_a, (void **)&s);

    TEST_ASSERT_EQUAL(NULL, s);
    TEST_ASSERT_EQUAL(0, size);

    // SE NULL
    nld_a = wah_8_new_non_leading(0);

    wah_8_uniq_append(&(nld_a->SA), 5);
    wah_8_uniq_append(&(nld_a->SA), 50);
    wah_8_uniq_append(&(nld_a->SA), 6);

    s = NULL;
    size = wah_8_non_leading_serialize(nld_a, (void **)&s);

    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SA)*sizeof(uint8_t),
                      size);

    uint32_t *i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SA)*sizeof(uint8_t),
                      i[0]);
    TEST_ASSERT_EQUAL(0, i[1]);
    uint8_t *w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SA),WAH_LEN(w));
    
    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);
    R = NULL;

    free(s);
    s = NULL;


    // SA and SE NOT NULL
    wah_8_uniq_append(&(nld_a->SE), 1);
    wah_8_uniq_append(&(nld_a->SE), 1000);

    size = wah_8_non_leading_serialize(nld_a, (void **)&s);
    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SA)*sizeof(uint8_t) +
                        sizeof(uint32_t)+WAH_LEN(nld_a->SE)*sizeof(uint8_t),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SA)*sizeof(uint8_t),
                      i[0]);
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SE)*sizeof(uint8_t),
                      i[1]);

    w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SA),WAH_LEN(w));
    
    R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);

    w = w + i[0];
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SE),WAH_LEN(w));

    R = NULL;
    TEST_ASSERT_EQUAL(2, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(1000, R[1]);

    free(R);

    free(s);
    s = NULL;

    // SA NULL
    free(nld_a->SA);
    nld_a->SA = NULL;

    size = wah_8_non_leading_serialize(nld_a, (void **)&s);
    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SE)*sizeof(uint8_t),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(0, i[0]);
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SE)*sizeof(uint8_t),
                      i[1]);

    w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SE),WAH_LEN(w));

    R = NULL;
    TEST_ASSERT_EQUAL(2, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(1000, R[1]);

    free(s);
    free(R);
    wah_8_non_leading_free((void **)&nld_a);
}
//}}}

//{{{void test_wah_8_non_leading_deserialize(void)
void test_wah_8_non_leading_deserialize(void)
{


    // BOTH NULL
    struct wah_8_bpt_non_leading_data *nld_d = NULL;

    uint8_t *s = NULL;
    uint64_t s_size = 0;

    uint64_t d_size = wah_8_non_leading_deserialize((void *)s,
                                                    s_size,
                                                    (void **)&nld_d);

    TEST_ASSERT_EQUAL(NULL, nld_d);
    TEST_ASSERT_EQUAL(0, d_size);

   

    struct wah_8_bpt_non_leading_data *nld_o = wah_8_new_non_leading(0);

    uint32_t i;
    for (i = 0; i < 10; ++i) {
        wah_8_uniq_append(&(nld_o->SA), rand());
    }

    s = NULL;
    s_size = wah_8_non_leading_serialize(nld_o, (void **)&s);

    d_size = wah_8_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_8_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(nld_o->SE, nld_d->SE);

    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SA), WAH_LEN(nld_d->SA));

    uint32_t *R_o = NULL, *R_d = NULL;
    uint32_t R_o_len = wah_get_ints_8(nld_o->SA, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints_8(nld_d->SA, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_8_non_leading_free((void **)&nld_d);

    free(R_o);
    free(R_d);
    free(s);

    // SA and SE NOT NULL
    for (i = 0; i < 5; ++i) {
        wah_8_uniq_append(&(nld_o->SE), rand());
    }

    s=NULL;
    s_size = wah_8_non_leading_serialize(nld_o, (void **)&s);

    nld_d = NULL;
    d_size = wah_8_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_8_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SA), WAH_LEN(nld_d->SA));
    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SE), WAH_LEN(nld_d->SE));

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints_8(nld_o->SA, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints_8(nld_d->SA, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    free(R_o);
    free(R_d);

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints_8(nld_o->SE, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints_8(nld_d->SE, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_8_non_leading_free((void **)&nld_d);

    free(R_o);
    free(R_d);
    free(s);

    // SA NULL
    free(nld_o->SA);
    nld_o->SA = NULL;

    s=NULL;
    s_size = wah_8_non_leading_serialize(nld_o, (void **)&s);

    nld_d = NULL;
    d_size = wah_8_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_8_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(nld_o->SA, nld_d->SA);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SE), WAH_LEN(nld_d->SE));

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints_8(nld_o->SE, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints_8(nld_d->SE, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_8_non_leading_free((void **)&nld_d);


    free(R_o);
    free(R_d);
    free(s);

    wah_8_non_leading_free((void **)&nld_o);
}
//}}}

//{{{void test_wah_8_leading_serialize(void)
void test_wah_8_leading_serialize(void)
{
    // NULL
    struct wah_8_bpt_leading_data *ld_a = NULL;

    uint8_t *s = NULL;

    uint64_t size = wah_8_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(NULL, s);
    TEST_ASSERT_EQUAL(0, size);

    // B NULL
    ld_a = wah_8_new_non_leading(0);

    s = NULL;
    size = wah_8_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(sizeof(uint32_t), size);

    uint32_t *i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(0, i[0]);

    free(s);


    // B NOT NULL

    wah_8_uniq_append(&(ld_a->B), 5);
    wah_8_uniq_append(&(ld_a->B), 50);
    wah_8_uniq_append(&(ld_a->B), 6);

    s = NULL;
    size = wah_8_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(sizeof(uint32_t)+
                      sizeof(uint32_t)+WAH_LEN(ld_a->B)*sizeof(uint8_t),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) + WAH_LEN(ld_a->B)*sizeof(uint8_t),
                      i[0]);
    uint8_t *w = (uint8_t *)(i+1);
    TEST_ASSERT_EQUAL(WAH_LEN(ld_a->B),WAH_LEN(w));
    
    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints_8(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);
    free(s);
    wah_8_leading_free((void **)&ld_a);
}
//}}}

//{{{void test_wah_8_leading_deserialize(void)
void test_wah_8_leading_deserialize(void)
{
    // NULL
    struct wah_8_bpt_leading_data *ld_d = NULL;

    uint8_t *s = NULL;
    uint64_t s_size = 0;

    uint64_t d_size = wah_8_leading_deserialize((void *)s,
                                                s_size,
                                                (void **)&ld_d);

    TEST_ASSERT_EQUAL(NULL, ld_d);
    TEST_ASSERT_EQUAL(0, d_size);

   
    // B is NULL
    struct wah_8_bpt_leading_data *ld_o = wah_8_new_leading(0);

    s = NULL;
    s_size = wah_8_leading_serialize(ld_o, (void **)&s);
    d_size = wah_8_leading_deserialize((void *)s,
                                       s_size,
                                       (void **)&ld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_8_bpt_leading_data), d_size);
    TEST_ASSERT_EQUAL(NULL, ld_d->B);

    free(s);
    wah_8_leading_free((void **)&ld_d);


    // B is NOT NULL
    uint32_t i;
    for (i = 0; i < 10; ++i) {
        wah_8_uniq_append(&(ld_o->B), rand());
    }

    s = NULL;
    s_size = wah_8_leading_serialize(ld_o, (void **)&s);

    d_size = wah_8_leading_deserialize((void *)s,
                                       s_size,
                                       (void **)&ld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_8_bpt_leading_data), d_size);
    TEST_ASSERT_EQUAL(WAH_LEN(ld_o->B), WAH_LEN(ld_d->B));

    uint32_t *R_o = NULL, *R_d = NULL;
    uint32_t R_o_len = wah_get_ints_8(ld_o->B, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints_8(ld_d->B, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_8_leading_free((void **)&ld_o);
    wah_8_leading_free((void **)&ld_d);

    free(R_o);
    free(R_d);
    free(s);
}
//}}}
