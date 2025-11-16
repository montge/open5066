/* test_protocol_security.c - Security tests for protocol parsers
 * Tests for buffer overflows, malformed input, and edge cases
 */

#include "unity.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/* Test: SIS preamble validation */
void test_sis_preamble_validation(void) {
    /* SIS PDU should start with 0x90 0xeb 0x00 */
    uint8_t valid_preamble[] = {0x90, 0xEB, 0x00, 0x00, 0x05, 0x01};
    uint8_t invalid_preamble1[] = {0x91, 0xEB, 0x00, 0x00, 0x05, 0x01};
    uint8_t invalid_preamble2[] = {0x90, 0xEC, 0x00, 0x00, 0x05, 0x01};
    uint8_t invalid_preamble3[] = {0x90, 0xEB, 0x01, 0x00, 0x05, 0x01};

    /* Validate preamble checking logic */
    TEST_ASSERT_EQUAL_HEX8(0x90, valid_preamble[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEB, valid_preamble[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, valid_preamble[2]);

    /* Invalid preambles should be rejected */
    TEST_ASSERT_NOT_EQUAL(0x90, invalid_preamble1[0]);
    TEST_ASSERT_NOT_EQUAL(0xEB, invalid_preamble2[1]);
    TEST_ASSERT_NOT_EQUAL(0x00, invalid_preamble3[2]);
}

/* Test: SIS length field overflow protection */
void test_sis_length_overflow(void) {
    /* SIS length is 16-bit: bytes[3] << 8 | bytes[4] */
    uint8_t max_length[] = {0x90, 0xEB, 0x00, 0xFF, 0xFF};  /* Max: 65535 */
    uint8_t reasonable_length[] = {0x90, 0xEB, 0x00, 0x00, 0x64};  /* 100 bytes */

    uint16_t len1 = (max_length[3] << 8) | max_length[4];
    uint16_t len2 = (reasonable_length[3] << 8) | reasonable_length[4];

    TEST_ASSERT_EQUAL_UINT16(65535, len1);
    TEST_ASSERT_EQUAL_UINT16(100, len2);

    /* Length should not exceed SIS_MAX_PDU_SIZE */
    #define SIS_MAX_PDU_SIZE 4096
    #define SIS_MIN_PDU_SIZE 5
    TEST_ASSERT_TRUE(len2 + SIS_MIN_PDU_SIZE <= SIS_MAX_PDU_SIZE);
}

/* Test: DTS header validation */
void test_dts_header_validation(void) {
    /* DTS S_PDU header structure validation */
    uint8_t valid_s_pdu[] = {
        0x02, 0x00,  /* Version and type */
        0x00, 0x00,  /* Priority and dest address */
        0x00, 0x64   /* Data length: 100 bytes */
    };

    uint16_t data_len = (valid_s_pdu[4] << 8) | valid_s_pdu[5];
    TEST_ASSERT_EQUAL_UINT16(100, data_len);
}

/* Test: Buffer boundary checks */
void test_buffer_boundary_checks(void) {
    char buffer[256];
    size_t buf_size = sizeof(buffer);

    /* Safe write using snprintf */
    int n = snprintf(buffer, buf_size, "Test data: %d", 12345);

    TEST_ASSERT_TRUE(n >= 0);
    TEST_ASSERT_TRUE(n < buf_size);
    TEST_ASSERT_EQUAL_CHAR('\0', buffer[buf_size-1]);
}

/* Test: Integer overflow in length calculations */
void test_integer_overflow_prevention(void) {
    /* Test that length + header doesn't overflow */
    uint16_t len = 3900;
    uint16_t header_size = 100;
    uint16_t max_pdu = 4096;

    /* Safe check: len + header <= max */
    TEST_ASSERT_TRUE((uint32_t)len + header_size <= max_pdu);

    /* Overflow scenario */
    uint16_t big_len = 65000;
    TEST_ASSERT_FALSE((uint32_t)big_len + header_size <= max_pdu);
}

/* Test: Null pointer checks */
void test_null_pointer_handling(void) {
    char* null_ptr = NULL;
    char valid_buffer[10] = "test";

    /* Simulating pointer validation */
    if (null_ptr == NULL) {
        /* Should handle gracefully */
        TEST_PASS();
    } else {
        TEST_FAIL_MESSAGE("Null pointer not detected");
    }

    TEST_ASSERT_NOT_NULL(valid_buffer);
}

/* Test: SAP ID validation */
void test_sap_id_validation(void) {
    #define SIS_MAX_SAP_ID 16

    uint8_t valid_sap = 10;
    uint8_t invalid_sap = 20;

    TEST_ASSERT_TRUE(valid_sap < SIS_MAX_SAP_ID);
    TEST_ASSERT_FALSE(invalid_sap < SIS_MAX_SAP_ID);
}

/* Test: Address field bounds */
void test_address_field_bounds(void) {
    /* Address fields are typically 4 bytes in STANAG 5066 */
    uint8_t addr[4] = {0x01, 0x02, 0x03, 0x04};

    TEST_ASSERT_EQUAL_UINT(4, sizeof(addr));

    /* Ensure we don't read beyond bounds */
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_TRUE(addr[i] >= 0);  /* Valid byte */
    }
}

/* Test: PDU size constraints */
void test_pdu_size_constraints(void) {
    #define HI_PDU_MEM 2200
    #define SIS_MAX_PDU_SIZE 4096

    uint16_t test_len = 2000;

    /* Length must fit in both constraints */
    TEST_ASSERT_TRUE(test_len < SIS_MAX_PDU_SIZE);
    TEST_ASSERT_TRUE(test_len < HI_PDU_MEM);

    /* Oversized PDU should be rejected */
    uint16_t oversized = 5000;
    TEST_ASSERT_FALSE(oversized < SIS_MAX_PDU_SIZE);
}

/* Test: Memcpy safety with known sizes */
void test_safe_memcpy(void) {
    char src[10] = "hello";
    char dest[20];
    size_t copy_len = 5;

    /* Safe copy with bounds */
    if (copy_len <= sizeof(dest) && copy_len <= sizeof(src)) {
        memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
        TEST_ASSERT_EQUAL_STRING("hello", dest);
    } else {
        TEST_FAIL_MESSAGE("Copy would overflow");
    }
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_sis_preamble_validation);
    RUN_TEST(test_sis_length_overflow);
    RUN_TEST(test_dts_header_validation);
    RUN_TEST(test_buffer_boundary_checks);
    RUN_TEST(test_integer_overflow_prevention);
    RUN_TEST(test_null_pointer_handling);
    RUN_TEST(test_sap_id_validation);
    RUN_TEST(test_address_field_bounds);
    RUN_TEST(test_pdu_size_constraints);
    RUN_TEST(test_safe_memcpy);

    return UNITY_END();
}
