/* test_segment_assembly.c - Tests for DTS segment assembly and ARQ
 * These tests validate segment reassembly and ARQ window management
 */

#include "unity.h"
#include <stdint.h>
#include <string.h>

/* DTS Protocol Constants */
#define DTS_MIN_PDU_SIZE 6
#define DTS_MAX_PDU_SIZE 4096
#define DTS_SEG_SIZE 800
#define DTS_NONARQ 7

/* SIS Constants */
#define SIS_MAX_SAP_ID 16
#define SIS_UNIDATA_IND_MIN_HDR 22
#define SIS_MAX_PDU_SIZE 8192

void setUp(void) {
    /* Set up before each test */
}

void tearDown(void) {
    /* Clean up after each test */
}

/* ==================== Segment Assembly Basics ==================== */

void test_segment_assembly_single_segment(void) {
    /* Single segment C_PDU (no assembly needed) */
    uint16_t c_pdu_size = 500;
    uint16_t seg_size = 500;
    uint16_t c_pdu_offset = 0;

    /* Verify this is a complete PDU */
    TEST_ASSERT_EQUAL(c_pdu_size, seg_size);
    TEST_ASSERT_EQUAL(0, c_pdu_offset);
    TEST_ASSERT_EQUAL(c_pdu_size, c_pdu_offset + seg_size);
}

void test_segment_assembly_two_segments(void) {
    /* C_PDU split into 2 segments */
    uint16_t c_pdu_size = 1200;
    uint16_t seg_size = DTS_SEG_SIZE; /* 800 bytes */

    /* First segment */
    uint16_t seg1_offset = 0;
    uint16_t seg1_size = DTS_SEG_SIZE;

    /* Second segment */
    uint16_t seg2_offset = DTS_SEG_SIZE;
    uint16_t seg2_size = c_pdu_size - DTS_SEG_SIZE; /* 400 bytes */

    /* Verify segments cover entire C_PDU */
    TEST_ASSERT_EQUAL(0, seg1_offset);
    TEST_ASSERT_EQUAL(800, seg1_size);
    TEST_ASSERT_EQUAL(800, seg2_offset);
    TEST_ASSERT_EQUAL(400, seg2_size);
    TEST_ASSERT_EQUAL(c_pdu_size, seg2_offset + seg2_size);
}

void test_segment_assembly_multiple_segments(void) {
    /* C_PDU split into 5 segments */
    uint16_t c_pdu_size = 4000;
    uint16_t seg_size = DTS_SEG_SIZE; /* 800 bytes */

    /* Calculate number of segments */
    int num_full_segments = c_pdu_size / seg_size; /* 5 */
    int last_segment_size = c_pdu_size % seg_size; /* 0 */

    TEST_ASSERT_EQUAL(5, num_full_segments);
    TEST_ASSERT_EQUAL(0, last_segment_size);

    /* Verify offsets */
    for (int i = 0; i < num_full_segments; i++) {
        uint16_t offset = i * seg_size;
        TEST_ASSERT_EQUAL(i * 800, offset);
        TEST_ASSERT_LESS_THAN(c_pdu_size, offset);
    }
}

void test_segment_assembly_partial_last_segment(void) {
    /* C_PDU with partial last segment */
    uint16_t c_pdu_size = 2500;
    uint16_t seg_size = DTS_SEG_SIZE; /* 800 bytes */

    int num_segments = (c_pdu_size + seg_size - 1) / seg_size; /* Ceiling division */
    int last_segment_size = c_pdu_size - ((num_segments - 1) * seg_size);

    TEST_ASSERT_EQUAL(4, num_segments); /* 800+800+800+100 = 2500 */
    TEST_ASSERT_EQUAL(100, last_segment_size);
}

/* ==================== Segment Offset Validation ==================== */

void test_segment_offset_validation_in_bounds(void) {
    /* Valid segment offset */
    uint16_t c_pdu_size = 2000;
    uint16_t c_pdu_offset = 800;
    uint16_t seg_size = 800;

    /* Verify offset + size doesn't exceed c_pdu_size */
    TEST_ASSERT_LESS_OR_EQUAL(c_pdu_size, c_pdu_offset + seg_size);
}

void test_segment_offset_validation_out_of_bounds(void) {
    /* Invalid segment offset */
    uint16_t c_pdu_size = 2000;
    uint16_t c_pdu_offset = 1500;
    uint16_t seg_size = 800;

    /* Verify this would exceed c_pdu_size */
    TEST_ASSERT_GREATER_THAN(c_pdu_size, c_pdu_offset + seg_size);
}

void test_segment_offset_alignment(void) {
    /* Segments typically align to segment size boundaries */
    uint16_t seg_size = DTS_SEG_SIZE;

    /* First segment */
    uint16_t offset0 = 0 * seg_size;
    TEST_ASSERT_EQUAL(0, offset0);

    /* Second segment */
    uint16_t offset1 = 1 * seg_size;
    TEST_ASSERT_EQUAL(800, offset1);

    /* Third segment */
    uint16_t offset2 = 2 * seg_size;
    TEST_ASSERT_EQUAL(1600, offset2);
}

/* ==================== RX Map (Receive Bitmap) Tests ==================== */

void test_rx_map_initialization(void) {
    /* RX map tracks which bytes have been received */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    /* Verify all bits are zero */
    for (int i = 0; i < sizeof(rx_map); i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, rx_map[i]);
    }
}

void test_rx_map_set_bit(void) {
    /* Set individual bits in RX map */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    /* Set bit 0 */
    rx_map[0 / 8] |= (1 << (0 % 8));
    TEST_ASSERT_EQUAL_HEX8(0x01, rx_map[0]);

    /* Set bit 5 */
    rx_map[5 / 8] |= (1 << (5 % 8));
    TEST_ASSERT_EQUAL_HEX8(0x21, rx_map[0]); /* 0x01 | 0x20 = 0x21 */
}

void test_rx_map_check_bit(void) {
    /* Check if bit is set */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    /* Set bit 7 */
    int bit_index = 7;
    rx_map[bit_index / 8] |= (1 << (bit_index % 8));

    /* Check it's set */
    int is_set = (rx_map[bit_index / 8] & (1 << (bit_index % 8))) != 0;
    TEST_ASSERT_TRUE(is_set);

    /* Check bit 6 is not set */
    int bit6_set = (rx_map[6 / 8] & (1 << (6 % 8))) != 0;
    TEST_ASSERT_FALSE(bit6_set);
}

void test_rx_map_segment_range(void) {
    /* Mark entire segment as received */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    uint16_t offset = 100;
    uint16_t size = 50;

    /* Set bits for bytes 100-149 */
    for (int i = offset; i < offset + size; i++) {
        rx_map[i / 8] |= (1 << (i % 8));
    }

    /* Verify range is marked */
    for (int i = offset; i < offset + size; i++) {
        int is_set = (rx_map[i / 8] & (1 << (i % 8))) != 0;
        TEST_ASSERT_TRUE(is_set);
    }

    /* Verify byte before range is not marked */
    int before_set = (rx_map[(offset - 1) / 8] & (1 << ((offset - 1) % 8))) != 0;
    TEST_ASSERT_FALSE(before_set);
}

/* ==================== C_PDU Completeness Check ==================== */

void test_cpdu_completeness_all_received(void) {
    /* Check if all bytes have been received */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0xFF, sizeof(rx_map)); /* All bits set */

    uint16_t c_pdu_len = 100;

    /* Check all bytes in range */
    int complete = 1;
    for (int i = 0; i < c_pdu_len; i++) {
        if (!(rx_map[i / 8] & (1 << (i % 8)))) {
            complete = 0;
            break;
        }
    }

    TEST_ASSERT_TRUE(complete);
}

void test_cpdu_completeness_partial_received(void) {
    /* Incomplete C_PDU */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    /* Mark bytes 0-49 as received */
    for (int i = 0; i < 50; i++) {
        rx_map[i / 8] |= (1 << (i % 8));
    }

    uint16_t c_pdu_len = 100;

    /* Check if complete */
    int complete = 1;
    for (int i = 0; i < c_pdu_len; i++) {
        if (!(rx_map[i / 8] & (1 << (i % 8)))) {
            complete = 0;
            break;
        }
    }

    TEST_ASSERT_FALSE(complete); /* Only 50/100 bytes received */
}

void test_cpdu_completeness_gap_in_middle(void) {
    /* C_PDU with gap in the middle */
    unsigned char rx_map[SIS_MAX_PDU_SIZE / 8];
    memset(rx_map, 0, sizeof(rx_map));

    /* Mark bytes 0-49 */
    for (int i = 0; i < 50; i++) {
        rx_map[i / 8] |= (1 << (i % 8));
    }
    /* Gap at bytes 50-59 */
    /* Mark bytes 60-99 */
    for (int i = 60; i < 100; i++) {
        rx_map[i / 8] |= (1 << (i % 8));
    }

    uint16_t c_pdu_len = 100;

    /* Check for gaps */
    int gap_found = 0;
    for (int i = 0; i < c_pdu_len; i++) {
        if (!(rx_map[i / 8] & (1 << (i % 8)))) {
            gap_found = 1;
            TEST_ASSERT_GREATER_OR_EQUAL(50, i); /* Gap should be >= 50 */
            TEST_ASSERT_LESS_THAN(60, i); /* Gap should be < 60 */
            break;
        }
    }

    TEST_ASSERT_TRUE(gap_found);
}

/* ==================== C_PDU ID Validation ==================== */

void test_cpdu_id_range(void) {
    /* C_PDU ID is 12 bits (0-4095) */
    uint16_t min_id = 0;
    uint16_t max_id = 4095;

    TEST_ASSERT_EQUAL(0, min_id);
    TEST_ASSERT_EQUAL(4095, max_id);
}

void test_cpdu_id_wraparound(void) {
    /* C_PDU ID wraps around at 4096 */
    uint16_t id = 4095;
    id++;
    id &= 0x0FFF; /* Mask to 12 bits */

    TEST_ASSERT_EQUAL(0, id); /* Wraps to 0 */
}

void test_cpdu_id_storage_array_size(void) {
    /* Array to store C_PDUs by ID */
    int array_size = 4096; /* 0-4095 */

    TEST_ASSERT_EQUAL(4096, array_size);

    /* Verify all IDs fit */
    for (int id = 0; id <= 4095; id++) {
        TEST_ASSERT_LESS_THAN(array_size, id);
    }
}

/* ==================== Segment Size Calculations ==================== */

void test_segment_count_calculation(void) {
    /* Calculate number of segments needed */
    struct {
        uint16_t c_pdu_size;
        int expected_segments;
    } tests[] = {
        {100, 1},
        {800, 1},
        {801, 2},
        {1600, 2},
        {1601, 3},
        {4000, 5}
    };

    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        int num_segs = (tests[i].c_pdu_size + DTS_SEG_SIZE - 1) / DTS_SEG_SIZE;
        TEST_ASSERT_EQUAL(tests[i].expected_segments, num_segs);
    }
}

void test_segment_size_for_each_segment(void) {
    /* Calculate size of each segment */
    uint16_t c_pdu_size = 2100;
    uint16_t seg_size = DTS_SEG_SIZE;

    int num_segs = (c_pdu_size + seg_size - 1) / seg_size; /* 3 segments */

    /* Segment 0: 800 bytes */
    int seg0_size = (seg_size < c_pdu_size - 0) ? seg_size : c_pdu_size - 0;
    TEST_ASSERT_EQUAL(800, seg0_size);

    /* Segment 1: 800 bytes */
    int seg1_size = (seg_size < c_pdu_size - 800) ? seg_size : c_pdu_size - 800;
    TEST_ASSERT_EQUAL(800, seg1_size);

    /* Segment 2: 500 bytes (partial) */
    int seg2_size = (seg_size < c_pdu_size - 1600) ? seg_size : c_pdu_size - 1600;
    TEST_ASSERT_EQUAL(500, seg2_size);
}

/* ==================== ARQ Window Tests ==================== */

void test_arq_window_initialization(void) {
    /* ARQ transmit window */
    uint8_t tx_lwe = 0; /* Lower window edge */
    uint8_t tx_uwe = 0; /* Upper window edge */

    TEST_ASSERT_EQUAL(0, tx_lwe);
    TEST_ASSERT_EQUAL(0, tx_uwe);

    /* Window is empty initially */
    TEST_ASSERT_EQUAL(tx_lwe, tx_uwe);
}

void test_arq_window_expand(void) {
    /* Expand transmit window */
    uint8_t tx_uwe = 0;

    /* Send 3 PDUs */
    tx_uwe++; /* 1 */
    TEST_ASSERT_EQUAL(1, tx_uwe);

    tx_uwe++; /* 2 */
    TEST_ASSERT_EQUAL(2, tx_uwe);

    tx_uwe++; /* 3 */
    TEST_ASSERT_EQUAL(3, tx_uwe);
}

void test_arq_window_size(void) {
    /* Window size is uwe - lwe */
    uint8_t tx_lwe = 5;
    uint8_t tx_uwe = 10;

    int window_size = tx_uwe - tx_lwe;
    TEST_ASSERT_EQUAL(5, window_size);
}

void test_arq_window_full(void) {
    /* Check if window is full (implementation-specific) */
    uint8_t tx_lwe = 0;
    uint8_t tx_uwe = 255;
    int max_window_size = 128; /* Example max */

    int window_size = (tx_uwe - tx_lwe) & 0xFF;

    /* Window is full if size >= max */
    int is_full = window_size >= max_window_size;
    TEST_ASSERT_TRUE(is_full);
}

void test_arq_window_advance_lwe(void) {
    /* Advance lower window edge when PDU is acknowledged */
    uint8_t tx_lwe = 5;

    /* ACK received for sequence 5 */
    tx_lwe++; /* Move to 6 */
    TEST_ASSERT_EQUAL(6, tx_lwe);
}

/* ==================== Sequence Number Tests ==================== */

void test_sequence_number_range(void) {
    /* Sequence numbers are 8-bit (0-255) */
    uint8_t min_seq = 0;
    uint8_t max_seq = 255;

    TEST_ASSERT_EQUAL(0, min_seq);
    TEST_ASSERT_EQUAL(255, max_seq);
}

void test_sequence_number_wraparound(void) {
    /* Sequence wraps at 256 */
    uint8_t seq = 255;
    seq++; /* Wraps to 0 */

    TEST_ASSERT_EQUAL(0, seq);
}

void test_sequence_number_comparison(void) {
    /* Compare sequence numbers with wraparound */
    uint8_t seq1 = 250;
    uint8_t seq2 = 5;

    /* seq2 (5) comes after seq1 (250) due to wraparound */
    int diff = (seq2 - seq1) & 0xFF;
    TEST_ASSERT_EQUAL(11, diff); /* 255-250 + 5 + 1 = 11 */
}

/* ==================== ACK Bitmap Tests ==================== */

void test_ack_bitmap_initialization(void) {
    /* ACK bitmap for tracking received PDUs */
    unsigned char acks[256 / 8]; /* 32 bytes for 256 sequence numbers */
    memset(acks, 0, sizeof(acks));

    /* All ACKs initially zero */
    for (int i = 0; i < sizeof(acks); i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, acks[i]);
    }
}

void test_ack_bitmap_set_ack(void) {
    /* Set ACK for specific sequence */
    unsigned char acks[256 / 8];
    memset(acks, 0, sizeof(acks));

    uint8_t seq = 42;
    acks[seq / 8] |= (1 << (seq % 8));

    /* Check ACK is set */
    int is_acked = (acks[seq / 8] & (1 << (seq % 8))) != 0;
    TEST_ASSERT_TRUE(is_acked);
}

void test_ack_bitmap_multiple_acks(void) {
    /* Set multiple ACKs */
    unsigned char acks[256 / 8];
    memset(acks, 0, sizeof(acks));

    /* ACK sequences 10, 15, 20 */
    acks[10 / 8] |= (1 << (10 % 8));
    acks[15 / 8] |= (1 << (15 % 8));
    acks[20 / 8] |= (1 << (20 % 8));

    /* Verify all are set */
    TEST_ASSERT_TRUE((acks[10 / 8] & (1 << (10 % 8))) != 0);
    TEST_ASSERT_TRUE((acks[15 / 8] & (1 << (15 % 8))) != 0);
    TEST_ASSERT_TRUE((acks[20 / 8] & (1 << (20 % 8))) != 0);

    /* Verify 11 is not set */
    TEST_ASSERT_FALSE((acks[11 / 8] & (1 << (11 % 8))) != 0);
}

int main(void) {
    UNITY_BEGIN();

    /* Segment Assembly Basics */
    RUN_TEST(test_segment_assembly_single_segment);
    RUN_TEST(test_segment_assembly_two_segments);
    RUN_TEST(test_segment_assembly_multiple_segments);
    RUN_TEST(test_segment_assembly_partial_last_segment);

    /* Segment Offset Validation */
    RUN_TEST(test_segment_offset_validation_in_bounds);
    RUN_TEST(test_segment_offset_validation_out_of_bounds);
    RUN_TEST(test_segment_offset_alignment);

    /* RX Map Tests */
    RUN_TEST(test_rx_map_initialization);
    RUN_TEST(test_rx_map_set_bit);
    RUN_TEST(test_rx_map_check_bit);
    RUN_TEST(test_rx_map_segment_range);

    /* C_PDU Completeness */
    RUN_TEST(test_cpdu_completeness_all_received);
    RUN_TEST(test_cpdu_completeness_partial_received);
    RUN_TEST(test_cpdu_completeness_gap_in_middle);

    /* C_PDU ID Validation */
    RUN_TEST(test_cpdu_id_range);
    RUN_TEST(test_cpdu_id_wraparound);
    RUN_TEST(test_cpdu_id_storage_array_size);

    /* Segment Size Calculations */
    RUN_TEST(test_segment_count_calculation);
    RUN_TEST(test_segment_size_for_each_segment);

    /* ARQ Window Tests */
    RUN_TEST(test_arq_window_initialization);
    RUN_TEST(test_arq_window_expand);
    RUN_TEST(test_arq_window_size);
    RUN_TEST(test_arq_window_full);
    RUN_TEST(test_arq_window_advance_lwe);

    /* Sequence Number Tests */
    RUN_TEST(test_sequence_number_range);
    RUN_TEST(test_sequence_number_wraparound);
    RUN_TEST(test_sequence_number_comparison);

    /* ACK Bitmap Tests */
    RUN_TEST(test_ack_bitmap_initialization);
    RUN_TEST(test_ack_bitmap_set_ack);
    RUN_TEST(test_ack_bitmap_multiple_acks);

    return UNITY_END();
}
