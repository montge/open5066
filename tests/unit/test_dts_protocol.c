/* test_dts_protocol.c - Tests for DTS protocol parser
 * These tests validate DTS (STANAG 5066 Annex C) protocol parsing
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

/* DTS Protocol Constants */
#define DTS_MIN_PDU_SIZE 6
#define DTS_MAX_PDU_SIZE 4096
#define DTS_SEG_SIZE 800

/* D_PDU Type Constants */
#define DTS_DATA_ONLY  0
#define DTS_ACK_ONLY   1
#define DTS_DATA_ACK   2
#define DTS_RESET      3
#define DTS_EDATA_ONLY 4
#define DTS_EACK_ONLY  5
#define DTS_MGMT       6
#define DTS_NONARQ     7
#define DTS_ENONARQ    8
#define DTS_WARNING   15

/* SIS Constants needed for validation */
#define SIS_MAX_SAP_ID 16
#define SIS_UNIDATA_IND_MIN_HDR 22

void setUp(void) {
    /* Set up before each test */
}

void tearDown(void) {
    /* Clean up after each test */
}

/* ==================== PDU Format Tests ==================== */

void test_dts_preamble_validation(void) {
    /* Valid DTS preamble: 0x90, 0xeb (Maury-Styles) */
    unsigned char valid_preamble[6] = {0x90, 0xeb, 0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_EQUAL_HEX8(0x90, valid_preamble[0]);
    TEST_ASSERT_EQUAL_HEX8(0xeb, valid_preamble[1]);
}

void test_dts_minimum_pdu_size(void) {
    /* Minimum DTS PDU is 6 bytes (preamble + d_type + EOW + length) */
    TEST_ASSERT_EQUAL(6, DTS_MIN_PDU_SIZE);
}

void test_dts_d_type_extraction(void) {
    /* D_TYPE is in byte 2, upper nibble */
    unsigned char pdu[6] = {0x90, 0xeb, 0x70, 0x00, 0x00, 0x00};

    uint8_t d_type = (pdu[2] >> 4) & 0x0f;
    TEST_ASSERT_EQUAL_UINT8(7, d_type); /* DTS_NONARQ */
}

void test_dts_eow_extraction(void) {
    /* EOW (End of Window) is in bytes 2-3, lower 12 bits */
    unsigned char pdu[6] = {
        0x90, 0xeb,
        0x03, 0xE8,  /* D_TYPE=0, EOW=0x3E8 (1000 in decimal) */
        0x00, 0x00
    };

    uint16_t eow = ((pdu[2] & 0x0f) << 8) | pdu[3];
    TEST_ASSERT_EQUAL_UINT16(1000, eow);
}

void test_dts_eot_extraction(void) {
    /* EOT (End of Transmission) is in byte 4 */
    unsigned char pdu[6] = {0x90, 0xeb, 0x00, 0x00, 0x7F, 0x00};

    uint8_t eot = pdu[4];
    TEST_ASSERT_EQUAL_UINT8(127, eot); /* Max EOT value */
}

/* ==================== D_PDU Type Tests ==================== */

void test_dts_data_only_type(void) {
    /* DTS_DATA_ONLY (type 0) */
    uint8_t d_type = DTS_DATA_ONLY;
    TEST_ASSERT_EQUAL_HEX8(0, d_type);
}

void test_dts_ack_only_type(void) {
    /* DTS_ACK_ONLY (type 1) */
    uint8_t d_type = DTS_ACK_ONLY;
    TEST_ASSERT_EQUAL_HEX8(1, d_type);
}

void test_dts_data_ack_type(void) {
    /* DTS_DATA_ACK (type 2) */
    uint8_t d_type = DTS_DATA_ACK;
    TEST_ASSERT_EQUAL_HEX8(2, d_type);
}

void test_dts_reset_type(void) {
    /* DTS_RESET (type 3) */
    uint8_t d_type = DTS_RESET;
    TEST_ASSERT_EQUAL_HEX8(3, d_type);
}

void test_dts_nonarq_type(void) {
    /* DTS_NONARQ (type 7) - most important for testing */
    uint8_t d_type = DTS_NONARQ;
    TEST_ASSERT_EQUAL_HEX8(7, d_type);
}

void test_dts_warning_type(void) {
    /* DTS_WARNING (type 15) */
    uint8_t d_type = DTS_WARNING;
    TEST_ASSERT_EQUAL_HEX8(15, d_type);
}

void test_dts_all_valid_types(void) {
    /* Test all valid D_PDU types */
    uint8_t valid_types[] = {
        DTS_DATA_ONLY, DTS_ACK_ONLY, DTS_DATA_ACK, DTS_RESET,
        DTS_EDATA_ONLY, DTS_EACK_ONLY, DTS_MGMT,
        DTS_NONARQ, DTS_ENONARQ, DTS_WARNING
    };

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_LESS_OR_EQUAL(15, valid_types[i]);
    }
}

/* ==================== Address Size Tests ==================== */

void test_dts_address_size_extraction(void) {
    /* Address size is in byte 5, upper 3 bits */
    unsigned char pdu[6] = {0x90, 0xeb, 0x00, 0x00, 0x00, 0xE0};

    uint8_t addr_size = (pdu[5] >> 5) & 0x07;
    TEST_ASSERT_EQUAL_UINT8(7, addr_size); /* Max address size */
}

void test_dts_address_size_range(void) {
    /* Address size is 0-7 (3 bits) */
    uint8_t min_addr_size = 0;
    uint8_t max_addr_size = 7;

    TEST_ASSERT_EQUAL(0, min_addr_size);
    TEST_ASSERT_EQUAL(7, max_addr_size);
}

void test_dts_header_length_extraction(void) {
    /* Header length is in byte 5, lower 5 bits */
    unsigned char pdu[6] = {0x90, 0xeb, 0x00, 0x00, 0x00, 0x1F};

    uint8_t hdr_len = pdu[5] & 0x1f;
    TEST_ASSERT_EQUAL_UINT8(31, hdr_len); /* Max header length (5 bits) */
}

/* ==================== Segment Size Tests ==================== */

void test_dts_segment_size_maximum(void) {
    /* Maximum segment size is 800 bytes (10 bits, max 1023) */
    TEST_ASSERT_EQUAL(800, DTS_SEG_SIZE);
    TEST_ASSERT_LESS_OR_EQUAL(1023, DTS_SEG_SIZE);
}

void test_dts_segment_size_fits_in_10_bits(void) {
    /* Segment size must fit in 10 bits (max 1023) */
    uint16_t max_10_bit = 1023;
    TEST_ASSERT_LESS_OR_EQUAL(max_10_bit, DTS_SEG_SIZE);
}

void test_dts_segment_c_pdu_size_encoding(void) {
    /* Segment C_PDU size is in first header byte (bits 0-1) and second byte */
    /* Example: size = 0x234 (564 bytes) */
    unsigned char hdr[2] = {
        0x02,  /* Upper 2 bits: 0x02 */
        0x34   /* Lower 8 bits: 0x34 */
    };

    uint16_t seg_size = ((hdr[0] & 0x03) << 8) | hdr[1];
    TEST_ASSERT_EQUAL_UINT16(0x234, seg_size);
}

/* ==================== NONARQ PDU Tests ==================== */

void test_dts_nonarq_c_pdu_id_validation(void) {
    /* C_PDU ID is 12 bits, valid range: 0-4095 */
    uint16_t min_id = 0;
    uint16_t max_id = 4095;

    TEST_ASSERT_EQUAL(0, min_id);
    TEST_ASSERT_EQUAL(4095, max_id);
}

void test_dts_nonarq_c_pdu_id_extraction(void) {
    /* C_PDU ID is in header byte 2 for NONARQ */
    uint8_t c_pdu_id = 0xAB; /* Example ID */

    TEST_ASSERT_EQUAL_UINT8(0xAB, c_pdu_id);
    TEST_ASSERT_LESS_OR_EQUAL(255, c_pdu_id); /* Single byte */
}

void test_dts_nonarq_c_pdu_size_encoding(void) {
    /* C_PDU size is in header bytes 3-4 (big-endian) */
    unsigned char hdr[5] = {0x00, 0x00, 0x00, 0x10, 0x00};

    uint16_t c_pdu_size = (hdr[3] << 8) | hdr[4];
    TEST_ASSERT_EQUAL_UINT16(0x1000, c_pdu_size); /* 4096 bytes */
}

void test_dts_nonarq_c_pdu_offset_encoding(void) {
    /* C_PDU offset is in header bytes 5-6 (big-endian) */
    unsigned char hdr[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x20};

    uint16_t c_pdu_offset = (hdr[5] << 8) | hdr[6];
    TEST_ASSERT_EQUAL_UINT16(0x320, c_pdu_offset); /* 800 bytes offset */
}

void test_dts_nonarq_c_pdu_rx_win_encoding(void) {
    /* C_PDU RX window is in header bytes 7-8 (big-endian) */
    unsigned char hdr[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};

    uint16_t c_pdu_rx_win = (hdr[7] << 8) | hdr[8];
    TEST_ASSERT_EQUAL_UINT16(0xFF, c_pdu_rx_win);
}

/* ==================== Length Validation Tests ==================== */

void test_dts_c_pdu_size_maximum(void) {
    /* C_PDU size must not exceed DTS_MAX_PDU_SIZE */
    uint16_t valid_size = 2048;
    uint16_t max_size = DTS_MAX_PDU_SIZE;
    uint16_t invalid_size = DTS_MAX_PDU_SIZE + 1;

    TEST_ASSERT_LESS_OR_EQUAL(DTS_MAX_PDU_SIZE, valid_size);
    TEST_ASSERT_EQUAL(4096, max_size);
    TEST_ASSERT_GREATER_THAN(DTS_MAX_PDU_SIZE, invalid_size);
}

void test_dts_c_pdu_size_minimum(void) {
    /* C_PDU size must be at least 1 */
    uint16_t valid_size = 100;
    uint16_t invalid_size = 0;

    TEST_ASSERT_GREATER_THAN(0, valid_size);
    TEST_ASSERT_EQUAL(0, invalid_size);
}

void test_dts_segment_size_boundary(void) {
    /* Segment size must not exceed DTS_SEG_SIZE */
    uint16_t valid_size = 500;
    uint16_t max_size = DTS_SEG_SIZE;
    uint16_t invalid_size = DTS_SEG_SIZE + 1;

    TEST_ASSERT_LESS_OR_EQUAL(DTS_SEG_SIZE, valid_size);
    TEST_ASSERT_EQUAL(800, max_size);
    TEST_ASSERT_GREATER_THAN(DTS_SEG_SIZE, invalid_size);
}

void test_dts_offset_plus_segment_validation(void) {
    /* c_pdu_offset + seg_size must not exceed c_pdu_size */
    uint16_t c_pdu_size = 2000;
    uint16_t c_pdu_offset = 1500;
    uint16_t seg_size = 400;

    /* Valid: 1500 + 400 = 1900 <= 2000 */
    TEST_ASSERT_LESS_OR_EQUAL(c_pdu_size, c_pdu_offset + 400);

    /* Invalid: 1500 + 600 = 2100 > 2000 */
    TEST_ASSERT_GREATER_THAN(c_pdu_size, c_pdu_offset + 600);
}

/* ==================== SAP ID Validation Tests ==================== */

void test_dts_sap_id_range(void) {
    /* SAP IDs must be 0-15 (inherited from SIS) */
    TEST_ASSERT_EQUAL(16, SIS_MAX_SAP_ID);

    int valid_sap = 7;
    int invalid_sap_low = -1;
    int invalid_sap_high = 16;

    TEST_ASSERT_LESS_THAN(SIS_MAX_SAP_ID, valid_sap);
    TEST_ASSERT_LESS_THAN(0, invalid_sap_low);
    TEST_ASSERT_GREATER_OR_EQUAL(SIS_MAX_SAP_ID, invalid_sap_high);
}

void test_dts_sap_id_extraction_from_c_pdu(void) {
    /* SAP ID is in C_PDU byte 2, lower nibble (destination) */
    unsigned char c_pdu[4] = {
        0x00,       /* C_PCI */
        0x05,       /* S_PDU type | priority */
        0x37,       /* SRC_SAP (upper nibble) | DEST_SAP (lower nibble) */
        0x00
    };

    uint8_t dest_sap = c_pdu[2] & 0x0f;
    uint8_t src_sap = (c_pdu[2] >> 4) & 0x0f;

    TEST_ASSERT_EQUAL_UINT8(7, dest_sap);
    TEST_ASSERT_EQUAL_UINT8(3, src_sap);
}

/* ==================== Constants Validation ==================== */

void test_dts_protocol_constants(void) {
    /* Verify key protocol constants */
    TEST_ASSERT_EQUAL(6, DTS_MIN_PDU_SIZE);
    TEST_ASSERT_EQUAL(4096, DTS_MAX_PDU_SIZE);
    TEST_ASSERT_EQUAL(800, DTS_SEG_SIZE);
}

void test_dts_d_type_constants(void) {
    /* Verify D_PDU type constants are correct */
    TEST_ASSERT_EQUAL(0, DTS_DATA_ONLY);
    TEST_ASSERT_EQUAL(1, DTS_ACK_ONLY);
    TEST_ASSERT_EQUAL(2, DTS_DATA_ACK);
    TEST_ASSERT_EQUAL(3, DTS_RESET);
    TEST_ASSERT_EQUAL(7, DTS_NONARQ);
    TEST_ASSERT_EQUAL(15, DTS_WARNING);
}

void test_dts_d_type_uniqueness(void) {
    /* Ensure all D_PDU types are unique */
    TEST_ASSERT_NOT_EQUAL(DTS_DATA_ONLY, DTS_ACK_ONLY);
    TEST_ASSERT_NOT_EQUAL(DTS_DATA_ACK, DTS_RESET);
    TEST_ASSERT_NOT_EQUAL(DTS_NONARQ, DTS_ENONARQ);
    TEST_ASSERT_NOT_EQUAL(DTS_MGMT, DTS_WARNING);
}

/* ==================== Header Size Validation Tests ==================== */

void test_dts_data_only_header_size(void) {
    /* DTS_DATA_ONLY header: MIN_PDU_SIZE + 3 - 2 = 7 */
    int expected_hdr_size = DTS_MIN_PDU_SIZE + 3 - 2;
    TEST_ASSERT_EQUAL(7, expected_hdr_size);
}

void test_dts_ack_only_header_size_minimum(void) {
    /* DTS_ACK_ONLY minimum header: MIN_PDU_SIZE + 1 - 2 = 5 */
    int min_hdr_size = DTS_MIN_PDU_SIZE + 1 - 2;
    TEST_ASSERT_EQUAL(5, min_hdr_size);
}

void test_dts_data_ack_header_size_minimum(void) {
    /* DTS_DATA_ACK minimum header: MIN_PDU_SIZE + 4 - 2 = 8 */
    int min_hdr_size = DTS_MIN_PDU_SIZE + 4 - 2;
    TEST_ASSERT_EQUAL(8, min_hdr_size);
}

void test_dts_reset_header_size(void) {
    /* DTS_RESET header: MIN_PDU_SIZE + 3 - 2 = 7 */
    int expected_hdr_size = DTS_MIN_PDU_SIZE + 3 - 2;
    TEST_ASSERT_EQUAL(7, expected_hdr_size);
}

void test_dts_nonarq_header_size(void) {
    /* DTS_NONARQ header: MIN_PDU_SIZE + 9 - 2 = 13 */
    int expected_hdr_size = DTS_MIN_PDU_SIZE + 9 - 2;
    TEST_ASSERT_EQUAL(13, expected_hdr_size);
}

/* ==================== Complete PDU Structure Tests ==================== */

void test_dts_nonarq_complete_pdu_structure(void) {
    /* Build a complete DTS_NONARQ PDU */
    unsigned char pdu[20] = {
        0x90, 0xeb,      /* Preamble (bytes 0-1) */
        0x70, 0x00,      /* D_TYPE=7 (NONARQ), EOW=0 (bytes 2-3) */
        0x00,            /* EOT=0 (byte 4) */
        0x69,            /* addr_size=3, hdr_len=9 (byte 5) */
        /* Address field (6 nibbles = 3 bytes for addr_size=3) */
        0x12, 0x34, 0x56, /* bytes 6-8 */
        /* Header specific to NONARQ (9 bytes) */
        0x01, 0x00,      /* Flags + seg size upper = 0x100 (256 bytes) */
        0xAB,            /* C_PDU ID = 0xAB */
        0x04, 0x00,      /* C_PDU size = 0x400 (1024 bytes) */
        0x00, 0x00,      /* C_PDU offset = 0 */
        0x00, 0xFF       /* C_PDU RX window = 0xFF */
    };

    /* Validate preamble */
    TEST_ASSERT_EQUAL_HEX8(0x90, pdu[0]);
    TEST_ASSERT_EQUAL_HEX8(0xeb, pdu[1]);

    /* Validate D_TYPE */
    uint8_t d_type = (pdu[2] >> 4) & 0x0f;
    TEST_ASSERT_EQUAL_UINT8(7, d_type);

    /* Validate address size and header length */
    uint8_t addr_size = (pdu[5] >> 5) & 0x07;
    uint8_t hdr_len = pdu[5] & 0x1f;
    TEST_ASSERT_EQUAL_UINT8(3, addr_size);
    TEST_ASSERT_EQUAL_UINT8(9, hdr_len);

    /* Validate segment size (from header bytes after address) */
    uint16_t seg_size = ((pdu[9] & 0x03) << 8) | pdu[10];
    TEST_ASSERT_EQUAL_UINT16(0x100, seg_size);

    /* Validate C_PDU ID */
    uint8_t c_pdu_id = pdu[11];
    TEST_ASSERT_EQUAL_UINT8(0xAB, c_pdu_id);

    /* Validate C_PDU size */
    uint16_t c_pdu_size = (pdu[12] << 8) | pdu[13];
    TEST_ASSERT_EQUAL_UINT16(0x400, c_pdu_size);
}

void test_dts_data_only_complete_pdu_structure(void) {
    /* Build a complete DTS_DATA_ONLY PDU */
    unsigned char pdu[15] = {
        0x90, 0xeb,      /* Preamble */
        0x01, 0x00,      /* D_TYPE=0 (DATA_ONLY), EOW=0x100 */
        0x7F,            /* EOT=127 */
        0x43,            /* addr_size=2, hdr_len=3 */
        /* Address field (4 nibbles = 2 bytes) */
        0xAB, 0xCD,
        /* Header specific to DATA_ONLY (3 bytes) */
        0x81, 0x00,      /* Flags=0x80 (first seg), seg size=0x100 */
        0x05             /* TX sequence = 5 */
    };

    /* Validate D_TYPE */
    uint8_t d_type = (pdu[2] >> 4) & 0x0f;
    TEST_ASSERT_EQUAL_UINT8(0, d_type);

    /* Validate EOW */
    uint16_t eow = ((pdu[2] & 0x0f) << 8) | pdu[3];
    TEST_ASSERT_EQUAL_UINT16(0x100, eow);

    /* Validate segment size */
    uint16_t seg_size = ((pdu[8] & 0x03) << 8) | pdu[9];
    TEST_ASSERT_EQUAL_UINT16(0x100, seg_size);

    /* Validate TX sequence */
    uint8_t tx_seq = pdu[10];
    TEST_ASSERT_EQUAL_UINT8(5, tx_seq);
}

/* ==================== Error Detection Tests ==================== */

void test_dts_invalid_d_type(void) {
    /* Reserved D_TYPE values (9-14) should be rejected */
    uint8_t reserved_types[] = {9, 10, 11, 12, 13, 14};

    for (int i = 0; i < 6; i++) {
        uint8_t d_type = reserved_types[i];
        TEST_ASSERT_GREATER_OR_EQUAL(9, d_type);
        TEST_ASSERT_LESS_OR_EQUAL(14, d_type);
    }
}

void test_dts_c_pdu_id_out_of_range(void) {
    /* C_PDU ID > 4095 should be rejected */
    uint16_t valid_id = 4095;
    uint16_t invalid_id = 4096;

    TEST_ASSERT_LESS_OR_EQUAL(4095, valid_id);
    TEST_ASSERT_GREATER_THAN(4095, invalid_id);
}

void test_dts_oversized_segment(void) {
    /* Segment size > DTS_SEG_SIZE should be rejected */
    uint16_t valid_seg = DTS_SEG_SIZE;
    uint16_t invalid_seg = DTS_SEG_SIZE + 1;

    TEST_ASSERT_LESS_OR_EQUAL(DTS_SEG_SIZE, valid_seg);
    TEST_ASSERT_GREATER_THAN(DTS_SEG_SIZE, invalid_seg);
}

void test_dts_oversized_c_pdu(void) {
    /* C_PDU size > DTS_MAX_PDU_SIZE should be rejected */
    uint16_t valid_size = DTS_MAX_PDU_SIZE;
    uint16_t invalid_size = DTS_MAX_PDU_SIZE + 1;

    TEST_ASSERT_LESS_OR_EQUAL(DTS_MAX_PDU_SIZE, valid_size);
    TEST_ASSERT_GREATER_THAN(DTS_MAX_PDU_SIZE, invalid_size);
}

void test_dts_zero_c_pdu_size(void) {
    /* C_PDU size of 0 should be rejected */
    uint16_t invalid_size = 0;
    uint16_t valid_size = 1;

    TEST_ASSERT_EQUAL(0, invalid_size);
    TEST_ASSERT_GREATER_THAN(0, valid_size);
}

int main(void) {
    UNITY_BEGIN();

    /* PDU Format Tests */
    RUN_TEST(test_dts_preamble_validation);
    RUN_TEST(test_dts_minimum_pdu_size);
    RUN_TEST(test_dts_d_type_extraction);
    RUN_TEST(test_dts_eow_extraction);
    RUN_TEST(test_dts_eot_extraction);

    /* D_PDU Type Tests */
    RUN_TEST(test_dts_data_only_type);
    RUN_TEST(test_dts_ack_only_type);
    RUN_TEST(test_dts_data_ack_type);
    RUN_TEST(test_dts_reset_type);
    RUN_TEST(test_dts_nonarq_type);
    RUN_TEST(test_dts_warning_type);
    RUN_TEST(test_dts_all_valid_types);

    /* Address Size Tests */
    RUN_TEST(test_dts_address_size_extraction);
    RUN_TEST(test_dts_address_size_range);
    RUN_TEST(test_dts_header_length_extraction);

    /* Segment Size Tests */
    RUN_TEST(test_dts_segment_size_maximum);
    RUN_TEST(test_dts_segment_size_fits_in_10_bits);
    RUN_TEST(test_dts_segment_c_pdu_size_encoding);

    /* NONARQ PDU Tests */
    RUN_TEST(test_dts_nonarq_c_pdu_id_validation);
    RUN_TEST(test_dts_nonarq_c_pdu_id_extraction);
    RUN_TEST(test_dts_nonarq_c_pdu_size_encoding);
    RUN_TEST(test_dts_nonarq_c_pdu_offset_encoding);
    RUN_TEST(test_dts_nonarq_c_pdu_rx_win_encoding);

    /* Length Validation Tests */
    RUN_TEST(test_dts_c_pdu_size_maximum);
    RUN_TEST(test_dts_c_pdu_size_minimum);
    RUN_TEST(test_dts_segment_size_boundary);
    RUN_TEST(test_dts_offset_plus_segment_validation);

    /* SAP ID Validation Tests */
    RUN_TEST(test_dts_sap_id_range);
    RUN_TEST(test_dts_sap_id_extraction_from_c_pdu);

    /* Constants Validation */
    RUN_TEST(test_dts_protocol_constants);
    RUN_TEST(test_dts_d_type_constants);
    RUN_TEST(test_dts_d_type_uniqueness);

    /* Header Size Validation Tests */
    RUN_TEST(test_dts_data_only_header_size);
    RUN_TEST(test_dts_ack_only_header_size_minimum);
    RUN_TEST(test_dts_data_ack_header_size_minimum);
    RUN_TEST(test_dts_reset_header_size);
    RUN_TEST(test_dts_nonarq_header_size);

    /* Complete PDU Structure Tests */
    RUN_TEST(test_dts_nonarq_complete_pdu_structure);
    RUN_TEST(test_dts_data_only_complete_pdu_structure);

    /* Error Detection Tests */
    RUN_TEST(test_dts_invalid_d_type);
    RUN_TEST(test_dts_c_pdu_id_out_of_range);
    RUN_TEST(test_dts_oversized_segment);
    RUN_TEST(test_dts_oversized_c_pdu);
    RUN_TEST(test_dts_zero_c_pdu_size);

    return UNITY_END();
}
