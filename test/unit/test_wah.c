#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <inttypes.h>
#include <err.h>


#include "wah.h"
#include "unity.h"
#include "util.h"

void setUp(void) { }
void tearDown(void) { }

//{{{ void test_wah_or(void)
void run_test_wah_or(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t size = 10;

    uint8_t **W = (uint8_t **)calloc(size, sizeof(uint8_t *));
    uint32_t *V = (uint32_t *)calloc(size, sizeof(uint32_t));

    uint32_t R_size, *R = NULL;

    uint32_t i;
    for (i = 0; i < size; ++i) {
        V[i] = rand();
        W[i] = wah_init(V[i]);

        TEST_ASSERT_EQUAL(1, wah_get_ints(W[i], &R));
        TEST_ASSERT_EQUAL(V[i], R[0]);

        free(R);
        R=NULL;
    }

    uint8_t *w = W[0];
    uint8_t *r[2] = {NULL, NULL};
    uint32_t r_size;

    for (i = 1; i < size; ++i) {
        uint32_t resize = wah_or(w, W[i], &(r[i%2]), &r_size);
        w = r[i%2];

        TEST_ASSERT_EQUAL(i + 1, wah_get_ints(w, &R));
        free(R);
        R=NULL;
    }

    TEST_ASSERT_EQUAL(10, wah_get_ints(w, &R));
}

//void test_wah_or(void) { run_test_wah_or(8); }
//void test_wah_or_16(void) { run_test_wah_or(16); }
void test_wah_or_32(void) { run_test_wah_or(32); }
//}}}

//{{{ void test_wah_nand(uint32_t word_size)
void run_test_wah_nand(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;
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

    uint8_t *w = wah_init( 150);
    uint8_t *u = wah_init( 100);
    uint8_t *x = wah_init( 1);
    uint8_t *y = wah_init( 3);
    uint8_t *z = wah_init( 6);

    uint8_t *r = NULL;
    uint8_t *r_3 = NULL;
    uint8_t *r_2 = NULL;
    uint32_t r_size = 0;
    uint32_t *R = NULL;

    uint32_t resize = wah_or(w, u, &r, &r_size);
    TEST_ASSERT_EQUAL(2, wah_get_ints(r, &R));
    free(R);
    R=NULL;
    

    resize = wah_or(r, x, &r_2, &r_size);
    resize = wah_or(r_2, r_2, &r_3, &r_size);
    resize = wah_or(r_3, y, &r_2, &r_size);
    resize = wah_or(r_2, z, &r_3, &r_size);

    TEST_ASSERT_EQUAL(5, wah_get_ints(r_3, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(3, R[1]);
    TEST_ASSERT_EQUAL(6, R[2]);
    TEST_ASSERT_EQUAL(100, R[3]);
    TEST_ASSERT_EQUAL(150, R[4]);

    free(R);
    R = NULL;

    resize = wah_nand(r_3, y, &r_2, &r_size);

    TEST_ASSERT_EQUAL(4, wah_get_ints(r_2, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(100, R[2]);
    TEST_ASSERT_EQUAL(150, R[3]);

    free(R);
    R = NULL;

    uint8_t *r_4 = NULL;
    resize = wah_or(x, w, &r_4, &r_size);

    resize = wah_nand(r_2, r_4, &r_3, &r_size);
    TEST_ASSERT_EQUAL(2, wah_get_ints(r_3, &R));
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

//void test_wah_nand(void) { run_test_wah_nand(8); }
//void test_wah_nand_16(void) { run_test_wah_nand(16); }
void test_wah_nand_32(void) { run_test_wah_nand(32); }
//}}}

//{{{void test_macros(void)
void test_wah_get_set(void)
{
    WAH_SIZE = 8;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint8_t *d = (uint8_t *)calloc(100, sizeof(uint8_t));
    uint32_t *i = (uint32_t *)d;

    i[0] = 10;

    uint32_t v;

    uint32_t o_8_0 = 0, o_8_1 = 0, o_8_2 = 0,
             o_16_0 = 0, o_16_1 = 0, o_16_2 = 0,
             o_32_0 = 0, o_32_1 = 0, o_32_2 = 0;

    // 8
    v = (1<<7) + 5;
    set_wah_i(d, &v, 8, 0);
    v += 1;
    set_wah_i(d, &v, 8, 1);
    v += 1;
    set_wah_i(d, &v, 8, 2);

    get_wah_i(d, &o_8_0, 8, 0);
    get_wah_i(d, &o_8_1, 8, 1);
    get_wah_i(d, &o_8_2, 8, 2);

    // 16 
    v = (1<<7) + 5;
    set_wah_i(d, &v, 16, 0);
    v += 1;
    set_wah_i(d, &v, 16, 1);
    v += 1;
    set_wah_i(d, &v, 16, 2);

    get_wah_i(d, &o_16_0, 16, 0);
    get_wah_i(d, &o_16_1, 16, 1);
    get_wah_i(d, &o_16_2, 16, 2);

    // 32 
    v = (1<<7) + 5;
    set_wah_i(d, &v, 32, 0);
    v += 1;
    set_wah_i(d, &v, 32, 1);
    v += 1;
    set_wah_i(d, &v, 32, 2);

    get_wah_i(d, &o_32_0, 32, 0);
    get_wah_i(d, &o_32_1, 32, 1);
    get_wah_i(d, &o_32_2, 32, 2);

    TEST_ASSERT_TRUE(o_8_0 == o_16_0);
    TEST_ASSERT_TRUE(o_8_1 == o_16_1);
    TEST_ASSERT_TRUE(o_8_2 == o_16_2);

    TEST_ASSERT_TRUE(o_16_0 == o_32_0);
    TEST_ASSERT_TRUE(o_16_1 == o_32_1);
    TEST_ASSERT_TRUE(o_16_2 == o_32_2);

    TEST_ASSERT_TRUE(o_8_0 == o_32_0);
    TEST_ASSERT_TRUE(o_8_1 == o_32_1);
    TEST_ASSERT_TRUE(o_8_2 == o_32_2);

    v = (1<<7) + 5;
    set_wah_i(d, &v, 8, 1);
    uint32_t r = 0;
    get_wah_i(d, &r, 8, 1);
    TEST_ASSERT_EQUAL(0, WAH_VAL(r, 8));
    TEST_ASSERT_EQUAL(5, WAH_NUM_WORDS(r, 8));
    v = 5;
    set_wah_i(d, &v, 8, 1);
    get_wah_i(d, &r, 8, 1);
    TEST_ASSERT_EQUAL(5, WAH_VAL(r, 8));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(r, 8));

    v = (1<<15) + 5;
    set_wah_i(d, &v, 16, 1);
    r = 0;
    get_wah_i(d, &r, 16, 1);
    TEST_ASSERT_EQUAL(0, WAH_VAL(r, 16));
    TEST_ASSERT_EQUAL(5, WAH_NUM_WORDS(r, 16));
    v = 5;
    set_wah_i(d, &v, 16, 1);
    get_wah_i(d, &r, 16, 1);
    TEST_ASSERT_EQUAL(5, WAH_VAL(r, 16));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(r, 16));

    v = (1<<31) + 5;
    set_wah_i(d, &v, 32, 1);
    r = 0;
    get_wah_i(d, &r, 32, 1);
    TEST_ASSERT_EQUAL(0, WAH_VAL(r, 32));
    TEST_ASSERT_EQUAL(5, WAH_NUM_WORDS(r, 32));
    v = 5;
    set_wah_i(d, &v, 32, 1);
    get_wah_i(d, &r, 32, 1);
    TEST_ASSERT_EQUAL(5, WAH_VAL(r, 32));
    TEST_ASSERT_EQUAL(1, WAH_NUM_WORDS(r, 32));
     
    free(d);
}
//}}}

//{{{void test_wah_init(void)
void run_test_wah_init(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t size = 10;

    uint8_t **W = (uint8_t **)calloc(size, sizeof(uint8_t *));
    uint32_t *V = (uint32_t *)calloc(size, sizeof(uint32_t));

    uint32_t R_size, *R = NULL;

    uint32_t i;
    for (i = 0; i < size; ++i) {
        V[i] = rand();
        W[i] = wah_init(V[i]);

        TEST_ASSERT_EQUAL(1, wah_get_ints(W[i], &R));
        TEST_ASSERT_EQUAL(V[i], R[0]);

        free(R);
        R=NULL;
    }
}

//void test_wah_init(void) { run_test_wah_init(8); }
//void test_wah_init_16(void){ run_test_wah_init(16); }
void test_wah_init_32(void){ run_test_wah_init(32); }
//}}}

//{{{ void test_wah_get_ints(void)
void run_test_wah_get_ints(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint8_t *w = NULL;
    uint32_t *R = NULL;
    uint32_t R_size;

    uint32_t i;
    for (i = 0; i < 100; ++i) {
        uint32_t set_v = rand(); 
        w = wah_init(set_v);
        R_size = wah_get_ints(w, &R);
        TEST_ASSERT_EQUAL(1, R_size);
        TEST_ASSERT_EQUAL(set_v, R[0]);

        free(R);
        free(w);
        R = NULL;
        w = NULL;
    }
}

//void test_wah_get_ints(void) { run_test_wah_get_ints(8); }
//void test_wah_get_ints_16(void) { run_test_wah_get_ints(16); }
void test_wah_get_ints_32(void) { run_test_wah_get_ints(32); }

//}}}

//{{{ void test_wah_uniq_append(void)
void run_test_wah_uniq_append(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t size = 10;

    uint8_t *w = NULL;
    uint32_t *V = (uint32_t *)calloc(size, sizeof(uint32_t));

    uint32_t R_size, *R = NULL;

    uint32_t i;
    for (i = 0; i < size; ++i) {
        V[i] = rand();
        wah_uniq_append(&w, V[i]);
        TEST_ASSERT_EQUAL(i + 1, wah_get_ints(w, &R));
        free(R);
        R = NULL;
    }

    qsort(V, size, sizeof(uint32_t), uint32_t_cmp);

    TEST_ASSERT_EQUAL(size, wah_get_ints(w, &R));

    for (i = 0; i < size; ++i) {
        TEST_ASSERT_EQUAL(V[i] , R[i]);
    }
}

//void wah_uniq_append(void) { test_wah_uniq_append(8); }
//void wah_uniq_append_16(void) { test_wah_uniq_append(16); }
void test_wah_uniq_append_32(void) { run_test_wah_uniq_append(32); }
//}}}

//{{{void test_wah_copy(void)
void run_test_wah_copy(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    uint32_t *R = NULL;
    uint8_t *o = NULL;
    wah_uniq_append(&o, rand());
    TEST_ASSERT_EQUAL(1, wah_get_ints(o, &R));
    free(R);
    R = NULL;

    wah_uniq_append(&o, rand());
    TEST_ASSERT_EQUAL(2, wah_get_ints(o, &R));
    free(R);
    R = NULL;

    wah_uniq_append(&o, rand());
    TEST_ASSERT_EQUAL(3, wah_get_ints(o, &R));
    free(R);
    R = NULL;

    wah_uniq_append(&o, rand());
    TEST_ASSERT_EQUAL(4, wah_get_ints(o, &R));
    free(R);
    R = NULL;

    uint8_t *c = wah_copy(o);
    TEST_ASSERT_EQUAL(WAH_LEN(o), WAH_LEN(c));

    uint32_t *R_o = NULL, *R_c = NULL;
    TEST_ASSERT_EQUAL(4, wah_get_ints(o, &R_o));
    TEST_ASSERT_EQUAL(4, wah_get_ints(c, &R_c));

    TEST_ASSERT_EQUAL(R_o[0], R_c[0]);
    TEST_ASSERT_EQUAL(R_o[1], R_c[1]);
    TEST_ASSERT_EQUAL(R_o[2], R_c[2]);
    TEST_ASSERT_EQUAL(R_o[3], R_c[3]);

    free(R_o);
    free(R_c);
    free(o);
    free(c);
}

//void test_wah_copy(void) { run_test_wah_copy(8); }
//void test_wah_copy(void) { run_test_wah_copy(8); }
void test_wah_copy_32(void) { run_test_wah_copy(32); }

//}}}

//{{{void test_wah_or_resize(void)
void run_test_wah_or_resize(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;


    uint8_t *w_150 = wah_init( 150);
    uint8_t *w_100 = wah_init( 100);
    uint8_t *w_1 = wah_init( 1);
    uint8_t *w_3 = wah_init( 3);
    uint8_t *w_8 = wah_init( 8);
    uint8_t *w_10 = wah_init( 10);


    // This one is too small
    uint32_t r_size = sizeof(uint32_t) + sizeof(uint8_t);
    uint8_t *r = (uint8_t *)malloc(r_size);
    uint32_t resize = wah_or(w_150, w_100, &r, &r_size);
    TEST_ASSERT_EQUAL(1, resize);

    uint32_t *R = NULL; 
    TEST_ASSERT_EQUAL(2, wah_get_ints(r, &R));
    TEST_ASSERT_EQUAL(100, R[0]);
    TEST_ASSERT_EQUAL(150, R[1]);

    free(R);
    R = NULL; 
    free(r);

    // Larger than needed
    r_size = sizeof(uint32_t) + 10*sizeof(uint32_t);
    r = (uint8_t *)malloc(r_size);

    resize = wah_or(w_150, w_100, &r, &r_size);
    TEST_ASSERT_EQUAL(0, resize);

    free(R);
    R = NULL; 
    free(r);

    // Reuse 
    uint32_t r_0_size = sizeof(uint32_t) + 10*sizeof(uint32_t);
    uint8_t *r_0 = (uint8_t *)malloc(r_0_size);
    memset(r_0, 0, r_0_size);

    uint32_t r_1_size = sizeof(uint32_t) + 10*sizeof(uint32_t);
    uint8_t *r_1 = (uint8_t *)malloc(r_1_size);
    memset(r_1, 0, r_1_size);

    resize = wah_or(w_150, w_100, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or(r_0, w_1, &r_1, &r_1_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or(r_1, w_3, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or(r_0, w_8, &r_1, &r_1_size);
    TEST_ASSERT_EQUAL(0, resize);
    resize = wah_or(r_1, w_10, &r_0, &r_0_size);
    TEST_ASSERT_EQUAL(0, resize);

    TEST_ASSERT_EQUAL(6, wah_get_ints(r_0, &R));
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

//void test_wah_or_resize(void) { run_test_wah_or_resize(8); }
//void test_wah_or_resize_16(void) { run_test_wah_or_resize(16); }
void test_wah_or_resize_32(void) { run_test_wah_or_resize(32); }
//}}}

//{{{void test_wah_non_leading_SA_SE_add_scalar(void)
void run_test_wah_non_leading_SA_SE_add_scalar(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct wah_bpt_non_leading_data 
            *nld = wah_new_non_leading(0);

    uint32_t v = 5;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 1;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 10;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 100;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 50;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 75;
    wah_non_leading_SA_add_scalar(0, nld, &v);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints(nld->SA, &R));
    // Values in wah are 1-based
    TEST_ASSERT_EQUAL(1, R[0] - 1);
    TEST_ASSERT_EQUAL(5, R[1] - 1);
    TEST_ASSERT_EQUAL(10, R[2] - 1);
    TEST_ASSERT_EQUAL(50, R[3] - 1);
    TEST_ASSERT_EQUAL(75, R[4] - 1);
    TEST_ASSERT_EQUAL(100, R[5] - 1);

    v = 1000;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 700;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 1;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 99;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 6;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 8;
    wah_non_leading_SE_add_scalar(0, nld, &v);

    free(R);
    R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints(nld->SE, &R));
    // Values in wah are 1-based
    TEST_ASSERT_EQUAL(1, R[0] - 1);
    TEST_ASSERT_EQUAL(6, R[1] - 1);
    TEST_ASSERT_EQUAL(8, R[2] - 1);
    TEST_ASSERT_EQUAL(99, R[3] - 1);
    TEST_ASSERT_EQUAL(700, R[4] - 1);
    TEST_ASSERT_EQUAL(1000, R[5] - 1);

    wah_non_leading_free((void **)&nld);
    free(R);
}

void test_wah_non_leading_SA_SE_add_scalar_32(void)
{
    run_test_wah_non_leading_SA_SE_add_scalar(32);
}

//}}}

//{{{void test_wah_leading_B_add_scalar(void)
void run_test_wah_leading_B_add_scalar(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct wah_bpt_leading_data 
            *ld = wah_new_leading(0);

    uint32_t v = 5;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 1;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 10;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 100;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 50;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 75;
    wah_leading_B_add_scalar(0, ld, &v);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints(ld->B, &R));
    // Values in wah are 1-based
    TEST_ASSERT_EQUAL(1, R[0] - 1);
    TEST_ASSERT_EQUAL(5, R[1] - 1);
    TEST_ASSERT_EQUAL(10, R[2] - 1);
    TEST_ASSERT_EQUAL(50, R[3] - 1);
    TEST_ASSERT_EQUAL(75, R[4] - 1);
    TEST_ASSERT_EQUAL(100, R[5] - 1);

    free(R);
    wah_leading_free((void **)&ld);
}

void test_wah_leading_B_add_scalar_32(void) {
    run_test_wah_leading_B_add_scalar(32);
}
//}}}

//{{{ void test_wah_leading_union_with_B(void)
void run_test_wah_leading_union_with_B(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct wah_bpt_leading_data 
            *ld = wah_new_leading(0);

    uint32_t v = 5;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 1;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 10;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 100;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 50;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 75;
    wah_leading_B_add_scalar(0, ld, &v);

    uint8_t *r = NULL;
    wah_leading_union_with_B(0, (void **)&r, (void *)ld);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(6, wah_get_ints(r, &R));
    // Values in wah are 1-based
    TEST_ASSERT_EQUAL(1, R[0] - 1);
    TEST_ASSERT_EQUAL(5, R[1] - 1);
    TEST_ASSERT_EQUAL(10, R[2] - 1);
    TEST_ASSERT_EQUAL(50, R[3] - 1);
    TEST_ASSERT_EQUAL(75, R[4] - 1);
    TEST_ASSERT_EQUAL(100, R[5] - 1);

    free(R);

    struct wah_bpt_leading_data 
            *ld_2 = wah_new_leading(0);

    v = 11;
    wah_leading_B_add_scalar(0, ld_2, &v);
    v = 17;
    wah_leading_B_add_scalar(0, ld_2, &v);

    wah_leading_union_with_B(0, (void **)&r, (void *)ld_2);

    R = NULL;
    TEST_ASSERT_EQUAL(8, wah_get_ints(r, &R));
    // Values in wah are 1-based
    TEST_ASSERT_EQUAL(1, R[0] - 1);
    TEST_ASSERT_EQUAL(5, R[1] - 1);
    TEST_ASSERT_EQUAL(10, R[2] - 1);
    TEST_ASSERT_EQUAL(11, R[3] - 1);
    TEST_ASSERT_EQUAL(17, R[4] - 1);
    TEST_ASSERT_EQUAL(50, R[5] - 1);
    TEST_ASSERT_EQUAL(75, R[6] - 1);
    TEST_ASSERT_EQUAL(100, R[7] - 1);

    free(R);

    free(r);
    wah_leading_free((void **)&ld);
    wah_leading_free((void **)&ld_2);
}

void test_wah_leading_union_with_B_32(void)
{
    run_test_wah_leading_union_with_B(32);
}
//}}}

//{{{void test_wah_non_leading_union_with_SA_subtract_SE(void)
void run_test_wah_non_leading_union_with_SA_subtract_SE(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;


    uint8_t *r = NULL;
    wah_uniq_append(&r, 5);
    wah_uniq_append(&r, 10);


    struct wah_bpt_non_leading_data *nld_a = wah_new_non_leading(0);

    wah_uniq_append(&(nld_a->SA), 5);
    wah_uniq_append(&(nld_a->SA), 50);
    wah_uniq_append(&(nld_a->SA), 6);

    wah_uniq_append(&(nld_a->SE), 10);

    struct wah_bpt_non_leading_data *nld_b = wah_new_non_leading(0);

    wah_uniq_append(&(nld_b->SA), 51);
    wah_uniq_append(&(nld_b->SA), 3);
    wah_uniq_append(&(nld_b->SA), 12);

    wah_uniq_append(&(nld_b->SE), 50);


    wah_non_leading_union_with_SA_subtract_SE(0, (void **)&r, (void *)nld_a);

    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints(r, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);

    wah_non_leading_union_with_SA_subtract_SE(0, (void **)&r, (void *)nld_b);

    R = NULL;
    TEST_ASSERT_EQUAL(5, wah_get_ints(r, &R));
    TEST_ASSERT_EQUAL(3, R[0]);
    TEST_ASSERT_EQUAL(5, R[1]);
    TEST_ASSERT_EQUAL(6, R[2]);
    TEST_ASSERT_EQUAL(12, R[3]);
    TEST_ASSERT_EQUAL(51, R[4]);

    free(r);
    free(R);
    wah_non_leading_free((void **)&nld_a);
    wah_non_leading_free((void **)&nld_b);
}

void test_wah_non_leading_union_with_SA_subtract_SE_32(void)
{
    run_test_wah_non_leading_union_with_SA_subtract_SE(32);
}


//}}}

//{{{void test_wah_non_leading_serialize(void)
void run_test_wah_non_leading_serialize(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    // BOTH NULL
    struct wah_bpt_non_leading_data *nld_a = NULL;

    uint8_t *s = NULL;

    uint64_t size = wah_non_leading_serialize(nld_a, (void **)&s);

    TEST_ASSERT_EQUAL(NULL, s);
    TEST_ASSERT_EQUAL(0, size);

    // SE NULL
    nld_a = wah_new_non_leading(0);

    wah_uniq_append(&(nld_a->SA), 5);
    wah_uniq_append(&(nld_a->SA), 50);
    wah_uniq_append(&(nld_a->SA), 6);

    s = NULL;
    size = wah_non_leading_serialize(nld_a, (void **)&s);

    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SA)*(WAH_SIZE/BYTE),
                      size);

    uint32_t *i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                        WAH_LEN(nld_a->SA)*(WAH_SIZE/BYTE),
                      i[0]);
    TEST_ASSERT_EQUAL(0, i[1]);
    uint8_t *w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SA),WAH_LEN(w));
    
    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);
    R = NULL;

    free(s);
    s = NULL;


    // SA and SE NOT NULL
    wah_uniq_append(&(nld_a->SE), 1);
    wah_uniq_append(&(nld_a->SE), 1000);

    size = wah_non_leading_serialize(nld_a, (void **)&s);
    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SA)*(WAH_SIZE/BYTE) +
                        sizeof(uint32_t)+WAH_LEN(nld_a->SE)*(WAH_SIZE/BYTE),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SA)*(WAH_SIZE/BYTE),
                      i[0]);
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SE)*(WAH_SIZE/BYTE),
                      i[1]);

    w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SA),WAH_LEN(w));
    
    R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);

    w = w + i[0];
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SE),WAH_LEN(w));

    R = NULL;
    TEST_ASSERT_EQUAL(2, wah_get_ints(w, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(1000, R[1]);

    free(R);

    free(s);
    s = NULL;

    // SA NULL
    free(nld_a->SA);
    nld_a->SA = NULL;

    size = wah_non_leading_serialize(nld_a, (void **)&s);
    TEST_ASSERT_EQUAL(2*sizeof(uint32_t)+
                        sizeof(uint32_t)+WAH_LEN(nld_a->SE)*(WAH_SIZE/BYTE),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(0, i[0]);
    TEST_ASSERT_EQUAL(sizeof(uint32_t) +
                            WAH_LEN(nld_a->SE)*(WAH_SIZE/BYTE),
                      i[1]);

    w = (uint8_t *)(i+2);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_a->SE),WAH_LEN(w));

    R = NULL;
    TEST_ASSERT_EQUAL(2, wah_get_ints(w, &R));
    TEST_ASSERT_EQUAL(1, R[0]);
    TEST_ASSERT_EQUAL(1000, R[1]);

    free(s);
    free(R);
    wah_non_leading_free((void **)&nld_a);
}

void test_wah_non_leading_serialize_32(void)
{
    run_test_wah_non_leading_serialize(32);
}
//}}}

//{{{void test_wah_non_leading_deserialize(void)
void run_test_wah_non_leading_deserialize(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;


    // BOTH NULL
    struct wah_bpt_non_leading_data *nld_d = NULL;

    uint8_t *s = NULL;
    uint64_t s_size = 0;

    uint64_t d_size = wah_non_leading_deserialize((void *)s,
                                                    s_size,
                                                    (void **)&nld_d);

    TEST_ASSERT_EQUAL(NULL, nld_d);
    TEST_ASSERT_EQUAL(0, d_size);

   

    struct wah_bpt_non_leading_data *nld_o = wah_new_non_leading(0);

    uint32_t i;
    for (i = 0; i < 10; ++i) {
        wah_uniq_append(&(nld_o->SA), rand());
    }

    s = NULL;
    s_size = wah_non_leading_serialize(nld_o, (void **)&s);

    d_size = wah_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(nld_o->SE, nld_d->SE);

    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SA), WAH_LEN(nld_d->SA));

    uint32_t *R_o = NULL, *R_d = NULL;
    uint32_t R_o_len = wah_get_ints(nld_o->SA, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints(nld_d->SA, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_non_leading_free((void **)&nld_d);

    free(R_o);
    free(R_d);
    free(s);

    // SA and SE NOT NULL
    for (i = 0; i < 5; ++i) {
        wah_uniq_append(&(nld_o->SE), rand());
    }

    s=NULL;
    s_size = wah_non_leading_serialize(nld_o, (void **)&s);

    nld_d = NULL;
    d_size = wah_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SA), WAH_LEN(nld_d->SA));
    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SE), WAH_LEN(nld_d->SE));

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints(nld_o->SA, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints(nld_d->SA, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    free(R_o);
    free(R_d);

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints(nld_o->SE, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints(nld_d->SE, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_non_leading_free((void **)&nld_d);

    free(R_o);
    free(R_d);
    free(s);

    // SA NULL
    free(nld_o->SA);
    nld_o->SA = NULL;

    s=NULL;
    s_size = wah_non_leading_serialize(nld_o, (void **)&s);

    nld_d = NULL;
    d_size = wah_non_leading_deserialize((void *)s,
                                           s_size,
                                           (void **)&nld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_bpt_non_leading_data), d_size);

    TEST_ASSERT_EQUAL(nld_o->SA, nld_d->SA);
    TEST_ASSERT_EQUAL(WAH_LEN(nld_o->SE), WAH_LEN(nld_d->SE));

    R_o = NULL;
    R_d = NULL;
    R_o_len = wah_get_ints(nld_o->SE, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints(nld_d->SE, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_non_leading_free((void **)&nld_d);


    free(R_o);
    free(R_d);
    free(s);

    wah_non_leading_free((void **)&nld_o);
}

void test_wah_non_leading_deserialize_32(void)
{
    run_test_wah_non_leading_deserialize(32);
}
//}}}

//{{{void test_wah_leading_serialize(void)
void run_test_wah_leading_serialize(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    // NULL
    struct wah_bpt_leading_data *ld_a = NULL;

    uint8_t *s = NULL;

    uint64_t size = wah_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(NULL, s);
    TEST_ASSERT_EQUAL(0, size);

    // B NULL
    ld_a = wah_new_non_leading(0);

    s = NULL;
    size = wah_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(sizeof(uint32_t), size);

    uint32_t *i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(0, i[0]);

    free(s);


    // B NOT NULL

    wah_uniq_append(&(ld_a->B), 5);
    wah_uniq_append(&(ld_a->B), 50);
    wah_uniq_append(&(ld_a->B), 6);

    s = NULL;
    size = wah_leading_serialize(ld_a, (void **)&s);

    TEST_ASSERT_EQUAL(sizeof(uint32_t)+
                      sizeof(uint32_t)+WAH_LEN(ld_a->B)*(WAH_SIZE/BYTE),
                      size);

    i = (uint32_t *)s;
    TEST_ASSERT_EQUAL(sizeof(uint32_t) + WAH_LEN(ld_a->B)*(WAH_SIZE/BYTE),
                      i[0]);
    uint8_t *w = (uint8_t *)(i+1);
    TEST_ASSERT_EQUAL(WAH_LEN(ld_a->B),WAH_LEN(w));
    
    uint32_t *R = NULL;
    TEST_ASSERT_EQUAL(3, wah_get_ints(w, &R));
    TEST_ASSERT_EQUAL(5, R[0]);
    TEST_ASSERT_EQUAL(6, R[1]);
    TEST_ASSERT_EQUAL(50, R[2]);

    free(R);
    free(s);
    wah_leading_free((void **)&ld_a);
}

void test_wah_leading_serialize_32(void)
{
    run_test_wah_leading_serialize(32);
}
//}}}

//{{{void test_wah_leading_deserialize(void)
void run_test_wah_leading_deserialize(uint32_t word_size)
{
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    // NULL
    struct wah_bpt_leading_data *ld_d = NULL;

    uint8_t *s = NULL;
    uint64_t s_size = 0;

    uint64_t d_size = wah_leading_deserialize((void *)s,
                                                s_size,
                                                (void **)&ld_d);

    TEST_ASSERT_EQUAL(NULL, ld_d);
    TEST_ASSERT_EQUAL(0, d_size);

   
    // B is NULL
    struct wah_bpt_leading_data *ld_o = wah_new_leading(0);

    s = NULL;
    s_size = wah_leading_serialize(ld_o, (void **)&s);
    d_size = wah_leading_deserialize((void *)s,
                                       s_size,
                                       (void **)&ld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_bpt_leading_data), d_size);
    TEST_ASSERT_EQUAL(NULL, ld_d->B);

    free(s);
    wah_leading_free((void **)&ld_d);


    // B is NOT NULL
    uint32_t i;
    for (i = 0; i < 10; ++i) {
        wah_uniq_append(&(ld_o->B), rand());
    }

    s = NULL;
    s_size = wah_leading_serialize(ld_o, (void **)&s);

    d_size = wah_leading_deserialize((void *)s,
                                       s_size,
                                       (void **)&ld_d);

    TEST_ASSERT_EQUAL(sizeof(struct wah_bpt_leading_data), d_size);
    TEST_ASSERT_EQUAL(WAH_LEN(ld_o->B), WAH_LEN(ld_d->B));

    uint32_t *R_o = NULL, *R_d = NULL;
    uint32_t R_o_len = wah_get_ints(ld_o->B, &R_o);
    TEST_ASSERT_EQUAL(R_o_len, wah_get_ints(ld_d->B, &R_d));

    for (i = 0; i < R_o_len; ++i) {
        TEST_ASSERT_EQUAL(R_o[i], R_d[i]);
    }

    wah_leading_free((void **)&ld_o);
    wah_leading_free((void **)&ld_d);

    free(R_o);
    free(R_d);
    free(s);
}

void test_wah_leading_deserialize_32(void)
{
    run_test_wah_leading_deserialize(32);
}
//}}}

//{{{ void  test_wah_leading_repair(void)
void  test_wah_leading_repair(void)
{
    uint32_t word_size = 32,
             domain = 0;
    WAH_SIZE = word_size;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    ORDER=5;
    struct simple_cache *sc = simple_cache_init(5, 1, NULL);

    /*  (1,2) 1:(3) 2:(4) 3:(5 3) */
    struct bpt_node *n1 = bpt_new_node(domain);
    BPT_NUM_KEYS(n1) = 3;
    BPT_IS_LEAF(n1) = 1;
    struct wah_bpt_leading_data *ld = wah_new_leading(domain);
    uint32_t v = 1;
    wah_leading_B_add_scalar(0, ld, &v);
    v = 2;
    wah_leading_B_add_scalar(0, ld, &v);

    BPT_LEADING(n1) = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              ld,
              sizeof(struct wah_bpt_leading_data),
              &wah_leading_cache_handler );

    struct wah_bpt_non_leading_data *nld = wah_new_non_leading(domain);
    v = 3;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    BPT_KEYS(n1)[0] = 1;
    BPT_POINTERS(n1)[0] = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              nld,
              sizeof(struct wah_bpt_non_leading_data),
              &wah_non_leading_cache_handler );

    nld = wah_new_non_leading(domain);
    v = 4;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    BPT_KEYS(n1)[1] = 2;
    BPT_POINTERS(n1)[1] = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              nld,
              sizeof(struct wah_bpt_non_leading_data),
              &wah_non_leading_cache_handler );

    nld = wah_new_non_leading(domain);
    v = 5;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    v = 3;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    v = 1;
    wah_non_leading_SE_add_scalar(0, nld, &v);
    BPT_KEYS(n1)[2] = 3;
    BPT_POINTERS(n1)[2] = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              nld,
              sizeof(struct wah_bpt_non_leading_data),
              &wah_non_leading_cache_handler );


    /*  () 4:(6) 5:(7) */
    struct bpt_node *n2 = bpt_new_node(domain);
    BPT_NUM_KEYS(n2) = 2;
    BPT_NEXT(n1) = BPT_ID(n2);
    BPT_IS_LEAF(n2) = 1;
    BPT_LEADING(n2) = 0;

    nld = wah_new_non_leading(domain);
    v = 6;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    BPT_KEYS(n2)[0] = 4;
    BPT_POINTERS(n2)[0] = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              nld,
              sizeof(struct wah_bpt_non_leading_data),
              &wah_non_leading_cache_handler );

    nld = wah_new_non_leading(domain);
    v = 7;
    wah_non_leading_SA_add_scalar(0, nld, &v);
    BPT_KEYS(n2)[1] = 5;
    BPT_POINTERS(n2)[1] = cache.seen(domain) + 1;
    cache.add(domain,
              cache.seen(domain),
              nld,
              sizeof(struct wah_bpt_non_leading_data),
              &wah_non_leading_cache_handler );

    wah_leading_repair(domain, n1, n2);

    struct wah_bpt_leading_data *new_ld = 
            cache.get(domain,
                      BPT_LEADING(n2) - 1,
                      &wah_leading_cache_handler );

    uint32_t R_size, *R = NULL;
    R_size = wah_get_ints(new_ld->B, &R);
    TEST_ASSERT_EQUAL(3,  R_size);
    // Wah is 1-based 
    TEST_ASSERT_EQUAL(2,  R[0] - 1);
    TEST_ASSERT_EQUAL(4,  R[1] - 1);
    TEST_ASSERT_EQUAL(5,  R[2] - 1);

    free(R);

    cache.destroy();
}
//}}}


void test_ints_to_wah(void)
{
    uint32_t i, size = 100;
    uint32_t *D = (uint32_t *)calloc(size,sizeof(uint32_t));


    D[0] = 5;
    D[1] = 6;
    D[2] = 13;
    D[3] = 14;
    D[4] = 15;
    D[5] = 100;
    D[6] = 101;
    D[7] = 1000;

    // 00000110000001110000000000000000 101122048
    // 10000000000000000000000000000010 2147483650
    // 00000001100000000000000000000000 25165824
    // 10000000000000000000000000011100 2147483676
    // 00000000100000000000000000000000 8388608

    uint8_t *w = uints_to_wah(D, 8);

    TEST_ASSERT_EQUAL(5, WAH_LEN(w));
    uint32_t v;
    get_wah_i(w, &v, 32, 0);
    TEST_ASSERT_EQUAL(bin_char_to_int("00000110000001110000000000000000"), v);
    get_wah_i(w, &v, 32, 1);
    TEST_ASSERT_EQUAL(bin_char_to_int("10000000000000000000000000000010"), v);
    get_wah_i(w, &v, 32, 2);
    TEST_ASSERT_EQUAL(bin_char_to_int("00000001100000000000000000000000"), v);
    get_wah_i(w, &v, 32, 3);
    TEST_ASSERT_EQUAL(bin_char_to_int("10000000000000000000000000011100"), v);
    get_wah_i(w, &v, 32, 4);
    TEST_ASSERT_EQUAL(bin_char_to_int("00000000100000000000000000000000"), v);

    free(w);
    w = NULL;

    for (i = 0; i < size; i+=2) {
        D[i] = rand();
        D[i+1] = D[i] + 30;
    }

    qsort(D, size, sizeof(uint32_t), uint32_t_cmp);

    w = uints_to_wah(D, size);

    uint32_t R_size, *R = NULL;
    R_size = wah_get_ints(w, &R);

    TEST_ASSERT_EQUAL(size, R_size);

    for (i = 0; i < size; ++i) 
        TEST_ASSERT_EQUAL(D[i], R[i]);

    free(w);
    free(R);
}
