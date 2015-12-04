#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "bpt.h"
#include "giggle.h"

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

    uint32_t_ll_free(&ll);

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

    uint32_t_ll_free(&ll);

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

    uint32_t_ll_free(&ll);

}
//}}}

//{{{void test_giggle_insert(void)
void test_giggle_insert(void)
{
    struct bpt_node *root = NULL;

    uint32_t r = giggle_insert(&root, 1, 3, 0);
    // 1(SA:0),4(SE:0)
    TEST_ASSERT_EQUAL(2, root->num_keys);
    TEST_ASSERT_EQUAL(1, root->keys[0]);
    TEST_ASSERT_EQUAL(4, root->keys[1]);

    r = giggle_insert(&root, 2, 4, 1);
    // 1(SA:0),2(SA:1),4(SE:0),5(SE:1)
    TEST_ASSERT_EQUAL(4, root->num_keys);
    TEST_ASSERT_EQUAL(1, root->keys[0]);
    TEST_ASSERT_EQUAL(2, root->keys[1]);
    TEST_ASSERT_EQUAL(4, root->keys[2]);
    TEST_ASSERT_EQUAL(5, root->keys[3]);

    r = giggle_insert(&root, 4, 6, 2);
    // 4
    // 1(SA:0),2(SA:1) (0 1)4(SA:2 SE:0),5(SE:1),7(SE:2)
    TEST_ASSERT_EQUAL(1, root->num_keys);
    TEST_ASSERT_EQUAL(4, root->keys[0]);
    struct bpt_node *first_leaf = (struct bpt_node *) root->pointers[0];
    TEST_ASSERT_EQUAL(2, first_leaf->num_keys);
    TEST_ASSERT_EQUAL(1, first_leaf->keys[0]);
    TEST_ASSERT_EQUAL(2, first_leaf->keys[1]);
    TEST_ASSERT_EQUAL(3, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(4, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(5, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(NULL, first_leaf->leading);
    TEST_ASSERT_TRUE(first_leaf->next->leading != NULL);

    struct giggle_bpt_leading_data *ld =
            (struct giggle_bpt_leading_data *)
            first_leaf->next->leading;

    TEST_ASSERT_EQUAL(2, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);


    r = giggle_insert(&root, 4, 5, 3);
    // 4
    // 1(SA:0),2(SA:1) (0 1)4(SA:2,3 SE:0),5(SE:1),6(SE:3),7(SE:2)
    first_leaf = (struct bpt_node *) root->pointers[0];
    ld = (struct giggle_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    TEST_ASSERT_EQUAL(2, first_leaf->num_keys);
    struct giggle_bpt_non_leading_data *nld = 
        (struct giggle_bpt_non_leading_data *)first_leaf->pointers[0];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(0, nld->SA->head->val);
    nld = (struct giggle_bpt_non_leading_data *)first_leaf->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(1, nld->SA->head->val);

    ld = (struct giggle_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(2, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(4, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(5, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(6, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[3]);
    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[0];
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(2, nld->SA->head->val);
    TEST_ASSERT_EQUAL(3, nld->SA->head->next->val);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(0, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(1, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[2];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(3, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[3];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(2, nld->SE->head->val);

    r = giggle_insert(&root, 1, 6, 4);
    // 4
    // 1(SA:0,4),2(SA:1) (0 1 4)4(SA:2,3 SE:0),5(SE:1),6(SE:3),7(SE:2,4)
    first_leaf = (struct bpt_node *) root->pointers[0];
    ld = (struct giggle_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    TEST_ASSERT_EQUAL(2, first_leaf->num_keys);
    nld = (struct giggle_bpt_non_leading_data *)first_leaf->pointers[0];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(0, nld->SA->head->val);
    TEST_ASSERT_EQUAL(4, nld->SA->head->next->val);
    nld = (struct giggle_bpt_non_leading_data *)first_leaf->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(1, nld->SA->head->val);

    ld = (struct giggle_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(3, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(4, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(5, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(6, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[3]);
    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[0];
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(2, nld->SA->head->val);
    TEST_ASSERT_EQUAL(3, nld->SA->head->next->val);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(0, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(1, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[2];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(3, nld->SE->head->val);

    nld = (struct giggle_bpt_non_leading_data *)first_leaf->next->pointers[3];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(2, nld->SE->len);
    TEST_ASSERT_EQUAL(2, nld->SE->head->val);
    TEST_ASSERT_EQUAL(4, nld->SE->head->next->val);

    r = giggle_insert(&root, 1, 7, 5);
    // 4,6
    //   (NULL)    1(SA:0,4,5),2(SA:1)
    //   (0 1 4 5) 4(SA:2,3 SE:0),5(SE:1)
    //   (2 3 4 5) 6(SE:3),7(SE:2,4),8(SE:5)
    TEST_ASSERT_EQUAL(2, root->num_keys);
    TEST_ASSERT_EQUAL(4, root->keys[0]);
    TEST_ASSERT_EQUAL(6, root->keys[1]);
    first_leaf = (struct bpt_node *) root->pointers[0];
    ld = (struct giggle_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    ld = (struct giggle_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(5, ld->B->head->next->next->next->val);
    ld = (struct giggle_bpt_leading_data *) first_leaf->next->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(4, ld->B->head->val);
    TEST_ASSERT_EQUAL(2, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(3, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(5, ld->B->head->next->next->next->val);
}
//}}}

//{{{void test_giggle_search(void)
void test_giggle_search(void)
{
    ORDER = 7;
    struct bpt_node *root = NULL;
    uint32_t r;

    /*
     *  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
     *  |0-------------------------|  |1----|     |9------| 
     *     |2----|  |3-------|
     *        |4-------|  |5----|     |7-------------------|
     *                    |6------------|
     *  |8----------------------------------------------|
     *
     */

    r = giggle_insert(&root, 1, 10, 0);
    r = giggle_insert(&root, 11, 13, 1);
    r = giggle_insert(&root, 2, 4, 2);
    r = giggle_insert(&root, 5, 8, 3);
    r = giggle_insert(&root, 3, 6, 4);
    r = giggle_insert(&root, 7, 9, 5);
    r = giggle_insert(&root, 7, 12, 6);

    r = giggle_insert(&root, 11, 18, 7);
    r = giggle_insert(&root, 1, 17, 8);
    r = giggle_insert(&root, 15, 18, 9);

    /*
    7 13
      (NULL)    1(SA:0,8) 2(SA:2) 3(SA:4) 5(SA:3 SE:2) 
      (0 3 4 8) 7(SA:5,6 SE:4) 9(SE: 3) 10(SE:5) 11(SA:1,7 SE:0) 
      (8 6 1 7) 13(SE:6) 14(SE:1) 15(SA:9) 18(SE:8) 19(SE:7,9)
    */

    TEST_ASSERT_EQUAL(2, root->num_keys);
    TEST_ASSERT_EQUAL(7, root->keys[0]);
    TEST_ASSERT_EQUAL(13, root->keys[1]);

    struct bpt_node *first_leaf = (struct bpt_node *) root->pointers[0];
    TEST_ASSERT_EQUAL(4, first_leaf->num_keys);
    TEST_ASSERT_EQUAL(1, first_leaf->keys[0]);
    TEST_ASSERT_EQUAL(2, first_leaf->keys[1]);
    TEST_ASSERT_EQUAL(3, first_leaf->keys[2]);
    TEST_ASSERT_EQUAL(5, first_leaf->keys[3]);

    struct giggle_bpt_leading_data *ld =
            (struct giggle_bpt_leading_data *)
            first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);

    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(9, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(10, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(11, first_leaf->next->keys[3]);

    ld = (struct giggle_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 0));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 3));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 4));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 8));

    TEST_ASSERT_EQUAL(5, first_leaf->next->next->num_keys);
    TEST_ASSERT_EQUAL(13, first_leaf->next->next->keys[0]);
    TEST_ASSERT_EQUAL(14, first_leaf->next->next->keys[1]);
    TEST_ASSERT_EQUAL(15, first_leaf->next->next->keys[2]);
    TEST_ASSERT_EQUAL(18, first_leaf->next->next->keys[3]);
    TEST_ASSERT_EQUAL(19, first_leaf->next->next->keys[4]);

    ld = (struct giggle_bpt_leading_data *) first_leaf->next->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 8));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 6));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 1));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(ld->B, 7));

    /*
     *  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
     *  |0-------------------------|  |1----|     |9------| 
     *     |2----|  |3-------|
     *        |4-------|  |5----|     |7-------------------|
     *                    |6------------|
     *  |8----------------------------------------------|
     *
     */

    struct uint32_t_ll *R = giggle_search(root, 2, 5);
    TEST_ASSERT_EQUAL(5, R->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 0));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 2));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 4));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 3));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 8));

    uint32_t_ll_free(&R);

    R = giggle_search(root, 5, 15);
    TEST_ASSERT_EQUAL(9, R->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 0));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 1));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 3));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 4));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 5));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 6));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 7));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 8));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 9));

    uint32_t_ll_free(&R);

    R = giggle_search(root, 19, 20);
    TEST_ASSERT_EQUAL(NULL, R);

    uint32_t_ll_free(&R);

    R = giggle_search(root, 18, 20);
    TEST_ASSERT_EQUAL(2, R->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 9));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 7));
}
//}}}

