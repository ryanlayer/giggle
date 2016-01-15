#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "unity.h"
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "bpt.h"
#include "lists.h"
#include "disk_store.h"

//{{{ int uint32_t_cmp(const void *a, const void *b)
static int uint32_t_cmp(const void *a, const void *b)
{
    uint32_t _a = *((uint32_t *)a), _b = *((uint32_t *)b);

    if (_a < _b)
        return -1;
    else if (_a > _b)
        return 1;
    else

    return 0;
}
//}}}

void setUp(void)
{
}

void tearDown(void)
{
}

//{{{ void test_b_search(void)
void test_b_search(void)
{
    uint32_t D[10] = {2, 3, 4, 6, 8, 10, 12, 14, 16, 18};
    uint32_t A[20] = {0, // 0
                      0, // 1
                      0, // 2
                      1, // 3
                      2, // 4
                      3, // 5
                      3, // 6
                      4, // 7
                      4, // 8
                      5, // 9
                      5, // 10
                      6, // 11
                      6, // 12
                      7, // 13
                      7, // 14
                      8, // 15
                      8, // 16
                      9, // 17
                      9, // 18
                      10}; // 19

    int i;
    for (i = 0; i < 20; ++i)
        TEST_ASSERT_EQUAL(A[i], b_search(i, D, 10));
}
//}}}

//{{{void test_bpt_node_macros(void)
void test_bpt_node_macros(void)
{
    struct bpt_node *node = (struct bpt_node *)malloc(sizeof(struct bpt_node));

    uint32_t data[17] = {1,  // id
                         0,  // parent
                         0,  // is_leaf
                         0,  // leading
                         0,  // next
                         2,  // num_keys
                         2, 3, 0, 0, 0,  // keys (ORDER = 4)
                         5, 6, 7, 0, 0, 0}; //pointers (ORDER = 4)
    node->data = data;

    TEST_ASSERT_EQUAL(node->data[0], BPT_ID(node));
    TEST_ASSERT_EQUAL(node->data[1], BPT_PARENT(node));
    TEST_ASSERT_EQUAL(node->data[2], BPT_IS_LEAF(node));
    TEST_ASSERT_EQUAL(node->data[3], BPT_LEADING(node));
    TEST_ASSERT_EQUAL(node->data[4], BPT_NEXT(node));
    TEST_ASSERT_EQUAL(node->data[5], BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(node->data + 6, BPT_KEYS(node));
    TEST_ASSERT_EQUAL(node->data + 6 + ORDER + 1, BPT_POINTERS(node));

    TEST_ASSERT_EQUAL(2, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[3]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[4]);

    TEST_ASSERT_EQUAL(5, BPT_POINTERS(node)[0]);
    TEST_ASSERT_EQUAL(6, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(7, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[3]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[4]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[5]);

    BPT_KEYS(node)[3] = 10;
    TEST_ASSERT_EQUAL(10, BPT_KEYS(node)[3]);

    BPT_ID(node) = 2;
    TEST_ASSERT_EQUAL(2, BPT_ID(node));

    ORDER=5;

    uint32_t data2[19] = {1,
                          0,  // parent
                          0,  // is_leaf
                          0,  // leading
                          0,  // next
                          2,  // num_keys
                          2, 3, 0, 0, 0, 0,  // keys (ORDER = 4)
                          5, 6, 7, 0, 0, 0, 0}; //pointers (ORDER = 4)

    node->data = data2;

    TEST_ASSERT_EQUAL(node->data[0], BPT_ID(node));
    TEST_ASSERT_EQUAL(node->data[1], BPT_PARENT(node));
    TEST_ASSERT_EQUAL(node->data[2], BPT_IS_LEAF(node));
    TEST_ASSERT_EQUAL(node->data[3], BPT_LEADING(node));
    TEST_ASSERT_EQUAL(node->data[4], BPT_NEXT(node));
    TEST_ASSERT_EQUAL(node->data[5], BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(node->data + 6, BPT_KEYS(node));
    TEST_ASSERT_EQUAL(node->data + 6 + ORDER + 1, BPT_POINTERS(node));

    TEST_ASSERT_EQUAL(2, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[3]);
    TEST_ASSERT_EQUAL(0, BPT_KEYS(node)[4]);

    TEST_ASSERT_EQUAL(5, BPT_POINTERS(node)[0]);
    TEST_ASSERT_EQUAL(6, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(7, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[3]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[4]);
    TEST_ASSERT_EQUAL(0, BPT_POINTERS(node)[5]);

    free(node);
}
//}}}

//{{{ void test_bpt_new_node(void)
void test_bpt_new_node(void)
{
    struct bpt_node *n = bpt_new_node();
    TEST_ASSERT_EQUAL(1, BPT_ID(n));
    //TEST_ASSERT_EQUAL(1, cache.seen(cache.cache));
    TEST_ASSERT_EQUAL(1, cache.seen(cache.cache));

    //struct bpt_node *r = lru_cache_get(cache, 1);
    struct bpt_node *r = cache.get(cache.cache, 1);
    TEST_ASSERT_EQUAL(n, r);

    //r = lru_cache_get(cache, 2);
    r = cache.get(cache.cache, 2);
    TEST_ASSERT_EQUAL(NULL, r);

    struct bpt_node *l = bpt_new_node();
    TEST_ASSERT_EQUAL(2, BPT_ID(l));
    TEST_ASSERT_EQUAL(2, cache.seen(cache.cache));
    
    r = cache.get(cache.cache, 2);
    TEST_ASSERT_EQUAL(l, r);

    r = cache.get(cache.cache, 3);
    TEST_ASSERT_EQUAL(NULL, r);

    cache.destroy(&(cache.cache));
}
//}}}

//{{{ void test_bpt_find_leaf(void)
void test_bpt_find_leaf(void)
{
    struct bpt_node *root = bpt_new_node();
    BPT_NUM_KEYS(root) = 1;
    BPT_KEYS(root)[0] = 9;

    struct bpt_node *n1 = bpt_new_node();
    BPT_NUM_KEYS(n1) = 1;
    BPT_KEYS(n1)[0] = 5;

    struct bpt_node *l1 = bpt_new_node();
    BPT_IS_LEAF(l1) = 1;
    BPT_NUM_KEYS(l1) = 4;
    BPT_KEYS(l1)[0] = 1;
    BPT_KEYS(l1)[1] = 2;
    BPT_KEYS(l1)[2] = 3;
    BPT_KEYS(l1)[3] = 4;

    struct bpt_node *l2 = bpt_new_node();
    BPT_IS_LEAF(l2) = 1;
    BPT_NUM_KEYS(l2) = 4;
    BPT_KEYS(l2)[0] = 5;
    BPT_KEYS(l2)[1] = 6;
    BPT_KEYS(l2)[2] = 7;
    BPT_KEYS(l2)[3] = 8;

    struct bpt_node *l3 = bpt_new_node();
    BPT_IS_LEAF(l3) = 1;
    BPT_NUM_KEYS(l3) = 4;
    BPT_KEYS(l3)[0] = 9;
    BPT_KEYS(l3)[1] = 10;
    BPT_KEYS(l3)[2] = 11;
    BPT_KEYS(l3)[3] = 12;

    BPT_NEXT(l1) = BPT_ID(l2);
    BPT_NEXT(l2) = BPT_ID(l3);


    BPT_POINTERS(n1)[0] = BPT_ID(l1);
    BPT_POINTERS(n1)[1] = BPT_ID(l2);
    BPT_PARENT(l1) = BPT_ID(n1);
    BPT_PARENT(l2) = BPT_ID(n1);

    BPT_POINTERS(root)[0] = BPT_ID(n1);
    BPT_POINTERS(root)[1] = BPT_ID(l3);

    BPT_PARENT(n1) = BPT_ID(root);
    BPT_PARENT(l3) = BPT_ID(root);

    int i;
    for (i = 1; i <= 4; ++i)
        TEST_ASSERT_EQUAL(BPT_ID(l1),
                          bpt_find_leaf(BPT_ID(root), i));

    for (i = 5; i <= 8; ++i) 
        TEST_ASSERT_EQUAL(BPT_ID(l2),
                          bpt_find_leaf(BPT_ID(root), i));

    for (i = 9; i <= 12; ++i) 
        TEST_ASSERT_EQUAL(BPT_ID(l3),
                          bpt_find_leaf(BPT_ID(root), i));

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_bpt_split_node_non_root_parent_has_room(void)
void test_bpt_split_node_non_root_parent_has_room(void)
{
    ORDER = 4;
    
    /*
     * 9
     * 6
     * 1,2,3,4,5
     *
     * Then split
     *
     * 9
     * 3,6
     * 1,2 3,4,5
     */
    struct bpt_node *root = bpt_new_node();
    BPT_NUM_KEYS(root) = 1;
    BPT_KEYS(root)[0] = 9;

    struct bpt_node *n1 = bpt_new_node();
    BPT_NUM_KEYS(n1) = 1;
    BPT_KEYS(n1)[0] = 6;

    BPT_POINTERS(root)[0] = BPT_ID(n1);
    BPT_PARENT(n1) = BPT_ID(root);

    struct bpt_node *l1 = bpt_new_node();
    BPT_IS_LEAF(l1) = 1;
    BPT_NUM_KEYS(l1) = 5;
    BPT_KEYS(l1)[0] = 1;
    BPT_KEYS(l1)[1] = 2;
    BPT_KEYS(l1)[2] = 3;
    BPT_KEYS(l1)[3] = 4;
    BPT_KEYS(l1)[4] = 5;

    BPT_POINTERS(n1)[0] = BPT_ID(l1);
    BPT_PARENT(l1) = BPT_ID(n1);

    //int V[5] = {2,3,4,5,6};
    //int *V = (int *) malloc(5*sizeof(int));
    int *V_0 = (int *) malloc(sizeof(int));
    *V_0 = 2;
    int *V_1 = (int *) malloc(sizeof(int));
    *V_1 = 3;
    int *V_2 = (int *) malloc(sizeof(int));
    *V_2 = 4;
    int *V_3 = (int *) malloc(sizeof(int));
    *V_3 = 5;
    int *V_4 = (int *) malloc(sizeof(int));
    *V_4 = 6;

    // Put the data in the cache
    TEST_ASSERT_EQUAL(4, cache.seen(cache.cache) + 1);
    BPT_POINTERS(l1)[0] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_0, free_wrapper);

    BPT_POINTERS(l1)[1] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_1, free_wrapper);

    BPT_POINTERS(l1)[2] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_2, free_wrapper);

    BPT_POINTERS(l1)[3] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_3, free_wrapper);

    BPT_POINTERS(l1)[4] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_4, free_wrapper);



    uint32_t lo_id, hi_id;
    int split;
    uint32_t root_id = bpt_split_node(BPT_ID(root),
                                      BPT_ID(l1),
                                      &lo_id,
                                      &hi_id,
                                      &split,
                                      NULL);

    TEST_ASSERT_EQUAL(BPT_ID(l1), lo_id);
    TEST_ASSERT_EQUAL(BPT_NEXT(l1), hi_id);
    TEST_ASSERT_EQUAL(2, split);


    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(l1));
    struct bpt_node *hi_node = cache.get(cache.cache, hi_id);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(hi_node));
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(n1));
    TEST_ASSERT_EQUAL(3, BPT_KEYS(n1)[0]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(n1)[1]);
    TEST_ASSERT_EQUAL(lo_id, BPT_POINTERS(n1)[0]);
    TEST_ASSERT_EQUAL(hi_id, BPT_POINTERS(n1)[1]);

    TEST_ASSERT_EQUAL(lo_id, bpt_find_leaf(root_id, 1));
    TEST_ASSERT_EQUAL(lo_id, bpt_find_leaf(root_id, 2));
    TEST_ASSERT_EQUAL(hi_id, bpt_find_leaf(root_id, 3));
    TEST_ASSERT_EQUAL(hi_id, bpt_find_leaf(root_id, 4));
    TEST_ASSERT_EQUAL(hi_id, bpt_find_leaf(root_id, 5));

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_bpt_split_node_root(void)
void test_bpt_split_node_root(void)
{
    ORDER = 4;

    int *V_0 = (int *) malloc(sizeof(int));
    *V_0 = 2;
    int *V_1 = (int *) malloc(sizeof(int));
    *V_1 = 3;
    int *V_2 = (int *) malloc(sizeof(int));
    *V_2 = 4;
    int *V_3 = (int *) malloc(sizeof(int));
    *V_3 = 5;
    int *V_4 = (int *) malloc(sizeof(int));
    *V_4 = 6;

    /*
     * 1,2,3,4,5
     *
     * SPLIT
     *
     * 3
     * 1,2 3,4,5
     */

    struct bpt_node *root = bpt_new_node();
    BPT_IS_LEAF(root) = 1;
    BPT_NUM_KEYS(root) = 5;
    BPT_KEYS(root)[0] = 1;
    BPT_KEYS(root)[1] = 2;
    BPT_KEYS(root)[2] = 3;
    BPT_KEYS(root)[3] = 4;
    BPT_KEYS(root)[4] = 5;

    BPT_POINTERS(root)[0] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_0, free_wrapper);

    BPT_POINTERS(root)[1] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_1, free_wrapper);

    BPT_POINTERS(root)[2] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_2, free_wrapper);

    BPT_POINTERS(root)[3] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_3, free_wrapper);

    BPT_POINTERS(root)[4] = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, cache.seen(cache.cache) + 1, V_4, free_wrapper);

    uint32_t lo_id, hi_id;
    int split;
    uint32_t root_id = bpt_split_node(BPT_ID(root),
                                      BPT_ID(root),
                                      &lo_id,
                                      &hi_id,
                                      &split,
                                      NULL);

    TEST_ASSERT_EQUAL(BPT_ID(root), lo_id);
    TEST_ASSERT_EQUAL(BPT_NEXT(root), hi_id);
    TEST_ASSERT_EQUAL(2, split);


    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(root));
    TEST_ASSERT_EQUAL(root_id, BPT_PARENT(root));

    struct bpt_node *next_node = cache.get(cache.cache, BPT_NEXT(root));
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(next_node));
    TEST_ASSERT_EQUAL(root_id, BPT_PARENT(next_node));

    struct bpt_node *new_root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(new_root));
    TEST_ASSERT_EQUAL(3, BPT_KEYS(new_root)[0]);
    TEST_ASSERT_EQUAL(lo_id, BPT_POINTERS(new_root)[0]);
    TEST_ASSERT_EQUAL(hi_id, BPT_POINTERS(new_root)[1]);

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_bpt_insert(void)
void test_bpt_insert(void)
{
    /*
     * 2,3,4,5
     *
     * 4
     *  2,3 4,5,6
     *
     * 4,6
     *  2,3 4,5 6,7,8
     *
     * 4,6,8
     *  2,3 4,5 6,7 8,9,10
     *
     * 4,6,8,10
     *  2,3 4,5 6,7 8,9 10,11,12
     *
     * 8 
     *  4,6 8,10,12
     *   2,3 4,5 6,7 8,9 10,11 12,13,14
     */
    ORDER = 4;
 
    int *V_0 = (int *) malloc(sizeof(int));
    *V_0 = 1;
    int *V_1 = (int *) malloc(sizeof(int));
    *V_1 = 2;
    int *V_2 = (int *) malloc(sizeof(int));
    *V_2 = 3;
    int *V_3 = (int *) malloc(sizeof(int));
    *V_3 = 4;
    int *V_4 = (int *) malloc(sizeof(int));
    *V_4 = 5;
    int *V_5 = (int *) malloc(sizeof(int));
    *V_5 = 6;
    int *V_6 = (int *) malloc(sizeof(int));
    *V_6 = 7;
    int *V_7 = (int *) malloc(sizeof(int));
    *V_7 = 8;
    int *V_8 = (int *) malloc(sizeof(int));
    *V_8 = 9;
    int *V_9 = (int *) malloc(sizeof(int));
    *V_9 = 10;
    int *V_10 = (int *) malloc(sizeof(int));
    *V_10 = 11;
    int *V_11 = (int *) malloc(sizeof(int));
    *V_11 = 12;
    int *V_12 = (int *) malloc(sizeof(int));
    *V_12 = 13;

    //cache = lru_cache_init(10000);
    cache.cache = cache.init(10000, NULL);
   
    uint32_t leaf_id;
    int pos;
    uint32_t V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_0, free_wrapper);

    //2
    uint32_t root_id = bpt_insert(0, 2, V_id, &leaf_id, &pos);

    struct bpt_node *root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(root));
    TEST_ASSERT_EQUAL(root_id, leaf_id);
    TEST_ASSERT_EQUAL(0, pos);

    // 4
    //  2,3 4,5,6
    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_1, free_wrapper);
    root_id = bpt_insert(root_id, 3, V_id, &leaf_id, &pos);
    TEST_ASSERT_EQUAL(root_id, leaf_id);
    TEST_ASSERT_EQUAL(1, pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_2, free_wrapper);
    root_id = bpt_insert(root_id, 4, V_id, &leaf_id, &pos);
    TEST_ASSERT_EQUAL(root_id, leaf_id);
    TEST_ASSERT_EQUAL(2, pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_3, free_wrapper);
    root_id = bpt_insert(root_id, 5, V_id, &leaf_id, &pos);
    TEST_ASSERT_EQUAL(root_id, leaf_id);
    TEST_ASSERT_EQUAL(3, pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_4, free_wrapper);
    root_id = bpt_insert(root_id, 6, V_id, &leaf_id, &pos);
    TEST_ASSERT_EQUAL(2, pos);

    TEST_ASSERT_FALSE(root_id == leaf_id);
    root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));
    TEST_ASSERT_EQUAL(leaf_id, BPT_POINTERS(root)[1]);

    struct bpt_node *node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node));

    node = cache.get(cache.cache, BPT_NEXT(node));
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node));

    // 4,6
    //  2,3 4,5 6,7,8
    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_5, free_wrapper);
    root_id = bpt_insert(root_id, 7, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_6, free_wrapper);
    root_id = bpt_insert(root_id, 8, V_id, &leaf_id, &pos);

    root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(root)[1]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node));

    TEST_ASSERT_EQUAL(BPT_POINTERS(root)[1], BPT_NEXT(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node));

    TEST_ASSERT_EQUAL(BPT_POINTERS(root)[2], BPT_NEXT(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[2]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(6, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(7, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(8, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node));

    // 8
    //   4,6 8,10,12
    //     2,3 4,5 6,7 8,9 10,11 12,13,14
    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_7, free_wrapper);
    root_id = bpt_insert(root_id, 9, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_8, free_wrapper);
    root_id = bpt_insert(root_id, 10, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_9, free_wrapper);
    root_id = bpt_insert(root_id, 11, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_10, free_wrapper);
    root_id = bpt_insert(root_id, 12, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_11, free_wrapper);
    root_id = bpt_insert(root_id, 13, V_id, &leaf_id, &pos);

    V_id = cache.seen(cache.cache) + 1;
    cache.add(cache.cache, V_id, V_12, free_wrapper);
    root_id = bpt_insert(root_id, 14, V_id, &leaf_id, &pos);

    // 8
    //   4,6 8,10,12
    //     2,3 4,5 6,7 8,9 10,11 12,13,14
    root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(10, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(12, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_1 = cache.get(cache.cache, BPT_POINTERS(node)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_1));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(leaf_1)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(leaf_1)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_1));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_2 = cache.get(cache.cache, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_2));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(leaf_2)[0]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(leaf_2)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_2));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_3 = cache.get(cache.cache, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_3));
    TEST_ASSERT_EQUAL(6, BPT_KEYS(leaf_3)[0]);
    TEST_ASSERT_EQUAL(7, BPT_KEYS(leaf_3)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_3));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_4 = cache.get(cache.cache, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_4));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(leaf_4)[0]);
    TEST_ASSERT_EQUAL(9, BPT_KEYS(leaf_4)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_4));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_5 = cache.get(cache.cache, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_5));
    TEST_ASSERT_EQUAL(10, BPT_KEYS(leaf_5)[0]);
    TEST_ASSERT_EQUAL(11, BPT_KEYS(leaf_5)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_5));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_6 = cache.get(cache.cache, BPT_POINTERS(node)[3]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(leaf_6));
    TEST_ASSERT_EQUAL(12, BPT_KEYS(leaf_6)[0]);
    TEST_ASSERT_EQUAL(13, BPT_KEYS(leaf_6)[1]);
    TEST_ASSERT_EQUAL(14, BPT_KEYS(leaf_6)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_6));

    TEST_ASSERT_EQUAL(BPT_ID(leaf_2), BPT_NEXT(leaf_1));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_3), BPT_NEXT(leaf_2));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_4), BPT_NEXT(leaf_3));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_5), BPT_NEXT(leaf_4));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_6), BPT_NEXT(leaf_5));

    cache.destroy(&(cache.cache));
}
//}}}

//{{{ void test_bpt_insert_new_value(void)
void test_bpt_insert_new_value(void)
{
    /*
     * 8 
     *  4,6 8,10,12
     *   2,3 4,5 6,7 8,9 10,11 12,13,14
     */
    ORDER = 4;
 
    int *V_0 = (int *) malloc(sizeof(int));
    *V_0 = 1;
    int *V_1 = (int *) malloc(sizeof(int));
    *V_1 = 2;
    int *V_2 = (int *) malloc(sizeof(int));
    *V_2 = 3;
    int *V_3 = (int *) malloc(sizeof(int));
    *V_3 = 4;
    int *V_4 = (int *) malloc(sizeof(int));
    *V_4 = 5;
    int *V_5 = (int *) malloc(sizeof(int));
    *V_5 = 6;
    int *V_6 = (int *) malloc(sizeof(int));
    *V_6 = 7;
    int *V_7 = (int *) malloc(sizeof(int));
    *V_7 = 8;
    int *V_8 = (int *) malloc(sizeof(int));
    *V_8 = 9;
    int *V_9 = (int *) malloc(sizeof(int));
    *V_9 = 10;
    int *V_10 = (int *) malloc(sizeof(int));
    *V_10 = 11;
    int *V_11 = (int *) malloc(sizeof(int));
    *V_11 = 12;
    int *V_12 = (int *) malloc(sizeof(int));
    *V_12 = 13;

    cache.cache = cache.init(10000, NULL);
   
    uint32_t leaf_id;
    int pos;
    uint32_t V_id, root_id;

    root_id = bpt_insert_new_value(0,
                                   2,
                                   V_0,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   3,
                                   V_1,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   4,
                                   V_2,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   5,
                                   V_3,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   6,
                                   V_4,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   7,
                                   V_5,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   8,
                                   V_6,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   9,
                                   V_7,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    root_id = bpt_insert_new_value(root_id,
                                   10,
                                   V_8,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);


    root_id = bpt_insert_new_value(root_id,
                                   11,
                                   V_9,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);


    root_id = bpt_insert_new_value(root_id,
                                   12,
                                   V_10,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);


    root_id = bpt_insert_new_value(root_id,
                                   13,
                                   V_11,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);


    root_id = bpt_insert_new_value(root_id,
                                   14,
                                   V_12,
                                   free_wrapper,
                                   &V_id,
                                   &leaf_id,
                                   &pos);

    // 8
    //   4,6 8,10,12
    //     2,3 4,5 6,7 8,9 10,11 12,13,14
    struct bpt_node *root = cache.get(cache.cache, root_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));

    struct bpt_node *node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(node));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(node)[0]);
    TEST_ASSERT_EQUAL(10, BPT_KEYS(node)[1]);
    TEST_ASSERT_EQUAL(12, BPT_KEYS(node)[2]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(node));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_1 = cache.get(cache.cache, BPT_POINTERS(node)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_1));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(leaf_1)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(leaf_1)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_1));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_2 = cache.get(cache.cache, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_2));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(leaf_2)[0]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(leaf_2)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_2));

    node = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *leaf_3 = cache.get(cache.cache, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_3));
    TEST_ASSERT_EQUAL(6, BPT_KEYS(leaf_3)[0]);
    TEST_ASSERT_EQUAL(7, BPT_KEYS(leaf_3)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_3));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_4 = cache.get(cache.cache, BPT_POINTERS(node)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_4));
    TEST_ASSERT_EQUAL(8, BPT_KEYS(leaf_4)[0]);
    TEST_ASSERT_EQUAL(9, BPT_KEYS(leaf_4)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_4));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_5 = cache.get(cache.cache, BPT_POINTERS(node)[2]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_5));
    TEST_ASSERT_EQUAL(10, BPT_KEYS(leaf_5)[0]);
    TEST_ASSERT_EQUAL(11, BPT_KEYS(leaf_5)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_5));

    node = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    struct bpt_node *leaf_6 = cache.get(cache.cache, BPT_POINTERS(node)[3]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(leaf_6));
    TEST_ASSERT_EQUAL(12, BPT_KEYS(leaf_6)[0]);
    TEST_ASSERT_EQUAL(13, BPT_KEYS(leaf_6)[1]);
    TEST_ASSERT_EQUAL(14, BPT_KEYS(leaf_6)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_6));

    TEST_ASSERT_EQUAL(BPT_ID(leaf_2), BPT_NEXT(leaf_1));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_3), BPT_NEXT(leaf_2));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_4), BPT_NEXT(leaf_3));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_5), BPT_NEXT(leaf_4));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_6), BPT_NEXT(leaf_5));

    cache.destroy(&(cache.cache));

}
//}}}

//{{{ void test_bpt_insert_new_value(void)
void test_bpt_rand_order_insert_new_value(void)
{
    /*
     * 8 
     *  4,6 8,10,12
     *   2,3 4,5 6,7 8,9 10,11 12,13,14
     */
    ORDER = 4;
 
    int *V_0 = (int *) malloc(sizeof(int));
    *V_0 = 1;
    int *V_1 = (int *) malloc(sizeof(int));
    *V_1 = 2;
    int *V_2 = (int *) malloc(sizeof(int));
    *V_2 = 3;
    int *V_3 = (int *) malloc(sizeof(int));
    *V_3 = 4;
    int *V_4 = (int *) malloc(sizeof(int));
    *V_4 = 5;
    int *V_5 = (int *) malloc(sizeof(int));
    *V_5 = 6;
    int *V_6 = (int *) malloc(sizeof(int));
    *V_6 = 7;
    int *V_7 = (int *) malloc(sizeof(int));
    *V_7 = 8;
    int *V_8 = (int *) malloc(sizeof(int));
    *V_8 = 9;
    int *V_9 = (int *) malloc(sizeof(int));
    *V_9 = 10;
    int *V_10 = (int *) malloc(sizeof(int));
    *V_10 = 11;
    int *V_11 = (int *) malloc(sizeof(int));
    *V_11 = 12;
    int *V_12 = (int *) malloc(sizeof(int));
    *V_12 = 13;

    cache.cache = cache.init(10000, NULL);
   
    uint32_t l;
    int p;
    uint32_t v, r;

    r = bpt_insert_new_value(0, 4, V_2, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 3, V_1, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 2, V_0, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 14, V_12, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 5, V_3, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 12, V_10, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 7, V_5, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 9, V_7, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 8, V_6, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 13, V_11, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 11, V_9, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 6, V_4, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 10, V_8, free_wrapper, &v, &l, &p);

    /*
     * 4,7,9,12
     * 2,3, 4,5,6 7,8, 9,10,11 12,13,14
     */

    struct bpt_node *root = cache.get(cache.cache, r);
    TEST_ASSERT_EQUAL(4, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(7, BPT_KEYS(root)[1]);
    TEST_ASSERT_EQUAL(9, BPT_KEYS(root)[2]);
    TEST_ASSERT_EQUAL(12, BPT_KEYS(root)[3]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));

    struct bpt_node *leaf_1 = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_1));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(leaf_1)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(leaf_1)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_1));

    struct bpt_node *leaf_2 = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(leaf_2));
    TEST_ASSERT_EQUAL(4, BPT_KEYS(leaf_2)[0]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(leaf_2)[1]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(leaf_2)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_2));

    struct bpt_node *leaf_3 = cache.get(cache.cache, BPT_POINTERS(root)[2]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(leaf_3));
    TEST_ASSERT_EQUAL(7, BPT_KEYS(leaf_3)[0]);
    TEST_ASSERT_EQUAL(8, BPT_KEYS(leaf_3)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_3));

    struct bpt_node *leaf_4 = cache.get(cache.cache, BPT_POINTERS(root)[3]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(leaf_4));
    TEST_ASSERT_EQUAL(9, BPT_KEYS(leaf_4)[0]);
    TEST_ASSERT_EQUAL(10, BPT_KEYS(leaf_4)[1]);
    TEST_ASSERT_EQUAL(11, BPT_KEYS(leaf_4)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_4));

    struct bpt_node *leaf_5 = cache.get(cache.cache, BPT_POINTERS(root)[4]);
    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(leaf_5));
    TEST_ASSERT_EQUAL(12, BPT_KEYS(leaf_5)[0]);
    TEST_ASSERT_EQUAL(13, BPT_KEYS(leaf_5)[1]);
    TEST_ASSERT_EQUAL(14, BPT_KEYS(leaf_5)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(leaf_5));

    TEST_ASSERT_EQUAL(BPT_ID(leaf_2), BPT_NEXT(leaf_1));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_3), BPT_NEXT(leaf_2));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_4), BPT_NEXT(leaf_3));
    TEST_ASSERT_EQUAL(BPT_ID(leaf_5), BPT_NEXT(leaf_4));

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_bpt_insert_id_updated_bpt_node(void)
void test_bpt_insert_id_updated_bpt_node(void)
{
    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

    cache.cache = cache.init(10000, NULL);

    uint32_t r = 0, l, v = 0, v_id;
    int p;

    //r = bpt_insert_new_value(0, 4, V_2, free_wrapper, &v, &l, &p);

    r = bpt_insert_new_value(r, 2, (V + v++), NULL, &v_id, &l, &p);
    r = bpt_insert_new_value(r, 5, (V + v++), NULL, &v_id, &l, &p);
    r = bpt_insert_new_value(r, 10, (V + v++), NULL, &v_id, &l, &p);
    r = bpt_insert_new_value(r, 15, (V + v++), NULL, &v_id, &l, &p);

    r = bpt_insert_new_value(r, 1, (void *)(V + v++), NULL, &v_id, &l, &p);
    struct bpt_node *root = cache.get(cache.cache, r);
    TEST_ASSERT_EQUAL(BPT_POINTERS(root)[0], l);

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_find(void)
void test_find(void)
{
    /*
     *  -> 2
     *  -> 4
     *  -> 6
     *  -> 8
     *  2,4,6,8
     *
     *  -> 10 
     *  6
     *  2,4, 6,8,10
     *
     *  -> 12 
     *  6
     *  2,4, 6,8,10,12
     *
     *  -> 14 
     *  6,10
     *  2,4, 6,8  10,12,14
     *
     *  -> 16 
     *  6,10
     *  2,4, 6,8  10,12,14,16
     *
     *  -> 18 
     *  6,10,14
     *  2,4, 6,8  10,12 14,16,18
     *
     *  -> 20 
     *  6,10,14
     *  2,4, 6,8  10,12 14,16,18,20
     *
     *  -> 22 
     *  6,10,14,18
     *  2,4, 6,8  10,12 14,16 18,20,22
     *
     *  -> 24 
     *  6,10,14,18
     *  2,4, 6,8  10,12 14,16 18,20,22,24
     *
     *  -> 26 
     *  14
     *  6,10 14,18,22
     *  2,4, 6,8  10,12 14,16 18,20 22,24,26
     *
     */

    cache.cache = cache.init(10000, NULL);

    int V[13] = {1,2,3,4,5,6,7,8,9,10,11,12,13};
    int v=0;

    uint32_t r_id = 0, l_id, v_id;
    int pos;

    int i;
    for (i = 0; i < 13; ++i)
        r_id = bpt_insert_new_value(r_id,
                                    (i+1)*2,
                                    (V + v++),
                                    NULL,
                                    &v_id,
                                    &l_id,
                                    &pos);

    uint32_t r_i;
    int *r, pos_r, pos_r_i = 0;
    v=0;
    int POS_R[13] = {0,1,0,1,0,1,0,1,0,1,0,1,2};
    for (i = 1; i <= 13; ++i) {
        r_i = bpt_find(r_id, &l_id, &pos_r, i);
        if (i % 2 != 0)
            TEST_ASSERT_EQUAL(0, r_i);
        else  {
            r = cache.get(cache.cache, r_i);
            TEST_ASSERT_EQUAL(i/2, *r);
            TEST_ASSERT_EQUAL(POS_R[pos_r_i], pos_r);
            pos_r_i += 1;
        }
    }

    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_split_repair(void)
void decrement_repair(struct bpt_node *a, struct bpt_node *b)
{
    uint32_t i;
    for (i = 0; i < BPT_NUM_KEYS(a); ++i)
        BPT_KEYS(a)[i] = BPT_KEYS(a)[i] - 1;

    for (i = 0; i < BPT_NUM_KEYS(b); ++i)
        BPT_KEYS(b)[i] = BPT_KEYS(b)[i] + 1;

}

void test_split_repair(void)
{
    /*
     * 2,3,4,5
     *
     * 4
     *  2,3 4,5,6
     *
     * 4,6
     *  2,3 4,5 6,7,8
     *
     * 4,6,8
     *  2,3 4,5 6,7 8,9,10
     *
     * 4,6,8,10
     *  2,3 4,5 6,7 8,9 10,11,12
     *
     * 8 
     *  4,6 8,10,12
     *   2,3 4,5 6,7 8,9 10,11 12,13,14
     */
    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    int v=0;

    repair = decrement_repair;

    cache.cache = cache.init(10000, NULL);

    uint32_t r_id = 0, l_id, v_id;
    int pos;

    //root = bpt_insert(root, 2, (void *)(V + v++), &leaf, &pos);
    int i;
    r_id = bpt_insert_new_value(r_id, 2, (V + v++), NULL, &v_id, &l_id, &pos);


    struct bpt_node *root = cache.get(cache.cache, r_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(root));

    //  2,3,4,5
    r_id = bpt_insert_new_value(r_id, 3, (V + v++), NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 4, (V + v++), NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 5, (V + v++), NULL, &v_id, &l_id, &pos);

    root = cache.get(cache.cache, r_id);
    TEST_ASSERT_EQUAL(4, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(2, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(3, BPT_KEYS(root)[1]);
    TEST_ASSERT_EQUAL(4, BPT_KEYS(root)[2]);
    TEST_ASSERT_EQUAL(5, BPT_KEYS(root)[3]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(root));

    // 4
    //  2,3 4,5,6
    r_id = bpt_insert_new_value(r_id, 6, (V + v++), NULL, &v_id, &l_id, &pos);

    // Using a silly split repair function just to test, 
    // -1 the keys in the left bpt_node and + those in the right

    // 5
    //  1,2 5,6,7
    root = cache.get(cache.cache, r_id);
    TEST_ASSERT_EQUAL(1, BPT_NUM_KEYS(root));
    TEST_ASSERT_EQUAL(5, BPT_KEYS(root)[0]);
    TEST_ASSERT_EQUAL(0, BPT_IS_LEAF(root));

    struct bpt_node *node_l = cache.get(cache.cache, BPT_POINTERS(root)[0]);
    struct bpt_node *node_r = cache.get(cache.cache, BPT_POINTERS(root)[1]);
    TEST_ASSERT_EQUAL(2, BPT_NUM_KEYS(node_l));
    TEST_ASSERT_EQUAL(1, BPT_KEYS(node_l)[0]);
    TEST_ASSERT_EQUAL(2, BPT_KEYS(node_l)[1]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node_l));
    TEST_ASSERT_EQUAL(BPT_POINTERS(root)[1], BPT_NEXT(node_l));

    TEST_ASSERT_EQUAL(3, BPT_NUM_KEYS(node_r));
    TEST_ASSERT_EQUAL(5, BPT_KEYS(node_r)[0]);
    TEST_ASSERT_EQUAL(6, BPT_KEYS(node_r)[1]);
    TEST_ASSERT_EQUAL(7, BPT_KEYS(node_r)[2]);
    TEST_ASSERT_EQUAL(1, BPT_IS_LEAF(node_r));
    TEST_ASSERT_EQUAL(0, BPT_NEXT(node_r));

    repair = NULL;

    cache.destroy(&(cache.cache));
}
//}}}

//{{{ void test_rand_test(void)
void test_rand_test(void)
{
    repair = NULL;
    ORDER = 10;
    cache.cache = cache.init(CACHE_SIZE, NULL);

    uint32_t r_id = 0, l_id, v_id;
    int pos;

    uint32_t size = 1000;

    uint32_t *d = (uint32_t *)malloc(size * sizeof(uint32_t));
    uint32_t *v = (uint32_t *)malloc(size * sizeof(uint32_t));

    time_t t = time(NULL);
    srand(t);

    uint32_t i, j;
    for (i = 0; i < size; ++i) {
        d[i] = rand() % 100000;
        v[i] = d[i] /2;
        r_id = bpt_insert_new_value(r_id,
                                    d[i],
                                    (v + i),
                                    NULL,
                                    &v_id,
                                    &l_id,
                                    &pos);
    }

    int pos_i;
    for (i = 0; i < size; ++i) {
        uint32_t r_i = bpt_find(r_id, &l_id, &pos_i, d[i]);
        int *r = cache.get(cache.cache, r_i);
        TEST_ASSERT_EQUAL(d[i]/2, *r);
    }

    free(d);
    free(v);
    cache.destroy(&(cache.cache));
}
//}}}

//{{{ void test_bpt_insert_repeat(void)
void test_bpt_insert_repeat(void)
{
    repair = NULL;
    ORDER = 4;
    cache.cache = cache.init(CACHE_SIZE, NULL);

    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

    uint32_t r_id = 0, v_id, l_id;
    int pos;

    r_id = bpt_insert_new_value(r_id, 3, V, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 5, V + 1, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 4, V + 2, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 6, V + 3, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 2, V + 4, NULL, &v_id, &l_id, &pos);

    r_id = bpt_insert_new_value(r_id, 2, V + 5, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 3, V + 6, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 4, V + 7, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 5, V + 8, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 6, V + 9, NULL, &v_id, &l_id, &pos);


    uint32_t r_i = bpt_find(r_id, &l_id, &pos, 2);
    int *r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(V[5], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 3);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(V[6], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 4);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(V[7], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 5);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(V[8], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 6);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(V[9], *r);

    cache.destroy(&(cache.cache));
}
//}}}

//{{{ void test_bpt_insert_repeat_append(void)

void append_sum(uint32_t new_value_id, uint32_t curr_value_id)
{

    int *new_value = cache.get(cache.cache, new_value_id);
    int *curr_value = cache.get(cache.cache, curr_value_id);
    *curr_value = *curr_value + *new_value;
}

void test_bpt_insert_repeat_append(void)
{
    append = append_sum;
    repair = NULL;
    ORDER = 4;
    cache.cache = cache.init(CACHE_SIZE, NULL);

    int R[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

    uint32_t r_id = 0, v_id, l_id;
    int pos;

    r_id = bpt_insert_new_value(r_id, 3, V, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 5, V + 1, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 4, V + 2, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 6, V + 3, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 2, V + 4, NULL, &v_id, &l_id, &pos);

    r_id = bpt_insert_new_value(r_id, 2, V + 5, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 3, V + 6, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 4, V + 7, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 5, V + 8, NULL, &v_id, &l_id, &pos);
    r_id = bpt_insert_new_value(r_id, 6, V + 9, NULL, &v_id, &l_id, &pos);


    uint32_t r_i = bpt_find(r_id, &l_id, &pos, 2);
    int *r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(R[5] + R[4], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 3);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(R[6] + R[0], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 4);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(R[7] + R[2], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 5);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(R[8] + R[1], *r);

    r_i = bpt_find(r_id, &l_id, &pos, 6);
    r = cache.get(cache.cache, r_i);
    TEST_ASSERT_EQUAL(R[9] + R[3], *r);

    cache.destroy(&(cache.cache));

    append = NULL;
}
//}}}

//{{{ void test_rand_test_high_order(void)
void test_rand_test_high_order(void)
{
    cache.cache = cache.init(10, NULL);
    ORDER=50;
    repair = NULL;
    uint32_t size = 100000;

    uint32_t *d = (uint32_t *)malloc(size * sizeof(uint32_t));
    uint32_t *v = (uint32_t *)malloc(size * sizeof(uint32_t));

    time_t t = time(NULL);
    srand(t);

    uint32_t r_id = 0, v_id, l_id;
    int pos;

 
    uint32_t i, j;
    for (i = 0; i < size; ++i) {
        d[i] = rand();
        v[i] = d[i] /2;
        r_id = bpt_insert_new_value(r_id,
                                    d[i],
                                    v + i,
                                    NULL,
                                    &v_id,
                                    &l_id,
                                    &pos);
    }

    uint32_t *r;
    int pos_r;
    for (i = 0; i < size; ++i) {
        uint32_t r_i = bpt_find(r_id, &l_id, &pos, d[i]);
        int *r = cache.get(cache.cache, r_i);
        TEST_ASSERT_EQUAL(v[i], *r);
    }

    free(d);
    free(v);
    cache.destroy(&(cache.cache));
}
//}}}

//{{{void test_bpt_write_tree(void)
void test_bpt_write_tree(void)
{
    ORDER=5;
    cache.cache = cache.init(10, NULL);
    repair = NULL;
    uint32_t size = 20;

    uint32_t *d = (uint32_t *)malloc(size * sizeof(uint32_t));
    uint32_t *v = (uint32_t *)malloc(size * sizeof(uint32_t));

    time_t t = time(NULL);
    srand(t);

    uint32_t r_id = 0, v_id, l_id;
    int pos;

    uint32_t i, j;
    for (i = 0; i < size; ++i) {
        d[i] = rand();
        v[i] = d[i] /2;
        r_id = bpt_insert_new_value(r_id,
                                    d[i],
                                    v + i,
                                    NULL,
                                    &v_id,
                                    &l_id,
                                    &pos);
    }

    char *bpt_file_out = "test_bpt_write_tree.out";

    FILE *f = fopen(bpt_file_out, "wb");
    bpt_write_tree(r_id, f, bpt_file_out);
    fclose(f);

    f = NULL;

    struct disk_store *ds = disk_store_load(&f, bpt_file_out);


    struct bpt_node *read_root = (struct bpt_node *)
            malloc(sizeof(struct bpt_node));
    uint64_t node_size;
    read_root->data = disk_store_get(ds, 0, &node_size);

    struct bpt_node *mem_root = cache.get(cache.cache, r_id);

    TEST_ASSERT_EQUAL(1, BPT_ID(read_root));
    TEST_ASSERT_EQUAL(BPT_NUM_KEYS(mem_root), BPT_NUM_KEYS(read_root));

    for (i = 0; i < BPT_NUM_KEYS(mem_root); ++i) 
        TEST_ASSERT_EQUAL(BPT_KEYS(mem_root)[i], BPT_KEYS(read_root)[i]);

    for (i = 0; i <= BPT_NUM_KEYS(mem_root); ++i) {
        struct bpt_node *mem_node = cache.get(cache.cache,
                                              BPT_POINTERS(mem_root)[i]);
        struct bpt_node *read_node = (struct bpt_node *)
                malloc(sizeof(struct bpt_node));
        read_node->data = disk_store_get(ds,
                                         BPT_POINTERS(read_root)[i] - 1,
                                         &node_size);
        TEST_ASSERT_EQUAL(BPT_NUM_KEYS(mem_node), BPT_NUM_KEYS(read_node));

        for (j = 0; j < BPT_NUM_KEYS(mem_node); ++j) {
            TEST_ASSERT_EQUAL(BPT_KEYS(mem_node)[j], BPT_KEYS(read_node)[j]);

            uint32_t *mem_val = cache.get(cache.cache,
                                          BPT_POINTERS(mem_node)[j]);
            uint32_t *read_val = disk_store_get(ds,
                                                BPT_POINTERS(read_node)[j] - 1,
                                                &node_size);
            TEST_ASSERT_EQUAL(*mem_val, *read_val);
            free(read_val);
        }

        free(read_node->data);
        free(read_node);
    }

    free(read_root->data);
    free(read_root);

    free(d);
    free(v);
    cache.destroy(&(cache.cache));
    disk_store_destroy(&ds);
}
//}}}

#if 0
////{{{ void test_bpt_destroy_tree(void)
//void test_bpt_destroy_tree(void)
//{
//    TEST_IGNORE()
//
//    /*
//     * 2,3,4,5
//     *
//     * 4
//     *  2,3 4,5,6
//     *
//     * 4,6
//     *  2,3 4,5 6,7,8
//     *
//     * 4,6,8
//     *  2,3 4,5 6,7 8,9,10
//     *
//     * 4,6,8,10
//     *  2,3 4,5 6,7 8,9 10,11,12
//     *
//     * 8 
//     *  4,6 8,10,12
//     *   2,3 4,5 6,7 8,9 10,11 12,13,14
//     */
//    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
//    int v=0;
//
//    struct bpt_node *root = NULL;
//    struct bpt_node *leaf = NULL;
//    int pos;
//
//    root = bpt_insert(root, 2, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 3, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 4, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 5, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 6, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 7, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 8, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 9, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 10, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 11, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 12, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 13, (void *)(V + v++), &leaf, &pos);
//    root = bpt_insert(root, 14, (void *)(V + v++), &leaf, &pos);
//
//    bpt_destroy_tree(&root);
//}
////}}}
////{{{ void test_bpt_find_null(void)
//void test_bpt_find_null(void)
//{
//    TEST_IGNORE()
//    int V[14] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};
//    int v=0;
//
//    struct bpt_node *root = NULL;
//    struct bpt_node *leaf = NULL;
//    int pos_r;
//
//    int *r = (int *)bpt_find(root, &leaf, &pos_r, 0);
//
//    TEST_ASSERT_EQUAL(NULL, r);
//
//    root = bpt_insert(root, 2, (void *)(V + v++), &leaf, &pos_r);
//
//    r = (int *)bpt_find(root, &leaf, &pos_r, 2);
//    TEST_ASSERT_EQUAL(V, r);
//
//    r = (int *)bpt_find(root, &leaf, &pos_r, 1);
//    TEST_ASSERT_EQUAL(NULL, r);
//
//    bpt_destroy_tree(&root);
//}
////}}}
#endif
