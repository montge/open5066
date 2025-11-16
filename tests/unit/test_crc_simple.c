/* test_crc_simple.c - CRC conceptual tests */

#include "unity.h"
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/* Test: CRC-16 polynomial */
void test_crc16_polynomial(void) {
    uint16_t polynomial = 0x9299;
    TEST_ASSERT_EQUAL_HEX16(0x9299, polynomial);
}

/* Test: CRC-32 polynomial */
void test_crc32_polynomial(void) {
    uint32_t polynomial = 0xf3a4e550;
    TEST_ASSERT_EQUAL_HEX32(0xf3a4e550, polynomial);
}

/* Test: CRC sizes */
void test_crc_sizes(void) {
    TEST_ASSERT_EQUAL(2, sizeof(uint16_t));
    TEST_ASSERT_EQUAL(4, sizeof(uint32_t));
}

/* Test: Polynomial operations */
void test_polynomial_operations(void) {
    uint16_t poly16 = 0x9299;
    uint16_t test = 0xFFFF;
    test ^= poly16;
    TEST_ASSERT_EQUAL_HEX16(0x6D66, test);
}

/* Test: Byte extraction */
void test_byte_extraction(void) {
    uint16_t value = 0x1234;
    uint8_t high = (value >> 8) & 0xFF;
    uint8_t low = value & 0xFF;
    
    TEST_ASSERT_EQUAL_HEX8(0x12, high);
    TEST_ASSERT_EQUAL_HEX8(0x34, low);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_crc16_polynomial);
    RUN_TEST(test_crc32_polynomial);
    RUN_TEST(test_crc_sizes);
    RUN_TEST(test_polynomial_operations);
    RUN_TEST(test_byte_extraction);
    
    return UNITY_END();
}
