#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "util.h"
#include "bpt.h"
#include "ll.h"
#include "wah.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_uint32_t_ll_append(void)
void test_uint32_t_ll_append(void)
{
    struct uint32_t_ll *ll = NULL;

    uint32_t_ll_append(&ll, 1);
    uint32_t_ll_append(&ll, 2);
    uint32_t_ll_append(&ll, 5);

    TEST_ASSERT_EQUAL(1, ll->head->val);
    TEST_ASSERT_EQUAL(2, ll->head->next->val);
    TEST_ASSERT_EQUAL(5, ll->head->next->next->val);

    uint32_t_ll_free((void **)&ll);

    TEST_ASSERT_EQUAL(NULL, ll);
}
//}}}

//{{{void test_uint32_t_ll_remove(void)
void test_uint32_t_ll_remove(void)
{
    struct uint32_t_ll *ll = NULL;

    uint32_t_ll_append(&ll, 1);
    uint32_t_ll_remove(&ll, 1);

    TEST_ASSERT_EQUAL(NULL, ll);


    uint32_t_ll_append(&ll, 1);
    uint32_t_ll_append(&ll, 2);

    uint32_t_ll_remove(&ll, 1);
    TEST_ASSERT_EQUAL(2, ll->head->val);
    uint32_t_ll_remove(&ll, 1);
    TEST_ASSERT_EQUAL(2, ll->head->val);
    uint32_t_ll_remove(&ll, 2);
    TEST_ASSERT_EQUAL(NULL, ll);


    uint32_t_ll_append(&ll, 1);
    uint32_t_ll_append(&ll, 2);
    uint32_t_ll_append(&ll, 5);

    uint32_t_ll_remove(&ll, 2);
    TEST_ASSERT_EQUAL(1, ll->head->val);
    TEST_ASSERT_EQUAL(5, ll->head->next->val);
    uint32_t_ll_remove(&ll, 5);
    uint32_t_ll_remove(&ll, 1);
    TEST_ASSERT_EQUAL(NULL, ll);

    uint32_t_ll_free((void **)&ll);

    TEST_ASSERT_EQUAL(NULL, ll);
}
//}}}

//{{{void test_uint32_t_ll_contains(void)
void test_uint32_t_ll_contains(void)
{
    struct uint32_t_ll *ll = NULL;

    TEST_ASSERT_EQUAL(0, uint32_t_ll_contains(ll, 1));
    
    uint32_t_ll_append(&ll, 1);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ll, 1));

    uint32_t_ll_append(&ll, 1);
    TEST_ASSERT_EQUAL(2, uint32_t_ll_contains(ll, 1));

    uint32_t_ll_remove(&ll, 1);
    TEST_ASSERT_EQUAL(0, uint32_t_ll_contains(ll, 1));


    uint32_t_ll_append(&ll, 1);
    uint32_t_ll_append(&ll, 2);
    uint32_t_ll_append(&ll, 5);

    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ll, 5));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ll, 1));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ll, 2));

    TEST_ASSERT_EQUAL(0, uint32_t_ll_contains(ll, 7));

    uint32_t_ll_free((void **)&ll);

}
//}}}

//{{{void test_uint32_t_ll_leading_serialize_to_wah(void)
void test_uint32_t_ll_leading_serialize_to_wah(void)
{
    WAH_SIZE = 32;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct uint32_t_ll *ll = NULL;
    uint32_t i, size = 10;
    uint32_t *D = (uint32_t *)calloc(size,sizeof(uint32_t));

    for (i = 0; i < size; ++i) {
        D[i] = rand();
        uint32_t_ll_append(&ll, D[i]);
    }

    struct uint32_t_ll_bpt_leading_data *ll_ld =  
        (struct uint32_t_ll_bpt_leading_data *)
        calloc(1,sizeof(struct uint32_t_ll_bpt_leading_data));

    ll_ld->B = ll;

    void *serialized = NULL;

    uint64_t s_size = 
            uint32_t_ll_leading_serialize_to_wah(ll_ld,
                                                 &serialized);

    void *deserialized = NULL;
    uint64_t d_size = wah_8_leading_deserialize(serialized,
                                                s_size,
                                                &deserialized);
    struct wah_8_bpt_leading_data *wah_ld =  
            (struct wah_8_bpt_leading_data *)deserialized;

    uint32_t R_size, *R = NULL;
    R_size = wah_get_ints_8(wah_ld->B, &R);

    TEST_ASSERT_EQUAL(size, R_size);

    qsort(D, size, sizeof(uint32_t), uint32_t_cmp);

    for (i = 0; i < size; ++i)
        TEST_ASSERT_EQUAL(D[i], R[i]);

    free(D);
    free(wah_ld->B);
    free(wah_ld);
    uint32_t_ll_free((void **)&(ll_ld->B));
    free(ll_ld);
}
//}}}

//{{{void test_uint32_t_ll_non_leading_serialize_to_wah(void)
void test_uint32_t_ll_non_leading_serialize_to_wah(void)
{
    WAH_SIZE = 32;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct uint32_t_ll *ll_SA = NULL, *ll_SE = NULL;
    uint32_t i, size = 10;
    uint32_t *SA = (uint32_t *)calloc(size,sizeof(uint32_t));
    uint32_t *SE = (uint32_t *)calloc(size,sizeof(uint32_t));

    for (i = 0; i < size; ++i) {
        SA[i] = rand();
        uint32_t_ll_append(&ll_SA, SA[i]);
        SE[i] = rand();
        uint32_t_ll_append(&ll_SE, SE[i]);
    }

    struct uint32_t_ll_bpt_non_leading_data *ll_nld =  
        (struct uint32_t_ll_bpt_non_leading_data *)
        calloc(1,sizeof(struct uint32_t_ll_bpt_non_leading_data));

    ll_nld->SA = ll_SA;
    ll_nld->SE = ll_SE;

    void *serialized = NULL;

    uint64_t s_size = 
            uint32_t_ll_non_leading_serialize_to_wah(ll_nld,
                                                     &serialized);

    void *deserialized = NULL;
    uint64_t d_size = wah_8_non_leading_deserialize(serialized,
                                                    s_size,
                                                    &deserialized);
    struct wah_8_bpt_non_leading_data *wah_nld =  
            (struct wah_8_bpt_non_leading_data *)deserialized;

    uint32_t R_size, *R = NULL;
    R_size = wah_get_ints_8(wah_nld->SA, &R);

    TEST_ASSERT_EQUAL(size, R_size);

    qsort(SA, size, sizeof(uint32_t), uint32_t_cmp);
    for (i = 0; i < size; ++i)
        TEST_ASSERT_EQUAL(SA[i], R[i]);

    free(R);
    R = NULL;

    R_size = wah_get_ints_8(wah_nld->SE, &R);

    TEST_ASSERT_EQUAL(size, R_size);

    qsort(SE, size, sizeof(uint32_t), uint32_t_cmp);
    for (i = 0; i < size; ++i)
        TEST_ASSERT_EQUAL(SE[i], R[i]);

    free(SA);
    free(SE);

    free(wah_nld->SA);
    free(wah_nld->SE);
    free(wah_nld);

    uint32_t_ll_free((void **)&(ll_nld->SA));
    uint32_t_ll_free((void **)&(ll_nld->SE));
    free(ll_nld);
}
//}}}

//{{{void test_uint32_t_ll_non_leading_serialize_to_wah_corners(void)
void test_uint32_t_ll_non_leading_serialize_to_wah_corners(void)
{
    WAH_SIZE = 32;
    WAH_MAX_FILL_WORDS = (1<<(WAH_SIZE-1)) - 1;

    struct uint32_t_ll_bpt_non_leading_data *ll_nld =  
        (struct uint32_t_ll_bpt_non_leading_data *)
        calloc(1,sizeof(struct uint32_t_ll_bpt_non_leading_data));

    ll_nld->SA = NULL;
    ll_nld->SE = NULL;

    void *serialized = NULL;

    uint64_t s_size = 
            uint32_t_ll_non_leading_serialize_to_wah(ll_nld,
                                                     &serialized);
    void *deserialized = NULL;
    uint64_t d_size = wah_8_non_leading_deserialize(serialized,
                                                    s_size,
                                                    &deserialized);
    struct wah_8_bpt_non_leading_data *wah_nld =  
            (struct wah_8_bpt_non_leading_data *)deserialized;

    TEST_ASSERT_EQUAL(NULL, wah_nld->SA);
    TEST_ASSERT_EQUAL(NULL, wah_nld->SE);

    free(serialized);
    free(deserialized);

    free(ll_nld);
    ll_nld = NULL;

    s_size = uint32_t_ll_non_leading_serialize_to_wah(ll_nld,
                                                     &serialized);
    TEST_ASSERT_EQUAL(0, s_size);
    TEST_ASSERT_EQUAL(NULL, serialized);
 

}
//}}}
