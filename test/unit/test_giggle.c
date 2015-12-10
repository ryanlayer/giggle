#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "bpt.h"
#include "giggle.h"
#include "lists.h"
#include "file_read.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_uint32_t_ll_giggle_insert(void)
void test_uint32_t_ll_giggle_insert(void)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

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

    struct uint32_t_ll_bpt_leading_data *ld =
            (struct uint32_t_ll_bpt_leading_data *)
            first_leaf->next->leading;

    TEST_ASSERT_EQUAL(2, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);


    r = giggle_insert(&root, 4, 5, 3);
    // 4
    // 1(SA:0),2(SA:1) (0 1)4(SA:2,3 SE:0),5(SE:1),6(SE:3),7(SE:2)
    first_leaf = (struct bpt_node *) root->pointers[0];
    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    TEST_ASSERT_EQUAL(2, first_leaf->num_keys);
    struct uint32_t_ll_bpt_non_leading_data *nld = 
        (struct uint32_t_ll_bpt_non_leading_data *)first_leaf->pointers[0];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(0, nld->SA->head->val);
    nld = (struct uint32_t_ll_bpt_non_leading_data *)first_leaf->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(1, nld->SA->head->val);

    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(2, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(4, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(5, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(6, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[3]);
    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[0];
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(2, nld->SA->head->val);
    TEST_ASSERT_EQUAL(3, nld->SA->head->next->val);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(0, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(1, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[2];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(3, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[3];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(2, nld->SE->head->val);

    r = giggle_insert(&root, 1, 6, 4);
    // 4
    // 1(SA:0,4),2(SA:1) (0 1 4)4(SA:2,3 SE:0),5(SE:1),6(SE:3),7(SE:2,4)
    first_leaf = (struct bpt_node *) root->pointers[0];
    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    TEST_ASSERT_EQUAL(2, first_leaf->num_keys);
    nld = (struct uint32_t_ll_bpt_non_leading_data *)first_leaf->pointers[0];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(0, nld->SA->head->val);
    TEST_ASSERT_EQUAL(4, nld->SA->head->next->val);
    nld = (struct uint32_t_ll_bpt_non_leading_data *)first_leaf->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SE);
    TEST_ASSERT_EQUAL(1, nld->SA->len);
    TEST_ASSERT_EQUAL(1, nld->SA->head->val);

    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(3, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(4, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(5, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(6, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[3]);
    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[0];
    TEST_ASSERT_EQUAL(2, nld->SA->len);
    TEST_ASSERT_EQUAL(2, nld->SA->head->val);
    TEST_ASSERT_EQUAL(3, nld->SA->head->next->val);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(0, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[1];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(1, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[2];
    TEST_ASSERT_EQUAL(NULL, nld->SA);
    TEST_ASSERT_EQUAL(1, nld->SE->len);
    TEST_ASSERT_EQUAL(3, nld->SE->head->val);

    nld = (struct uint32_t_ll_bpt_non_leading_data *)
            first_leaf->next->pointers[3];
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
    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);
    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(0, ld->B->head->val);
    TEST_ASSERT_EQUAL(1, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(4, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(5, ld->B->head->next->next->next->val);
    ld = (struct uint32_t_ll_bpt_leading_data *) 
            first_leaf->next->next->leading;
    TEST_ASSERT_EQUAL(4, ld->B->len);
    TEST_ASSERT_EQUAL(4, ld->B->head->val);
    TEST_ASSERT_EQUAL(2, ld->B->head->next->val);
    TEST_ASSERT_EQUAL(3, ld->B->head->next->next->val);
    TEST_ASSERT_EQUAL(5, ld->B->head->next->next->next->val);
}
//}}}

//{{{ void test_uint32_t_ll_giggle_search(void)
void test_uint32_t_ll_giggle_search(void)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

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

    struct uint32_t_ll_bpt_leading_data *ld =
            (struct uint32_t_ll_bpt_leading_data *)
            first_leaf->leading;
    TEST_ASSERT_EQUAL(NULL, ld);

    TEST_ASSERT_EQUAL(4, first_leaf->next->num_keys);
    TEST_ASSERT_EQUAL(7, first_leaf->next->keys[0]);
    TEST_ASSERT_EQUAL(9, first_leaf->next->keys[1]);
    TEST_ASSERT_EQUAL(10, first_leaf->next->keys[2]);
    TEST_ASSERT_EQUAL(11, first_leaf->next->keys[3]);

    ld = (struct uint32_t_ll_bpt_leading_data *) first_leaf->next->leading;
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

    ld = (struct uint32_t_ll_bpt_leading_data *)
            first_leaf->next->next->leading;
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

    struct uint32_t_ll *R = (struct uint32_t_ll *)giggle_search(root, 2, 5);
    TEST_ASSERT_EQUAL(5, R->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 0));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 2));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 4));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 3));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 8));

    uint32_t_ll_free(&R);

    R = (struct uint32_t_ll *)giggle_search(root, 5, 15);
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

    R = (struct uint32_t_ll *)giggle_search(root, 19, 20);
    TEST_ASSERT_EQUAL(NULL, R);

    uint32_t_ll_free(&R);

    R = (struct uint32_t_ll *)giggle_search(root, 18, 20);
    TEST_ASSERT_EQUAL(2, R->len);
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 9));
    TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, 7));
}
//}}}

//{{{void test_giggle_get_chrm_id(void)
void test_giggle_get_chrm_id(void)
{
    struct giggle_index *gi = giggle_init_index(3);
    TEST_ASSERT_EQUAL(0, giggle_get_chrm_id(gi, "chr2"));
    TEST_ASSERT_EQUAL(1, giggle_get_chrm_id(gi, "chr1"));
    TEST_ASSERT_EQUAL(2, giggle_get_chrm_id(gi, "chr3"));
    TEST_ASSERT_EQUAL(0, giggle_get_chrm_id(gi, "chr2"));

    TEST_ASSERT_EQUAL(3, giggle_get_chrm_id(gi, "chr9"));
    TEST_ASSERT_EQUAL(4, giggle_get_chrm_id(gi, "chr8"));
    TEST_ASSERT_EQUAL(5, giggle_get_chrm_id(gi, "chr7"));
    TEST_ASSERT_EQUAL(3, giggle_get_chrm_id(gi, "chr9"));
}
//}}}

//{{{void test_giggle_index_file(void)
void test_giggle_index_file(void)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

    struct bpt_node *root = NULL;

    char *file_name = "../data/1k.unsort.bed.gz";

    ORDER = 10;

    struct giggle_index *gi = giggle_init_index(23);
    uint32_t r = giggle_index_file(gi, file_name);

    TEST_ASSERT_EQUAL(1000, r);
    TEST_ASSERT_EQUAL(23, gi->chrm_index->num); 
    TEST_ASSERT_EQUAL(23, gi->num); 

    uint32_t sizes[23] = {152,
                          44,
                          66,
                          41,
                          9,
                          7,
                          30,
                          43,
                          38,
                          23,
                          77,
                          64,
                          21,
                          18,
                          27,
                          50,
                          20,
                          40,
                          73,
                          80,
                          33,
                          32,
                          12};

    char *chrms[23] = {"chr1",
                       "chr10",
                       "chr11",
                       "chr12",
                       "chr13",
                       "chr14",
                       "chr15",
                       "chr16",
                       "chr17",
                       "chr18",
                       "chr19",
                       "chr2",
                       "chr20",
                       "chr21",
                       "chr22",
                       "chr3",
                       "chr4",
                       "chr5",
                       "chr6",
                       "chr7",
                       "chr8",
                       "chr9",
                       "chrX"};
    /*
      for c in `gunzip -c 1k.unsort.bed.gz | cut -f1 | sort | uniq`
      do 
        V=`gunzip -c 1k.unsort.bed.gz | grep -n -e "$c\t" | cut -d":" -f1 | tr '\n' ',' | sed -e "s/,$//"`
        echo "{$V}"
      done
     */
    uint32_t ids[23][152] = {
        //{{{
        {18,22,41,42,59,62,67,69,71,85,86,87,88,98,103,104,106,114,131,133,141,148,151,156,168,170,177,180,189,190,193,197,204,207,211,212,218,221,241,247,255,257,261,267,276,282,311,322,327,333,338,358,359,365,372,386,408,414,418,458,466,467,468,473,480,483,485,496,499,500,504,512,525,549,552,558,568,570,574,578,581,587,601,607,612,613,625,626,628,638,641,650,657,666,676,677,678,679,684,686,687,688,693,697,715,718,724,729,730,735,740,745,749,753,755,763,764,766,776,781,783,785,800,801,817,821,822,828,844,846,865,869,884,885,889,924,928,930,934,941,943,952,954,956,962,964,977,978,988,994,995,1000},
        {3,27,34,76,79,92,113,130,164,215,216,222,271,272,332,390,409,425,463,486,519,569,585,593,606,611,622,645,665,669,689,696,719,748,775,780,790,853,855,871,886,898,908,998},
        {1,2,55,90,91,117,118,128,129,144,147,217,244,290,317,323,326,341,342,347,383,394,413,435,436,514,515,521,528,540,544,551,557,573,590,619,633,639,660,682,702,709,714,731,738,746,760,765,774,804,816,833,834,837,856,858,863,868,873,874,904,912,913,950,957,973},
        {47,70,80,107,124,145,159,205,210,228,236,253,258,288,393,398,401,410,474,481,507,531,537,554,579,584,588,635,640,670,704,712,713,727,770,805,810,887,896,925,974},
        {56,82,158,194,396,426,809,849,909},
        {102,198,501,516,672,705,918},
        {5,37,100,105,196,279,382,387,400,442,487,518,526,562,572,597,621,651,708,710,761,769,778,806,824,826,857,953,965,999},
        {4,50,64,115,127,149,152,214,225,248,259,275,281,293,310,321,357,421,424,439,491,506,547,609,767,768,795,862,870,872,878,893,905,906,911,915,919,921,935,944,975,979,996},
        {29,43,96,123,171,191,203,226,264,302,331,335,348,349,420,438,471,510,610,627,636,658,692,758,779,789,814,851,861,892,894,916,917,923,931,971,980,989},
        {8,30,31,73,122,140,162,200,213,251,309,336,378,490,524,527,556,637,711,784,802,907,983},
        {6,7,25,39,48,49,61,77,78,84,108,110,116,126,136,137,160,165,166,185,232,260,270,277,294,295,303,307,313,315,325,345,350,356,368,369,370,375,381,385,419,422,443,454,457,465,475,495,548,560,577,598,618,620,630,685,690,706,716,717,737,747,771,794,808,811,812,845,859,891,926,932,947,958,960,961,968},
        {17,51,60,68,101,142,175,178,227,234,235,252,254,274,286,287,292,296,298,314,316,339,344,353,361,397,433,445,448,451,455,523,530,539,594,596,599,608,629,642,674,681,732,742,754,757,762,807,815,818,825,847,854,875,900,901,902,914,938,948,963,966,981,986},
        {23,35,65,157,280,389,395,403,444,494,563,567,605,673,721,725,803,850,883,903,936},
        {9,112,181,209,324,402,411,520,536,564,643,683,759,798,835,836,929,997},
        {28,66,81,94,150,169,238,245,273,284,304,377,392,456,502,604,698,791,793,797,820,838,839,882,895,991,992},
        {24,54,72,83,89,93,139,153,161,163,224,230,242,243,269,283,300,328,346,362,363,367,373,406,428,434,460,470,476,479,488,497,513,517,532,534,543,580,617,649,664,680,723,788,827,880,881,942,945,949},
        {53,58,97,167,188,352,376,431,446,522,555,589,614,632,662,703,726,751,939,951},
        {16,21,57,121,174,195,223,233,237,285,297,312,320,329,337,355,404,437,441,477,498,505,511,535,550,583,586,694,699,734,787,823,830,840,866,876,879,959,976,984},
        {14,15,33,36,44,74,99,135,138,143,154,172,173,182,183,184,199,202,220,265,266,268,278,306,308,330,371,374,391,412,417,427,447,452,469,472,478,509,538,541,542,545,553,566,576,582,592,603,623,644,653,668,675,700,720,728,733,736,744,752,756,772,777,799,843,848,860,877,888,940,969,970,982},
        {10,11,12,13,26,40,46,52,63,95,109,132,134,176,179,201,208,219,231,240,250,256,299,305,318,319,334,340,351,354,360,384,388,415,416,423,449,453,459,461,462,529,533,546,559,561,565,571,575,595,600,615,631,634,648,655,656,663,695,701,722,741,773,782,792,819,829,864,890,897,910,922,927,933,946,967,972,985,990,993},
        {19,20,32,45,75,111,120,146,186,229,262,263,364,399,430,432,450,464,482,489,493,508,591,602,616,624,647,661,667,671,739,813,867},
        {38,119,125,155,187,192,206,246,289,291,301,366,379,380,429,440,484,492,646,659,691,743,750,796,831,832,841,842,899,937,955,987},
        {239,249,343,405,407,503,652,654,707,786,852,920}};
    //}}}

    int j,k;
    for (j = 0; j < 23; ++j) {
        uint32_t i = giggle_get_chrm_id(gi, chrms[j]);
        struct uint32_t_ll *R = (struct uint32_t_ll *)
            giggle_search(gi->roots[i], 0, 3000000000);
        TEST_ASSERT_EQUAL(sizes[j], R->len); 
        for (k = 0; k < R->len; ++k) {
            TEST_ASSERT_EQUAL(1, uint32_t_ll_contains(R, ids[j][k]-1));
        }
    }
}
//}}}

//{{{void test_giggle_index_file(void)
void test_giggle_query_region(void)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

    char *file_name = "../data/1k.sort.bed.gz";

    ORDER = 10;

    struct giggle_index *gi = giggle_init_index(23);
    uint32_t num = giggle_index_file(gi, file_name);
    TEST_ASSERT_EQUAL(1000, num);
    TEST_ASSERT_EQUAL(1, gi->file_index->num);
    TEST_ASSERT_EQUAL(1000, gi->offset_index->num);


    struct uint32_t_ll *R = (struct uint32_t_ll *)giggle_query_region(gi,
                                                                      "chr11",
                                                                      1000,
                                                                      3000000);
    /*
     * tabix 1k.sort.bed.gz chr11:1000-3000000
     * chr11    575808  576604  .   1000    .   ...
     * chr11    2950239 2952321 .   1000    .   ...
     */

    TEST_ASSERT_EQUAL(2, R->len);

    struct file_id_offset_pair *r = 
        (struct file_id_offset_pair *)
        unordered_list_get(gi->offset_index, R->head->val);

    struct input_file *i = input_file_init(unordered_list_get(gi->file_index,
                                           r->file_id));

    input_file_seek(i, r->offset);

    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;
            
    int x = input_file_get_next_interval(i,
                                        &chrm,
                                        &chrm_len,
                                        &start,
                                        &end,
                                        &offset);

    TEST_ASSERT_EQUAL(0, strcmp("chr11", chrm));
    TEST_ASSERT_EQUAL(575808, start);
    TEST_ASSERT_EQUAL(576604, end);

    r = (struct file_id_offset_pair *)
            unordered_list_get(gi->offset_index, R->head->next->val);
    input_file_seek(i, r->offset);
    x = input_file_get_next_interval(i,
                                     &chrm,
                                     &chrm_len,
                                     &start,
                                     &end,
                                     &offset);

    TEST_ASSERT_EQUAL(0, strcmp("chr11", chrm));
    TEST_ASSERT_EQUAL(2950239, start);
    TEST_ASSERT_EQUAL(2952321, end);


    input_file_destroy(&i);

}
//}}}

//{{{void test_giggle_index_directory(void)
void test_giggle_index_directory(void)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

    char *path_name = "../data/many/*bed.gz";

    ORDER = 10;

    struct giggle_index *gi = giggle_init_index(23);
    
    uint32_t r = giggle_index_directory(gi, path_name, 0);

    TEST_ASSERT_EQUAL(10000, r);
    TEST_ASSERT_EQUAL(10, gi->file_index->num);
    TEST_ASSERT_EQUAL(10000, gi->offset_index->num);

    giggle_query_region(gi, "chr11", 1000, 3000000);

    struct uint32_t_ll *R = (struct uint32_t_ll *)giggle_query_region(gi,
                                                                      "chr1",
                                                                      1000,
                                                                      3000000);
    /*
     * ls *gz | xargs -I{} tabix {} chr1:1000-3000000 | wc -l
     * 4456
     */
    TEST_ASSERT_EQUAL(4456, R->len);
}
//}}}
