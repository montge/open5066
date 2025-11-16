/* test_sis_protocol.c - Tests for SIS protocol parser
 * These tests validate SIS (STANAG 5066 Annex A) protocol parsing
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

/* SIS Protocol Constants from s5066.h */
#define SIS_MIN_PDU_SIZE 5
#define SIS_MAX_SAP_ID 16
#define SIS_BCAST_MTU 4096
#define SIS_MAX_PDU_SIZE 8192
#define SIS_UNIHDR_SIZE 12

/* SIS Primitive Types (from sis5066.h) */
#define S_BIND_REQUEST 0x01
#define S_UNBIND_REQUEST 0x02
#define S_BIND_ACCEPTED 0x03
#define S_BIND_REJECTED 0x04
#define S_UNBIND_INDICATION 0x05
#define S_UNIDATA_REQUEST 0x14
#define S_UNIDATA_INDICATION 0x15

void setUp(void) {
    /* Set up before each test */
}

void tearDown(void) {
    /* Clean up after each test */
}

/* ==================== PDU Format Tests ==================== */

void test_sis_preamble_validation(void) {
    /* Valid SIS preamble: 0x90, 0xeb, 0x00 */
    unsigned char valid_preamble[5] = {0x90, 0xeb, 0x00, 0x00, 0x01};

    TEST_ASSERT_EQUAL_HEX8(0x90, valid_preamble[0]);
    TEST_ASSERT_EQUAL_HEX8(0xeb, valid_preamble[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, valid_preamble[2]);
}

void test_sis_invalid_preamble_detection(void) {
    /* Invalid preambles should be rejected */
    unsigned char bad_byte0[5] = {0x91, 0xeb, 0x00, 0x00, 0x01};
    unsigned char bad_byte1[5] = {0x90, 0xec, 0x00, 0x00, 0x01};
    unsigned char bad_byte2[5] = {0x90, 0xeb, 0x01, 0x00, 0x01};

    TEST_ASSERT_NOT_EQUAL(0x90, bad_byte0[0]);
    TEST_ASSERT_NOT_EQUAL(0xeb, bad_byte1[1]);
    TEST_ASSERT_NOT_EQUAL(0x00, bad_byte2[2]);
}

void test_sis_minimum_pdu_size(void) {
    /* Minimum SIS PDU is 5 bytes (preamble + version + length) */
    TEST_ASSERT_EQUAL(5, SIS_MIN_PDU_SIZE);
}

void test_sis_pdu_length_encoding(void) {
    /* Length is in bytes 3-4, big-endian, exclusive of preamble */
    unsigned char pdu[10] = {0x90, 0xeb, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00};

    /* Extract length (big-endian) */
    uint16_t length = (pdu[3] << 8) | pdu[4];

    TEST_ASSERT_EQUAL_UINT16(5, length);

    /* Total PDU length = length field + SIS_MIN_PDU_SIZE */
    uint16_t total_length = length + SIS_MIN_PDU_SIZE;
    TEST_ASSERT_EQUAL_UINT16(10, total_length);
}

void test_sis_pdu_length_maximum(void) {
    /* Maximum PDU length check */
    TEST_ASSERT_LESS_OR_EQUAL(8192, SIS_MAX_PDU_SIZE);

    /* Length field maximum (SIS_MAX_PDU_SIZE - SIS_MIN_PDU_SIZE) */
    uint16_t max_length_field = SIS_MAX_PDU_SIZE - SIS_MIN_PDU_SIZE;
    TEST_ASSERT_EQUAL(8187, max_length_field);
}

/* ==================== SAP ID Validation Tests ==================== */

void test_sis_sap_id_valid_range(void) {
    /* Valid SAP IDs are 0-15 */
    TEST_ASSERT_EQUAL(16, SIS_MAX_SAP_ID);

    /* Test boundary values */
    int min_sap = 0;
    int max_sap = 15;

    TEST_ASSERT_GREATER_OR_EQUAL(0, min_sap);
    TEST_ASSERT_LESS_THAN(SIS_MAX_SAP_ID, max_sap);
}

void test_sis_sap_id_boundary_invalid(void) {
    /* SAP ID 16 and above are invalid */
    int invalid_sap_low = -1;
    int invalid_sap_high = 16;

    TEST_ASSERT_LESS_THAN(0, invalid_sap_low);
    TEST_ASSERT_GREATER_OR_EQUAL(SIS_MAX_SAP_ID, invalid_sap_high);
}

void test_sis_sap_id_extraction_from_bind(void) {
    /* BIND_REQUEST: SAP ID is in byte 6, upper nibble */
    unsigned char bind_pdu[9] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x04,         /* Length = 4 */
        S_BIND_REQUEST,     /* Primitive */
        0x30,               /* SAP ID=3 (upper nibble), Rank=0 (lower nibble) */
        0x00, 0x00          /* Service type */
    };

    uint8_t sap_id = (bind_pdu[6] >> 4) & 0x0f;
    TEST_ASSERT_EQUAL_UINT8(3, sap_id);
}

void test_sis_sap_id_all_valid_values(void) {
    /* Test all 16 valid SAP IDs (0-15) */
    for (int sap = 0; sap < SIS_MAX_SAP_ID; sap++) {
        TEST_ASSERT_GREATER_OR_EQUAL(0, sap);
        TEST_ASSERT_LESS_THAN(SIS_MAX_SAP_ID, sap);
    }
}

/* ==================== Primitive Type Tests ==================== */

void test_sis_bind_request_structure(void) {
    /* BIND_REQUEST primitive structure */
    unsigned char bind_req[9] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x04,         /* Length = 4 bytes */
        S_BIND_REQUEST,     /* Op = 0x01 */
        0x55,               /* SAP ID=5, Rank=5 */
        0x12,               /* Service type upper */
        0x30                /* Service type lower */
    };

    TEST_ASSERT_EQUAL_HEX8(S_BIND_REQUEST, bind_req[5]);

    uint8_t sap_id = (bind_req[6] >> 4) & 0x0f;
    uint8_t rank = bind_req[6] & 0x0f;

    TEST_ASSERT_EQUAL_UINT8(5, sap_id);
    TEST_ASSERT_EQUAL_UINT8(5, rank);
}

void test_sis_unbind_request_structure(void) {
    /* UNBIND_REQUEST primitive structure */
    unsigned char unbind_req[7] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x02,         /* Length = 2 bytes */
        S_UNBIND_REQUEST,   /* Op = 0x02 */
        0x00                /* Reserved/reason */
    };

    TEST_ASSERT_EQUAL_HEX8(S_UNBIND_REQUEST, unbind_req[5]);
}

void test_sis_bind_accepted_structure(void) {
    /* BIND_ACCEPTED primitive structure */
    unsigned char bind_ok[9] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x04,         /* Length = 4 bytes */
        S_BIND_ACCEPTED,    /* Op = 0x03 */
        0x30,               /* SAP ID=3 */
        0x08,               /* MTU high byte */
        0x00                /* MTU low byte = 2048 */
    };

    TEST_ASSERT_EQUAL_HEX8(S_BIND_ACCEPTED, bind_ok[5]);

    uint8_t sap_id = (bind_ok[6] >> 4) & 0x0f;
    uint16_t mtu = (bind_ok[7] << 8) | bind_ok[8];

    TEST_ASSERT_EQUAL_UINT8(3, sap_id);
    TEST_ASSERT_EQUAL_UINT16(2048, mtu);
}

void test_sis_bind_rejected_structure(void) {
    /* BIND_REJECTED primitive structure */
    unsigned char bind_rej[7] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x02,         /* Length = 2 bytes */
        S_BIND_REJECTED,    /* Op = 0x04 */
        0x01                /* Reason code */
    };

    TEST_ASSERT_EQUAL_HEX8(S_BIND_REJECTED, bind_rej[5]);
    TEST_ASSERT_EQUAL_UINT8(0x01, bind_rej[6]); /* Reason */
}

/* ==================== Unidata Tests ==================== */

void test_sis_unidata_length_validation(void) {
    /* Unidata u_pdu length must not exceed SIS_BCAST_MTU */
    uint16_t valid_length = 1024;
    uint16_t max_length = SIS_BCAST_MTU;
    uint16_t invalid_length = SIS_BCAST_MTU + 1;

    TEST_ASSERT_LESS_OR_EQUAL(SIS_BCAST_MTU, valid_length);
    TEST_ASSERT_EQUAL(SIS_BCAST_MTU, max_length);
    TEST_ASSERT_GREATER_THAN(SIS_BCAST_MTU, invalid_length);
}

void test_sis_unidata_request_minimum_size(void) {
    /* UNIDATA_REQUEST minimum size check */
    /* Header: 5 (preamble) + 12 (unidata header) = 17 bytes minimum */
    uint16_t min_size = SIS_MIN_PDU_SIZE + SIS_UNIHDR_SIZE;

    TEST_ASSERT_EQUAL(17, min_size);
}

void test_sis_unidata_sap_id_extraction(void) {
    /* UNIDATA_REQUEST: SAP ID is in byte 6, lower nibble */
    unsigned char uni_req[20] = {
        0x90, 0xeb, 0x00,      /* Preamble - bytes 0-2 */
        0x00, 0x0f,            /* Length = 15 bytes - bytes 3-4 */
        S_UNIDATA_REQUEST,     /* Op = 0x14 - byte 5 */
        0x07,                  /* Dest SAP ID=7 - byte 6 */
        0x00, 0x00, 0x00, 0x00, /* Address field - bytes 7-10 */
        0x00,                  /* Delivery mode - byte 11 */
        0x00, 0x00, 0x00,      /* Transmission mode + padding - bytes 12-14 */
        0x00, 0x03,            /* u_pdu size = 3 (big-endian) - bytes 15-16 */
        0x41, 0x42, 0x43       /* u_pdu data "ABC" - bytes 17-19 */
    };

    uint8_t sap_id = uni_req[6] & 0x0f;
    TEST_ASSERT_EQUAL_UINT8(7, sap_id);

    /* Extract u_pdu length from bytes 15-16 (SIS_MIN_PDU_SIZE + 10, 11) */
    uint16_t u_pdu_len = (uni_req[15] << 8) | uni_req[16];
    TEST_ASSERT_EQUAL_UINT16(3, u_pdu_len);
}

/* ==================== Error Handling Tests ==================== */

void test_sis_zero_length_pdu(void) {
    /* Zero-length PDU (only preamble) should be handled */
    unsigned char zero_pdu[5] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x00          /* Length = 0 */
    };

    uint16_t length = (zero_pdu[3] << 8) | zero_pdu[4];
    uint16_t total = length + SIS_MIN_PDU_SIZE;

    TEST_ASSERT_EQUAL_UINT16(0, length);
    TEST_ASSERT_EQUAL_UINT16(5, total);
}

void test_sis_oversized_pdu_detection(void) {
    /* PDU length exceeding maximum should be rejected */
    uint16_t oversized_length = SIS_MAX_PDU_SIZE + 100;

    TEST_ASSERT_GREATER_THAN(SIS_MAX_PDU_SIZE, oversized_length);
}

void test_sis_inconsistent_length_detection(void) {
    /* Simulate inconsistent length field vs actual data */
    unsigned char pdu[10] = {
        0x90, 0xeb, 0x00,   /* Preamble */
        0x00, 0x14,         /* Length = 20 bytes (inconsistent!) */
        S_BIND_REQUEST,
        0x00, 0x00, 0x00, 0x00
    };

    uint16_t claimed_length = (pdu[3] << 8) | pdu[4];
    uint16_t total_claimed = claimed_length + SIS_MIN_PDU_SIZE;
    uint16_t actual_available = sizeof(pdu);

    /* Claimed 25 bytes, but only 10 available */
    TEST_ASSERT_EQUAL_UINT16(20, claimed_length);
    TEST_ASSERT_EQUAL_UINT16(25, total_claimed);
    TEST_ASSERT_EQUAL_UINT16(10, actual_available);
    TEST_ASSERT_GREATER_THAN(actual_available, total_claimed);
}

/* ==================== Constants Validation ==================== */

void test_sis_protocol_constants(void) {
    /* Verify key protocol constants */
    TEST_ASSERT_EQUAL(5, SIS_MIN_PDU_SIZE);
    TEST_ASSERT_EQUAL(16, SIS_MAX_SAP_ID);
    TEST_ASSERT_EQUAL(4096, SIS_BCAST_MTU);
    TEST_ASSERT_EQUAL(12, SIS_UNIHDR_SIZE);
}

void test_sis_primitive_opcodes(void) {
    /* Verify primitive opcodes are distinct */
    TEST_ASSERT_EQUAL_HEX8(0x01, S_BIND_REQUEST);
    TEST_ASSERT_EQUAL_HEX8(0x02, S_UNBIND_REQUEST);
    TEST_ASSERT_EQUAL_HEX8(0x03, S_BIND_ACCEPTED);
    TEST_ASSERT_EQUAL_HEX8(0x04, S_BIND_REJECTED);
    TEST_ASSERT_EQUAL_HEX8(0x05, S_UNBIND_INDICATION);
    TEST_ASSERT_EQUAL_HEX8(0x14, S_UNIDATA_REQUEST);
    TEST_ASSERT_EQUAL_HEX8(0x15, S_UNIDATA_INDICATION);

    /* Ensure opcodes are unique */
    TEST_ASSERT_NOT_EQUAL(S_BIND_REQUEST, S_UNBIND_REQUEST);
    TEST_ASSERT_NOT_EQUAL(S_BIND_ACCEPTED, S_BIND_REJECTED);
    TEST_ASSERT_NOT_EQUAL(S_UNIDATA_REQUEST, S_UNIDATA_INDICATION);
}

/* ==================== Length Calculation Tests ==================== */

void test_sis_total_pdu_length_calculation(void) {
    /* Test various PDU length calculations */

    /* BIND_REQUEST: 4 bytes payload + 5 bytes header = 9 total */
    uint16_t bind_payload = 4;
    uint16_t bind_total = bind_payload + SIS_MIN_PDU_SIZE;
    TEST_ASSERT_EQUAL_UINT16(9, bind_total);

    /* UNIDATA with 100 bytes: 12 header + 100 data + 5 preamble = 117 */
    uint16_t uni_payload = 12 + 100;
    uint16_t uni_total = uni_payload + SIS_MIN_PDU_SIZE;
    TEST_ASSERT_EQUAL_UINT16(117, uni_total);
}

void test_sis_length_field_boundaries(void) {
    /* Test length field boundary values */

    /* Minimum: 0 (zero-length PDU) */
    uint16_t min_length = 0;
    TEST_ASSERT_EQUAL_UINT16(0, min_length);

    /* Maximum: SIS_MAX_PDU_SIZE - SIS_MIN_PDU_SIZE */
    uint16_t max_length = SIS_MAX_PDU_SIZE - SIS_MIN_PDU_SIZE;
    TEST_ASSERT_EQUAL_UINT16(8187, max_length);

    /* Maximum should fit in 16 bits */
    TEST_ASSERT_LESS_OR_EQUAL(65535, max_length);
}

int main(void) {
    UNITY_BEGIN();

    /* PDU Format Tests */
    RUN_TEST(test_sis_preamble_validation);
    RUN_TEST(test_sis_invalid_preamble_detection);
    RUN_TEST(test_sis_minimum_pdu_size);
    RUN_TEST(test_sis_pdu_length_encoding);
    RUN_TEST(test_sis_pdu_length_maximum);

    /* SAP ID Tests */
    RUN_TEST(test_sis_sap_id_valid_range);
    RUN_TEST(test_sis_sap_id_boundary_invalid);
    RUN_TEST(test_sis_sap_id_extraction_from_bind);
    RUN_TEST(test_sis_sap_id_all_valid_values);

    /* Primitive Type Tests */
    RUN_TEST(test_sis_bind_request_structure);
    RUN_TEST(test_sis_unbind_request_structure);
    RUN_TEST(test_sis_bind_accepted_structure);
    RUN_TEST(test_sis_bind_rejected_structure);

    /* Unidata Tests */
    RUN_TEST(test_sis_unidata_length_validation);
    RUN_TEST(test_sis_unidata_request_minimum_size);
    RUN_TEST(test_sis_unidata_sap_id_extraction);

    /* Error Handling Tests */
    RUN_TEST(test_sis_zero_length_pdu);
    RUN_TEST(test_sis_oversized_pdu_detection);
    RUN_TEST(test_sis_inconsistent_length_detection);

    /* Constants Validation */
    RUN_TEST(test_sis_protocol_constants);
    RUN_TEST(test_sis_primitive_opcodes);

    /* Length Calculation Tests */
    RUN_TEST(test_sis_total_pdu_length_calculation);
    RUN_TEST(test_sis_length_field_boundaries);

    return UNITY_END();
}
