#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "bpt.h"
#include "giggle.h"
#include "data_reg.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_data_reg_init(void)
void test_data_reg_init(void)
{
    struct data_reg *d = data_reg_init(10);

    TEST_ASSERT_EQUAL(d->num_data, 0);
    TEST_ASSERT_EQUAL(d->data_size, 10);

    data_reg_destroy(&d);

    TEST_ASSERT_EQUAL(d, NULL);
}
//}}}

//{{{void test_data_reg_add(void)
void test_data_reg_add(void)
{
    struct data_reg *d = data_reg_init(10);

    int i, V[20];

    for (i = 0; i < 20; ++i)
        V[i] = (i+1)*2; 

    for (i = 0; i < 9; ++i) {
        TEST_ASSERT_EQUAL(i, data_reg_add(d, (void *)(V + i)));
        TEST_ASSERT_EQUAL(V + i, d->data[i]);
        TEST_ASSERT_EQUAL(i + 1, d->num_data);
        TEST_ASSERT_EQUAL(10, d->data_size);
    }
    for (; i < 19; ++i) {
        TEST_ASSERT_EQUAL(i, data_reg_add(d, (void *)(V + i)));
        TEST_ASSERT_EQUAL(V + i, d->data[i]);
        TEST_ASSERT_EQUAL(i + 1, d->num_data);
        TEST_ASSERT_EQUAL(20, d->data_size);
    }

    data_reg_destroy(&d);
}
//}}}

//{{{void test_data_reg_get(void)
void test_data_reg_get(void)
{
    struct data_reg *dr = data_reg_init(10);

    int i, V[20];

    for (i = 0; i < 20; ++i)
        V[i] = (i+1)*2; 

    for (i = 0; i < 20; ++i) 
        TEST_ASSERT_EQUAL(i, data_reg_add(dr, (void *)(V + i)));

    for (i = 0; i < 20; ++i)  {
        int *r = (int *)data_reg_get(dr, i);
        TEST_ASSERT_EQUAL(V[i], (int)(*r));
    }
    
    data_reg_destroy(&dr);
}
//}}}
