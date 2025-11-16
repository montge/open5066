/* test_util.c - Basic unit tests */

#include "unity.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/* Basic test to verify test framework works */
void test_framework_works(void) {
    TEST_ASSERT_EQUAL_INT(1, 1);
    TEST_ASSERT_TRUE(1 == 1);
}

/* Test NULL pointer checks */
void test_null_pointer_checks(void) {
    char* null_ptr = NULL;
    char* valid_ptr = "test";
    TEST_ASSERT_NULL(null_ptr);
    TEST_ASSERT_NOT_NULL(valid_ptr);
}

/* Test array bounds */
void test_array_bounds(void) {
    int array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    TEST_ASSERT_EQUAL_INT(0, array[0]);
    TEST_ASSERT_EQUAL_INT(9, array[9]);
    TEST_ASSERT_EQUAL_INT(10, sizeof(array)/sizeof(array[0]));
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_framework_works);
    RUN_TEST(test_null_pointer_checks);
    RUN_TEST(test_array_bounds);
    return UNITY_END();
}
