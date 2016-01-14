#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "unity.h"
#include "disk_store.h"

void setUp(void) { }
void tearDown(void) { }

void test_disk_store(void)
{
    char *file_name = "test_disk_store.out";
    FILE *f = NULL;
    struct disk_store *ds = disk_store_init(10, &f, file_name);

    TEST_ASSERT_EQUAL(0, ds->num);
    TEST_ASSERT_EQUAL(10, ds->size);

    uint32_t V[10] = {2,4,6,8,10,12,14,16,18,20}; 

    uint32_t id = disk_store_append(ds, V, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(0, id);

    id = disk_store_append(ds, V+1, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(1, id);

    id = disk_store_append(ds, V+2, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(2, id);

    disk_store_destroy(&ds);
    TEST_ASSERT_EQUAL(NULL, ds);

    f = NULL;

    ds = disk_store_load(&f, file_name);
 
    TEST_ASSERT_EQUAL(3, ds->num);
    TEST_ASSERT_EQUAL(10, ds->size);

    uint64_t size;

    uint32_t *v = disk_store_get(ds, 2, &size);
    TEST_ASSERT_EQUAL(V[2], *v);
    TEST_ASSERT_EQUAL(sizeof(uint32_t), size);

    id = disk_store_append(ds, V+3, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(3, id);

    id = disk_store_append(ds, V+4, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(4, id);

    disk_store_destroy(&ds);

    f = NULL;

    ds = disk_store_load(&f, file_name);
 
    TEST_ASSERT_EQUAL(5, ds->num);
    TEST_ASSERT_EQUAL(10, ds->size);

    free(v);

    v = disk_store_get(ds, 3, &size);
    TEST_ASSERT_EQUAL(V[3], *v);
    TEST_ASSERT_EQUAL(sizeof(uint32_t), size);

    free(v);

    v = disk_store_get(ds, 1, &size);
    TEST_ASSERT_EQUAL(V[1], *v);
    TEST_ASSERT_EQUAL(sizeof(uint32_t), size);

    disk_store_destroy(&ds);
    free(v);
}
