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
#include "lists.h"

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
        int ret = i->input_file_get_next_interval(i,
                                                  &chrm,
                                                  &chrm_len,
                                                  &start,
                                                  &end,
                                                  &offset);
        TEST_ASSERT_EQUAL(0,strcmp(chrm_A[j], chrm));
        TEST_ASSERT_EQUAL(start_A[j], start);
        TEST_ASSERT_EQUAL(end_A[j], end);
    }

    while (i->input_file_get_next_interval(i,
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

//{{{void test_get_file_stats(void)
void test_get_file_stats(void)
{

    struct input_file *i = input_file_init("../data/1k.unsort.bed.gz");
    struct unordered_list *file_index = unordered_list_init(1);


    struct file_data *fd = (struct file_data *)
        calloc(1, sizeof(struct file_data));

    uint32_t file_id = unordered_list_add(file_index, fd);

    fd->file_name = strdup("../data/1k.unsort.bed.gz");
    fd->num_intervals = 0;
    fd->mean_interval_size = 0;

    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;

    uint32_t j = 0;

    struct file_id_offset_pair *p;
    uint32_t intrv_id;

    while (i->input_file_get_next_interval(i,
                                           &chrm,
                                           &chrm_len,
                                           &start,
                                           &end,
                                           &offset) >= 0) {
        fd->mean_interval_size += end-start;
        fd->num_intervals += 1;
    }

    fd->mean_interval_size = fd->mean_interval_size/fd->num_intervals;
    input_file_destroy(&i);
    free(chrm);

    char *out_file_name = "test_file_data_read_write.tmp";

    FILE *f = fopen(out_file_name, "wb");
    unordered_list_store(file_index, f, out_file_name, file_data_store);
    fclose(f);

    f = fopen(out_file_name, "rb");
    struct unordered_list *file_index_r = 
        unordered_list_load(f,
                            out_file_name,
                            file_data_load);

    struct file_data *fd_r = (struct file_data *)
            unordered_list_get(file_index_r, file_id);

    TEST_ASSERT_EQUAL(0, strcmp(fd->file_name, fd_r->file_name));
    TEST_ASSERT_EQUAL(fd->num_intervals, fd_r->num_intervals);
    TEST_ASSERT_EQUAL(fd->mean_interval_size, fd_r->mean_interval_size);

    unordered_list_destroy(&file_index, file_data_free);
    unordered_list_destroy(&file_index_r, file_data_free);

    remove(out_file_name);
}
//}}}

//{{{void test_get_file_stats(void)
void test_input_file_get_next_interval_vcf(void)
{
#if 0
    struct input_file *i = input_file_init("../data/1k.vcf.gz");
    int chrm_len = 10;
    char *chrm = (char *)malloc(chrm_len*sizeof(char));
    uint32_t start, end;
    long offset;

    while (i->input_file_get_next_interval(i,
                                           &chrm,
                                           &chrm_len,
                                           &start,
                                           &end,
                                           &offset) >= 0) {
        offset += 1;
    }
#endif
}
//}}}
