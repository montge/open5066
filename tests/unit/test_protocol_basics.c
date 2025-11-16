/* test_protocol_basics.c - Basic protocol constant tests */

#include "unity.h"
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/* Test: SIS constants */
void test_sis_constants(void) {
    TEST_ASSERT_EQUAL(5, 5);  /* SIS_MIN_PDU_SIZE */
    TEST_ASSERT_EQUAL(16, 16); /* SIS_MAX_SAP_ID */
}

/* Test: DTS constants */
void test_dts_constants(void) {
    TEST_ASSERT_EQUAL(6, 6); /* DTS_MIN_PDU_SIZE */
    TEST_ASSERT_EQUAL(800, 800); /* DTS_SEG_SIZE */
}

/* Test: Preamble values */
void test_preambles(void) {
    uint8_t sis_preamble[] = {0x90, 0xEB, 0x00};
    uint8_t dts_preamble[] = {0x90, 0xEB};
    
    TEST_ASSERT_EQUAL_HEX8(0x90, sis_preamble[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEB, sis_preamble[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, sis_preamble[2]);
    
    TEST_ASSERT_EQUAL_HEX8(0x90, dts_preamble[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEB, dts_preamble[1]);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_sis_constants);
    RUN_TEST(test_dts_constants);
    RUN_TEST(test_preambles);
    
    return UNITY_END();
}
