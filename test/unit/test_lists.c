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

//{{{void test_fifo_q(void)
void test_fifo_q(void)
{
    int V[5] = {1,2,3,4,5};

    struct fifo_q *q = NULL;
    int *r = (int *)fifo_q_peek(q);
    TEST_ASSERT_EQUAL(NULL, r);

    fifo_q_push(&q, V);
    r = (int *)fifo_q_peek(q);
    TEST_ASSERT_EQUAL(V[0], *r);

    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[0], *r);
    TEST_ASSERT_EQUAL(NULL, q);
    r = (int *)fifo_q_peek(q);
    TEST_ASSERT_EQUAL(NULL, r);

    int v = 0;
    fifo_q_push(&q, V + v++);
    fifo_q_push(&q, V + v++);
    fifo_q_push(&q, V + v++);
    fifo_q_push(&q, V + v++);
    fifo_q_push(&q, V + v++);

    r = (int *)fifo_q_peek(q);
    TEST_ASSERT_EQUAL(V[4], *r);

    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[--v], *r);
    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[--v], *r);
    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[--v], *r);
    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[--v], *r);
    r = (int *)fifo_q_pop(&q);
    TEST_ASSERT_EQUAL(V[--v], *r);
    TEST_ASSERT_EQUAL(NULL, q);
}
//}}}

//{{{ void test_pointer_uint_pair(void)
void test_pointer_uint_pair(void)
{

    int V[10] = {1,2,3,4};

    struct ordered_set *os =
            ordered_set_init(3,
                             pointer_uint_pair_sort_element_cmp,
                             pointer_uint_pair_search_element_cmp,
                             pointer_uint_pair_search_key_cmp);

    struct pointer_uint_pair *p = (struct pointer_uint_pair *)
            malloc(sizeof(struct pointer_uint_pair));
    p->pointer = V+3;
    p->uint = 3;

    struct pointer_uint_pair *r = (struct pointer_uint_pair *)
            ordered_set_add(os, p);

    p = (struct pointer_uint_pair *)
            malloc(sizeof(struct pointer_uint_pair));
    p->pointer = V+1;
    p->uint = 1;
    r = (struct pointer_uint_pair *) ordered_set_add(os, p);

    p = (struct pointer_uint_pair *)
            malloc(sizeof(struct pointer_uint_pair));
    p->pointer = V;
    p->uint = 0;
    r = (struct pointer_uint_pair *) ordered_set_add(os, p);

    r = (struct pointer_uint_pair *) ordered_set_get(os, V);
    TEST_ASSERT_EQUAL(0,r->uint);
    r = (struct pointer_uint_pair *) ordered_set_get(os, V+1);
    TEST_ASSERT_EQUAL(1,r->uint);
    r = (struct pointer_uint_pair *) ordered_set_get(os, V+3);
    TEST_ASSERT_EQUAL(3,r->uint);
    r = (struct pointer_uint_pair *) ordered_set_get(os, V+2);
    TEST_ASSERT_EQUAL(NULL,r);
}
//}}}

//{{{ void test_pointer_uint_pair(void)
void test_uint_offset_size_pair(void)
{
    struct ordered_set *os =
            ordered_set_init(3,
                             uint_offset_size_pair_sort_element_cmp,
                             uint_offset_size_pair_search_element_cmp,
                             uint_offset_size_pair_search_key_cmp);

    struct uint_offset_size_pair *p = (struct uint_offset_size_pair *)
            malloc(sizeof(struct uint_offset_size_pair));
    p->uint = 1;
    p->offset = 50;
    struct uint_offset_size_pair *r = (struct uint_offset_size_pair *)
            ordered_set_add(os, p);

    p = (struct uint_offset_size_pair *)
            malloc(sizeof(struct uint_offset_size_pair));
    p->uint = 5;
    p->offset = 200;
    r = (struct uint_offset_size_pair *) ordered_set_add(os, p);

    p = (struct uint_offset_size_pair *)
            malloc(sizeof(struct uint_offset_size_pair));
    p->uint = 2;
    p->offset = 100;
    r = (struct uint_offset_size_pair *) ordered_set_add(os, p);

    p = (struct uint_offset_size_pair *)
            malloc(sizeof(struct uint_offset_size_pair));
    p->uint = 4;
    p->offset = 175;
    r = (struct uint_offset_size_pair *) ordered_set_add(os, p);

    p = (struct uint_offset_size_pair *)
            malloc(sizeof(struct uint_offset_size_pair));
    p->uint = 3;
    p->offset = 150;
    r = (struct uint_offset_size_pair *) ordered_set_add(os, p);


    uint32_t s = 1;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(50,r->offset);

    s = 2;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(100,r->offset);

    s = 3;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(150,r->offset);

    s = 4;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(175,r->offset);

    s = 5;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(200,r->offset);

    s = 7;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(NULL, r);

    s = 0;
    r = (struct uint_offset_size_pair *) ordered_set_get(os, &s);
    TEST_ASSERT_EQUAL(NULL, r);
}
//}}}

//{{{void test_byte_array(void)
void test_byte_array(void)
{
    struct byte_array *ba = byte_array_init(3 * sizeof(uint32_t));
    TEST_ASSERT_EQUAL(0, ba->num);
    TEST_ASSERT_EQUAL(3 * sizeof(uint32_t), ba->size);

    uint64_t A[3] = {2,4,6};
    uint32_t B[3] = {1,2,3};

    byte_array_append(ba, A, 3*sizeof(uint64_t));
    TEST_ASSERT_EQUAL(3*sizeof(uint64_t), ba->num);

    byte_array_append(ba, B, 3*sizeof(uint32_t));
    TEST_ASSERT_EQUAL(3*sizeof(uint64_t) + 3*sizeof(uint32_t), ba->num);

    uint64_t *A_r = (uint64_t *)(ba->data);
    TEST_ASSERT_EQUAL(A[0], A_r[0]);
    TEST_ASSERT_EQUAL(A[1], A_r[1]);
    TEST_ASSERT_EQUAL(A[2], A_r[2]);

    uint32_t *B_r = (uint32_t *)(ba->data + 3*sizeof(uint64_t));
    TEST_ASSERT_EQUAL(B[0], B_r[0]);
    TEST_ASSERT_EQUAL(B[1], B_r[1]);
    TEST_ASSERT_EQUAL(B[2], B_r[2]);

    byte_array_append_zeros(ba, 5*sizeof(uint32_t));
    byte_array_append(ba, B, 3*sizeof(uint32_t));
    B_r = (uint32_t *)(ba->data + 3*sizeof(uint64_t));
    TEST_ASSERT_EQUAL(0,  B_r[3]);
    TEST_ASSERT_EQUAL(0,  B_r[4]);
    TEST_ASSERT_EQUAL(0,  B_r[5]);
    TEST_ASSERT_EQUAL(0,  B_r[6]);
    TEST_ASSERT_EQUAL(0,  B_r[7]);
    TEST_ASSERT_EQUAL(B[0], B_r[8]);
    TEST_ASSERT_EQUAL(B[1], B_r[9]);
    TEST_ASSERT_EQUAL(B[2], B_r[10]);

    byte_array_destory(&ba);
    TEST_ASSERT_EQUAL(NULL, ba);
}
//}}}

//{{{void test_indexed_list(void)
void test_indexed_list(void)
{

    struct offset_size_pair p, *r;

    struct indexed_list *il =
            indexed_list_init(5,
                              sizeof(struct offset_size_pair));
    uint32_t i;
    for (i = 0; i < 5; ++i) {
        p.offset = i;
        p.size = i*2;
        TEST_ASSERT_EQUAL(0, indexed_list_add(il, i, &p));
    }
   for (i = 0; i < 5; ++i) {
        r = (struct offset_size_pair*)indexed_list_get(il, i);
        TEST_ASSERT_EQUAL(i, r->offset);
        TEST_ASSERT_EQUAL(i*2, r->size);
    }

    p.offset = 100;
    p.size = 200;
    TEST_ASSERT_EQUAL(1, indexed_list_add(il, 100, &p));

    r = (struct offset_size_pair*)indexed_list_get(il, 100);
    TEST_ASSERT_EQUAL(100, r->offset);
    TEST_ASSERT_EQUAL(200, r->size);

    for (i = 0; i < 5; ++i) {
        r = (struct offset_size_pair*)indexed_list_get(il, i);
        TEST_ASSERT_EQUAL(i, r->offset);
        TEST_ASSERT_EQUAL(i*2, r->size);
    }

    indexed_list_destroy(&il);
    TEST_ASSERT_EQUAL(NULL, il);
}
//}}}

//{{{void test_cc_hash(void)
void test_cc_hash(void)
{

    struct cc_hash *hash = cc_hash_init(2500);

    uint32_t size = 1000;
    uint32_t K[size],V[size];

    uint32_t i;

    for (i = 0; i < size; ++i){
        V[i] = i * 3;
        K[i] = rand();
        int r = cc_hash_add(hash, K[i], V+i);
    }

    for (i = 0; i < size; ++i){
        uint32_t *r = (uint32_t *)cc_hash_get(hash, K[i]);
        TEST_ASSERT_EQUAL(V[i], *r);
    }

    for (i = 0; i < size; ++i){
        uint32_t *r = (uint32_t *)cc_hash_remove(hash, K[i]);
        TEST_ASSERT_EQUAL(V[i], *r);
        r = (uint32_t *)cc_hash_remove(hash, K[i]);
        TEST_ASSERT_EQUAL(NULL, r);
    }

    cc_hash_destroy(&hash);
}
//}}}

//{{{void test_lru_cache(void)
void test_lru_cache(void)
{
    struct lru_cache *lruc = lru_cache_init(5);

    int V[10] = {2,4,6,8,10,12,14,16,18,20};

    lru_cache_add(lruc, 1, V);

    TEST_ASSERT_EQUAL(V, lruc->head->value);
    TEST_ASSERT_EQUAL(V, lruc->tail->value);

    int *r = (int *)lru_cache_get(lruc, 1);

    TEST_ASSERT_EQUAL(V[0], *r);
    TEST_ASSERT_EQUAL(NULL, lru_cache_get(lruc, 2));

    lru_cache_add(lruc, 2, V+1);

    TEST_ASSERT_EQUAL(V, lruc->head->value);
    TEST_ASSERT_EQUAL(V+1, lruc->tail->value);

    r = (int *)lru_cache_get(lruc, 1);

    TEST_ASSERT_EQUAL(V+1, lruc->head->value);
    TEST_ASSERT_EQUAL(V, lruc->tail->value);

    lru_cache_add(lruc, 3, V+2);
    lru_cache_add(lruc, 4, V+3);
    lru_cache_add(lruc, 5, V+4);
    lru_cache_add(lruc, 6, V+5);

    TEST_ASSERT_EQUAL(NULL, lru_cache_get(lruc, 2));
    r = (int *)lru_cache_get(lruc, 1);
    TEST_ASSERT_EQUAL(V[0], *r);
    r = (int *)lru_cache_get(lruc, 3);
    TEST_ASSERT_EQUAL(V[2], *r);
    r = (int *)lru_cache_get(lruc, 4);
    TEST_ASSERT_EQUAL(V[3], *r);
    r = (int *)lru_cache_get(lruc, 5);
    TEST_ASSERT_EQUAL(V[4], *r);
    r = (int *)lru_cache_get(lruc, 6);
    TEST_ASSERT_EQUAL(V[5], *r);

    lru_cache_destroy(&lruc);
}
///}}}
