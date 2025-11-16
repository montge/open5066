/* test_string_safety.c - Tests for safe string handling */

#include "unity.h"
#include <string.h>
#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

/* Test: snprintf vs sprintf */
void test_snprintf_safe(void) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "Hello %s", "World");
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
}

/* Test: snprintf prevents overflow */
void test_snprintf_overflow_prevention(void) {
    char small[5];
    const char* big_string = "This is way too long";
    snprintf(small, sizeof(small), "%s", big_string);
    TEST_ASSERT_EQUAL_INT(4, strlen(small));
    TEST_ASSERT_EQUAL_STRING("This", small);
}

/* Test: strncpy safe usage */
void test_strncpy_safe(void) {
    char dest[10];
    const char* src = "Hello";
    memset(dest, 0, sizeof(dest));
    strncpy(dest, src, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
    TEST_ASSERT_EQUAL_STRING("Hello", dest);
}

/* Test: strncat safe concatenation */
void test_strncat_safe(void) {
    char buffer[20] = "Hello ";
    const char* append = "World";
    strncat(buffer, append, sizeof(buffer) - strlen(buffer) - 1);
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_snprintf_safe);
    RUN_TEST(test_snprintf_overflow_prevention);
    RUN_TEST(test_strncpy_safe);
    RUN_TEST(test_strncat_safe);
    return UNITY_END();
}
