#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "lists.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_unordered_list_init(void)
void test_unordered_list_init(void)
{
    struct unordered_list *ul = unordered_list_init(10);

    TEST_ASSERT_EQUAL(ul->num, 0);
    TEST_ASSERT_EQUAL(ul->size, 10);

    unordered_list_destroy(&ul);

    TEST_ASSERT_EQUAL(ul, NULL);
}
//}}}

//{{{void test_unordered_list_add(void)
void test_unordered_list_add(void)
{
    struct unordered_list *ul = unordered_list_init(10);

    int i, V[20];

    for (i = 0; i < 20; ++i)
        V[i] = (i+1)*2; 

    for (i = 0; i < 9; ++i) {
        TEST_ASSERT_EQUAL(i, unordered_list_add(ul, (void *)(V + i)));
        TEST_ASSERT_EQUAL(V + i, ul->data[i]);
        TEST_ASSERT_EQUAL(i + 1, ul->num);
        TEST_ASSERT_EQUAL(10, ul->size);
    }
    for (; i < 19; ++i) {
        TEST_ASSERT_EQUAL(i, unordered_list_add(ul, (void *)(V + i)));
        TEST_ASSERT_EQUAL(V + i, ul->data[i]);
        TEST_ASSERT_EQUAL(i + 1, ul->num);
        TEST_ASSERT_EQUAL(20, ul->size);
    }

    unordered_list_destroy(&ul);
}
//}}}

//{{{void test_unordered_list_get(void)
void test_unordered_list_get(void)
{
    struct unordered_list *ul = unordered_list_init(10);

    int i, V[20];

    for (i = 0; i < 20; ++i)
        V[i] = (i+1)*2; 

    for (i = 0; i < 20; ++i) 
        TEST_ASSERT_EQUAL(i, unordered_list_add(ul, (void *)(V + i)));

    for (i = 0; i < 20; ++i)  {
        int *r = (int *)unordered_list_get(ul, i);
        TEST_ASSERT_EQUAL(V[i], (int)(*r));
    }

    void *r = unordered_list_get(ul, 5000);
    TEST_ASSERT_EQUAL(NULL, r);
    
    unordered_list_destroy(&ul);
}
//}}}

//{{{void test_ordered_set_add(void)
void test_ordered_set_add(void)
{
    struct ordered_set *os = ordered_set_init(3,
                                              str_uint_pair_sort_element_cmp,
                                              str_uint_pair_search_element_cmp,
                                              str_uint_pair_search_key_cmp);
    struct str_uint_pair *p = (struct str_uint_pair *)
            malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("one");

    struct str_uint_pair *r = (struct str_uint_pair *)
            ordered_set_add(os, p);

    TEST_ASSERT_EQUAL(0, r->uint);


    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("two");

    r = (struct str_uint_pair *) ordered_set_add(os, p);
    TEST_ASSERT_EQUAL(1, r->uint);

    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("one");

    r = (struct str_uint_pair *) ordered_set_add(os, p);
    TEST_ASSERT_EQUAL(0, r->uint);

    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("three");

    r = (struct str_uint_pair *) ordered_set_add(os, p);
    TEST_ASSERT_EQUAL(2, r->uint);

    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("four");

    r = (struct str_uint_pair *) ordered_set_add(os, p);
    TEST_ASSERT_EQUAL(3, r->uint);



}
//}}}

//{{{void test_ordered_set_get(void)
void test_ordered_set_get(void)
{
    struct ordered_set *os = ordered_set_init(3,
                                              str_uint_pair_sort_element_cmp,
                                              str_uint_pair_search_element_cmp,
                                              str_uint_pair_search_key_cmp);
    struct str_uint_pair *p = (struct str_uint_pair *)
            malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("one");
    struct str_uint_pair *r = (struct str_uint_pair *)ordered_set_add(os, p);


    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("three");
    r = (struct str_uint_pair *)ordered_set_add(os, p);

    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("four");
    r = (struct str_uint_pair *)ordered_set_add(os, p);

    p = (struct str_uint_pair *) malloc(sizeof(struct str_uint_pair));
    p->uint = os->num;
    p->str = strdup("two");
    r = (struct str_uint_pair *)ordered_set_add(os, p);

    char *one = "one";
    r = (struct str_uint_pair *) ordered_set_get(os, one);
    TEST_ASSERT_EQUAL(0, r->uint);

    char *two = "two";
    r = (struct str_uint_pair *) ordered_set_get(os, two);
    TEST_ASSERT_EQUAL(3, r->uint);

    char *three = "three";
    r = (struct str_uint_pair *) ordered_set_get(os, three);
    TEST_ASSERT_EQUAL(1, r->uint);

    char *four = "four";
    r = (struct str_uint_pair *) ordered_set_get(os, four);
    TEST_ASSERT_EQUAL(2, r->uint);
}
//}}}
