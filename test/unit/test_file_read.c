#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <htslib/bgzf.h>
#include <htslib/tbx.h>

#include "unity.h"
#include "bpt.h"
#include "giggle.h"
#include "file_read.h"

void setUp(void) { }
void tearDown(void) { }

//{{{void test_init_file(void)
void test_init_file(void)
{
    struct input_file *i = input_file_init("../data/1k.sort.bed.gz");
    input_file_destroy(&i);

    TEST_ASSERT_EQUAL(NULL, i);
}
//}}}

//{{{void test_bed_file_read(void)
void test_bed_file_read(void)
{

    struct input_file *i = input_file_init("../data/1k.unsort.bed.gz");

    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;


    char *chrm_A[10] = {"chr11",
                        "chr11",  
                        "chr10",  
                        "chr16",  
                        "chr15",  
                        "chr19",  
                        "chr19",  
                        "chr18",  
                        "chr21",  
                        "chr7"};

    uint32_t start_A[10] = {64691252,
                             129871988,
                             74031859, 
                             3070038,
                             93346819,  
                             4374094,
                             42805980,
                             3602738,
                             9825304,
                             2393484};


    uint32_t end_A[10] = {64692359,
                           129873775,
                           74037598,
                           3072761,
                           93347932,
                           4376369,
                           42807400,
                           3605403,
                           9827741,
                           2394629};

    int j;
    for (j = 0; j < 10; ++j) {
        int ret = input_file_get_next_interval(i,
                                              &chrm,
                                              &chrm_len,
                                              &start,
                                              &end,
                                              &offset);
        TEST_ASSERT_EQUAL(0,strcmp(chrm_A[j], chrm));
        TEST_ASSERT_EQUAL(start_A[j], start);
        TEST_ASSERT_EQUAL(end_A[j], end);
    }

    while (input_file_get_next_interval(i,
                                        &chrm,
                                        &chrm_len,
                                        &start,
                                        &end,
                                        &offset) >= 0) {
        ++j;
    }

    TEST_ASSERT_EQUAL(0,strcmp("chr1", chrm));
    TEST_ASSERT_EQUAL(25895359, start);
    TEST_ASSERT_EQUAL(25896171, end);

    TEST_ASSERT_EQUAL(1000, j);

    input_file_destroy(&i);
}
//}}}
