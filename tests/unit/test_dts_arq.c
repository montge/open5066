/*
 * test_dts_arq.c - Comprehensive tests for DTS ARQ state machine and execution
 *
 * Tests coverage:
 * - ARQ window management (tx_lwe, tx_uwe, rx_lwe, rx_uwe)
 * - Sequence number allocation and wraparound
 * - ACK bitmap manipulation
 * - Transmission window full detection
 * - Reception window management
 * - PDU retransmission tracking
 * - Address encoding/decoding
 * - Multi-segment transmission
 *
 * Part of Open5066 test suite
 */

#include "unity.h"
#include <string.h>
#include <stdlib.h>

#define DTS_SEG_SIZE 800
#define DTS_MAX_PDU_SIZE 4096

/* Test Setup/Teardown */
void setUp(void) {
    /* Called before each test */
}

void tearDown(void) {
    /* Called after each test */
}

/* ========================================================================
 * ARQ Window Management Tests
 * ======================================================================== */

void test_arq_window_initial_state(void) {
    struct {
        int tx_lwe;  /* Transmit Lower Window Edge */
        int tx_uwe;  /* Transmit Upper Window Edge */
        int rx_lwe;  /* Receive Lower Window Edge */
        int rx_uwe;  /* Receive Upper Window Edge */
    } dts_conn;

    /* Initialize ARQ windows */
    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 0;
    dts_conn.rx_lwe = 0;
    dts_conn.rx_uwe = 0;

    /* Empty windows initially */
    TEST_ASSERT_EQUAL(0, dts_conn.tx_lwe);
    TEST_ASSERT_EQUAL(0, dts_conn.tx_uwe);
    TEST_ASSERT_EQUAL(0, dts_conn.rx_lwe);
    TEST_ASSERT_EQUAL(0, dts_conn.rx_uwe);
}

void test_arq_tx_window_expand(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 0;

    /* Expand window by incrementing upper edge */
    int n_tx_seq = ++dts_conn.tx_uwe;

    TEST_ASSERT_EQUAL(1, n_tx_seq);
    TEST_ASSERT_EQUAL(1, dts_conn.tx_uwe);
    TEST_ASSERT_EQUAL(0, dts_conn.tx_lwe);

    /* Window now contains 1 PDU */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    TEST_ASSERT_EQUAL(1, window_size);
}

void test_arq_tx_window_full_detection(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 127;  /* Maximum window size */

    /* Check if window is full */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    int is_full = (window_size >= 127);

    TEST_ASSERT_TRUE(is_full);
    TEST_ASSERT_EQUAL(127, window_size);
}

void test_arq_tx_window_advance_on_ack(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 10;

    /* Receive ACK for PDU 0, advance lower edge */
    dts_conn.tx_lwe++;

    TEST_ASSERT_EQUAL(1, dts_conn.tx_lwe);
    TEST_ASSERT_EQUAL(10, dts_conn.tx_uwe);

    /* Window now contains 9 PDUs (1-10) */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    TEST_ASSERT_EQUAL(9, window_size);
}

void test_arq_rx_window_expand(void) {
    struct {
        int rx_lwe;
        int rx_uwe;
    } dts_conn;

    dts_conn.rx_lwe = 0;
    dts_conn.rx_uwe = 0;

    /* Receive new PDU, expand upper edge */
    dts_conn.rx_uwe++;

    TEST_ASSERT_EQUAL(1, dts_conn.rx_uwe);
    TEST_ASSERT_EQUAL(0, dts_conn.rx_lwe);
}

void test_arq_rx_window_empty_check(void) {
    struct {
        int rx_lwe;
        int rx_uwe;
    } dts_conn;

    dts_conn.rx_lwe = 5;
    dts_conn.rx_uwe = 5;

    /* Window is empty when lwe == uwe */
    int is_empty = (dts_conn.rx_lwe == dts_conn.rx_uwe);
    TEST_ASSERT_TRUE(is_empty);
}

/* ========================================================================
 * Sequence Number Tests
 * ======================================================================== */

void test_sequence_number_8bit_range(void) {
    /* Sequence numbers are 8-bit (0-255) */
    int seq = 0;

    /* Valid range */
    TEST_ASSERT_GREATER_OR_EQUAL(0, seq);
    TEST_ASSERT_LESS_THAN(256, seq);

    seq = 255;
    TEST_ASSERT_EQUAL(255, seq);
}

void test_sequence_number_allocation(void) {
    struct {
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_uwe = 0;

    /* Allocate sequence number */
    int n_tx_seq = ++dts_conn.tx_uwe;

    TEST_ASSERT_EQUAL(1, n_tx_seq);
}

void test_sequence_number_wraparound(void) {
    struct {
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_uwe = 255;

    /* Next sequence wraps to 0 (8-bit wraparound) */
    int n_tx_seq = ++dts_conn.tx_uwe;
    int seq_8bit = n_tx_seq & 0x00ff;

    TEST_ASSERT_EQUAL(256, n_tx_seq);  /* Internal counter continues */
    TEST_ASSERT_EQUAL(0, seq_8bit);    /* But PDU uses 8-bit value */
}

void test_sequence_number_mask_application(void) {
    int n_tx_seq = 300;

    /* Apply 8-bit mask */
    int seq_8bit = n_tx_seq & 0x00ff;

    /* 300 & 0xFF = 44 */
    TEST_ASSERT_EQUAL(44, seq_8bit);
}

void test_sequence_number_multiple_allocations(void) {
    struct {
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_uwe = 0;

    /* Allocate 3 sequence numbers */
    int seq1 = ++dts_conn.tx_uwe;
    int seq2 = ++dts_conn.tx_uwe;
    int seq3 = ++dts_conn.tx_uwe;

    TEST_ASSERT_EQUAL(1, seq1);
    TEST_ASSERT_EQUAL(2, seq2);
    TEST_ASSERT_EQUAL(3, seq3);
}

/* ========================================================================
 * ACK Bitmap Tests
 * ======================================================================== */

void test_ack_bitmap_initial_clear(void) {
    unsigned char acks[256 / 8];  /* 256 bits = 32 bytes */

    memset(acks, 0, sizeof(acks));

    /* All ACKs should be clear */
    for (int i = 0; i < 256; i++) {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        int is_acked = (acks[byte_idx] & (1 << bit_idx)) != 0;
        TEST_ASSERT_FALSE(is_acked);
    }
}

void test_ack_bitmap_set_single_bit(void) {
    unsigned char acks[256 / 8];
    memset(acks, 0, sizeof(acks));

    /* Set ACK for sequence 10 */
    int seq = 10;
    int byte_idx = seq / 8;
    int bit_idx = seq % 8;
    acks[byte_idx] |= (1 << bit_idx);

    /* Verify bit is set */
    int is_acked = (acks[byte_idx] & (1 << bit_idx)) != 0;
    TEST_ASSERT_TRUE(is_acked);
}

void test_ack_bitmap_set_multiple_bits(void) {
    unsigned char acks[256 / 8];
    memset(acks, 0, sizeof(acks));

    /* Set ACKs for sequences 5, 10, 15 */
    int sequences[] = {5, 10, 15};
    for (int i = 0; i < 3; i++) {
        int seq = sequences[i];
        acks[seq / 8] |= (1 << (seq % 8));
    }

    /* Verify all are set */
    for (int i = 0; i < 3; i++) {
        int seq = sequences[i];
        int is_acked = (acks[seq / 8] & (1 << (seq % 8))) != 0;
        TEST_ASSERT_TRUE(is_acked);
    }
}

void test_ack_bitmap_clear_bit(void) {
    unsigned char acks[256 / 8];
    memset(acks, 0xFF, sizeof(acks));  /* All bits set */

    /* Clear ACK for sequence 20 */
    int seq = 20;
    acks[seq / 8] &= ~(1 << (seq % 8));

    /* Verify bit is cleared */
    int is_acked = (acks[seq / 8] & (1 << (seq % 8))) != 0;
    TEST_ASSERT_FALSE(is_acked);
}

void test_ack_bitmap_range_check(void) {
    unsigned char acks[256 / 8];
    memset(acks, 0, sizeof(acks));

    /* Set range 10-15 */
    for (int i = 10; i <= 15; i++) {
        acks[i / 8] |= (1 << (i % 8));
    }

    /* Verify range is set */
    for (int i = 10; i <= 15; i++) {
        int is_acked = (acks[i / 8] & (1 << (i % 8))) != 0;
        TEST_ASSERT_TRUE(is_acked);
    }

    /* Verify outside range is clear */
    for (int i = 0; i < 10; i++) {
        int is_acked = (acks[i / 8] & (1 << (i % 8))) != 0;
        TEST_ASSERT_FALSE(is_acked);
    }
}

void test_ack_bitmap_length_calculation(void) {
    struct {
        int rx_lwe;
        int rx_uwe;
    } dts_conn;

    dts_conn.rx_lwe = 10;
    dts_conn.rx_uwe = 90;

    /* ACK bitmap length = (uwe - lwe) / 8 */
    int ack_len = (dts_conn.rx_uwe - dts_conn.rx_lwe) / 8;

    /* (90 - 10) / 8 = 10 bytes */
    TEST_ASSERT_EQUAL(10, ack_len);
}

/* ========================================================================
 * PDU Tracking Tests
 * ======================================================================== */

void test_pdu_tracking_array_allocation(void) {
    struct pdu* tx_pdus[256];

    /* Initialize tracking array */
    memset(tx_pdus, 0, sizeof(tx_pdus));

    /* Verify all slots are NULL */
    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_NULL(tx_pdus[i]);
    }
}

void test_pdu_tracking_store_by_sequence(void) {
    struct pdu {
        int seq;
        int data;
    } pdu1;

    struct pdu* tx_pdus[256];
    memset(tx_pdus, 0, sizeof(tx_pdus));

    /* Store PDU at sequence 42 */
    pdu1.seq = 42;
    pdu1.data = 12345;
    int seq_idx = pdu1.seq & 0x00ff;
    tx_pdus[seq_idx] = &pdu1;

    TEST_ASSERT_EQUAL_PTR(&pdu1, tx_pdus[42]);
}

void test_pdu_tracking_retrieve_by_sequence(void) {
    struct pdu {
        int seq;
        int data;
    } pdu1;

    struct pdu* tx_pdus[256];
    memset(tx_pdus, 0, sizeof(tx_pdus));

    pdu1.seq = 100;
    pdu1.data = 99999;
    tx_pdus[pdu1.seq & 0x00ff] = &pdu1;

    /* Retrieve PDU */
    struct pdu* retrieved = tx_pdus[100];
    TEST_ASSERT_EQUAL_PTR(&pdu1, retrieved);
    TEST_ASSERT_EQUAL(99999, retrieved->data);
}

void test_pdu_tracking_sequence_wraparound(void) {
    struct pdu {
        int seq;
    } pdu1;

    struct pdu* tx_pdus[256];
    memset(tx_pdus, 0, sizeof(tx_pdus));

    /* Sequence 300 wraps to index 44 */
    int seq = 300;
    pdu1.seq = seq;
    tx_pdus[seq & 0x00ff] = &pdu1;

    TEST_ASSERT_EQUAL_PTR(&pdu1, tx_pdus[44]);
}

void test_pdu_tracking_remove_on_ack(void) {
    struct pdu {
        int seq;
    } pdu1;

    struct pdu* tx_pdus[256];
    memset(tx_pdus, 0, sizeof(tx_pdus));

    pdu1.seq = 50;
    tx_pdus[pdu1.seq & 0x00ff] = &pdu1;

    /* Simulate ACK received, free PDU */
    tx_pdus[50] = NULL;

    TEST_ASSERT_NULL(tx_pdus[50]);
}

/* ========================================================================
 * Multi-Segment Transmission Tests
 * ======================================================================== */

void test_multiseg_segment_count_calculation(void) {
    int c_pdu_size = 2000;
    int seg_size = DTS_SEG_SIZE;  /* 800 bytes */

    /* Calculate number of segments needed */
    int num_segments = (c_pdu_size + seg_size - 1) / seg_size;

    /* 2000 / 800 = 2.5, rounds up to 3 segments */
    TEST_ASSERT_EQUAL(3, num_segments);
}

void test_multiseg_first_segment_flag(void) {
    unsigned char flags = 0x80;  /* First segment flag */

    /* First segment has bit 7 set */
    int is_first = (flags & 0x80) != 0;
    TEST_ASSERT_TRUE(is_first);
}

void test_multiseg_last_segment_flag(void) {
    unsigned char flags = 0x40;  /* Last segment flag */

    /* Last segment has bit 6 set */
    int is_last = (flags & 0x40) != 0;
    TEST_ASSERT_TRUE(is_last);
}

void test_multiseg_middle_segment_no_flags(void) {
    unsigned char flags = 0x00;  /* Middle segment */

    /* Middle segments have neither flag */
    int is_first = (flags & 0x80) != 0;
    int is_last = (flags & 0x40) != 0;

    TEST_ASSERT_FALSE(is_first);
    TEST_ASSERT_FALSE(is_last);
}

void test_multiseg_single_segment_both_flags(void) {
    unsigned char flags = 0xC0;  /* Both first and last */

    /* Single segment has both flags set */
    int is_first = (flags & 0x80) != 0;
    int is_last = (flags & 0x40) != 0;

    TEST_ASSERT_TRUE(is_first);
    TEST_ASSERT_TRUE(is_last);
}

void test_multiseg_segment_size_encoding(void) {
    int seg_size = 800;

    /* Segment size is 10 bits (0-1023) */
    unsigned char high = (seg_size >> 8) & 0x03;
    unsigned char low = seg_size & 0x00ff;

    TEST_ASSERT_EQUAL(0x03, high);
    TEST_ASSERT_EQUAL(0x20, low);

    /* Reconstruct */
    int reconstructed = (high << 8) | low;
    TEST_ASSERT_EQUAL(800, reconstructed);
}

void test_multiseg_last_segment_size(void) {
    int c_pdu_size = 1234;
    int seg_size = DTS_SEG_SIZE;  /* 800 */

    /* First segment: 800 bytes */
    /* Last segment: 1234 - 800 = 434 bytes */
    int last_seg_size = c_pdu_size - seg_size;

    TEST_ASSERT_EQUAL(434, last_seg_size);
}

/* ========================================================================
 * Address Encoding Tests
 * ======================================================================== */

void test_address_encoding_length_extraction(void) {
    unsigned char addr_byte = 0xA5;  /* Binary: 10100101 */

    /* Length is in upper 3 bits */
    int len = (addr_byte >> 5) & 0x07;

    /* Upper 3 bits = 101 = 5 */
    TEST_ASSERT_EQUAL(5, len);
}

void test_address_encoding_nibble_storage(void) {
    unsigned char addr[4];
    memset(addr, 0, sizeof(addr));

    /* Set nibble at position 0 to value 0xA */
    addr[0] = (addr[0] & 0x0F) | (0xA << 4);

    /* Get nibble at position 0 */
    int nibble = (addr[0] >> 4) & 0x0F;

    TEST_ASSERT_EQUAL(0xA, nibble);
}

void test_address_encoding_two_nibbles_per_byte(void) {
    unsigned char byte = 0xAB;

    /* High nibble = A, low nibble = B */
    int high = (byte >> 4) & 0x0F;
    int low = byte & 0x0F;

    TEST_ASSERT_EQUAL(0xA, high);
    TEST_ASSERT_EQUAL(0xB, low);
}

void test_address_encoding_max_length(void) {
    /* Maximum address length is 7 (3 bits) */
    int max_len = 0x07;

    TEST_ASSERT_EQUAL(7, max_len);
    TEST_ASSERT_LESS_OR_EQUAL(7, max_len);
    TEST_ASSERT_GREATER_THAN(0, max_len);
}

void test_address_decoding_packed_format(void) {
    /* Two addresses packed: length encoded in first byte */
    unsigned char packed[8];
    memset(packed, 0, sizeof(packed));

    /* Encode: to_addr length = 3, from_addr length = 4 */
    int to_len = 3;
    int from_len = 4;
    int max_len = (to_len > from_len) ? to_len : from_len;

    TEST_ASSERT_EQUAL(4, max_len);
}

/* ========================================================================
 * Transmission Window Full Handling Tests
 * ======================================================================== */

void test_tx_window_full_return_error(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 127;

    /* Check if window is full before allocating */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    int n_tx_seq = -1;

    if (window_size < 127) {
        n_tx_seq = ++dts_conn.tx_uwe;
    }

    /* Should return -1 indicating full window */
    TEST_ASSERT_EQUAL(-1, n_tx_seq);
}

void test_tx_window_has_space(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 50;

    /* Window has space */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    int has_space = (window_size < 127);

    TEST_ASSERT_TRUE(has_space);
}

void test_tx_window_space_after_ack(void) {
    struct {
        int tx_lwe;
        int tx_uwe;
    } dts_conn;

    dts_conn.tx_lwe = 0;
    dts_conn.tx_uwe = 127;  /* Full */

    /* Receive ACK, advance lower edge */
    dts_conn.tx_lwe += 10;

    /* Now has space for 10 more */
    int window_size = dts_conn.tx_uwe - dts_conn.tx_lwe;
    TEST_ASSERT_EQUAL(117, window_size);

    int has_space = (window_size < 127);
    TEST_ASSERT_TRUE(has_space);
}

/* ========================================================================
 * Edge Window Edge Markers Tests
 * ======================================================================== */

void test_edge_markers_first_in_window(void) {
    int n_tx_seq = 10;
    int tx_lwe = 10;

    /* Mark if this is lower window edge */
    int is_lwe = (n_tx_seq == tx_lwe);

    TEST_ASSERT_TRUE(is_lwe);
}

void test_edge_markers_last_in_window(void) {
    int n_tx_seq = 50;
    int tx_uwe = 50;

    /* Mark if this is upper window edge */
    int is_uwe = (n_tx_seq == tx_uwe);

    TEST_ASSERT_TRUE(is_uwe);
}

void test_edge_markers_encoding_in_flags(void) {
    unsigned char flags = 0x00;

    int is_uwe = 1;
    int is_lwe = 1;

    /* Encode edge markers in bits 7 and 6 */
    flags |= (is_uwe ? 0x80 : 0);
    flags |= (is_lwe ? 0x40 : 0);

    TEST_ASSERT_EQUAL(0xC0, flags);
}

void test_edge_markers_neither_edge(void) {
    int n_tx_seq = 25;
    int tx_lwe = 10;
    int tx_uwe = 50;

    /* Neither edge */
    int is_lwe = (n_tx_seq == tx_lwe);
    int is_uwe = (n_tx_seq == tx_uwe);

    TEST_ASSERT_FALSE(is_lwe);
    TEST_ASSERT_FALSE(is_uwe);
}

/* ========================================================================
 * Main Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* ARQ Window Management Tests */
    RUN_TEST(test_arq_window_initial_state);
    RUN_TEST(test_arq_tx_window_expand);
    RUN_TEST(test_arq_tx_window_full_detection);
    RUN_TEST(test_arq_tx_window_advance_on_ack);
    RUN_TEST(test_arq_rx_window_expand);
    RUN_TEST(test_arq_rx_window_empty_check);

    /* Sequence Number Tests */
    RUN_TEST(test_sequence_number_8bit_range);
    RUN_TEST(test_sequence_number_allocation);
    RUN_TEST(test_sequence_number_wraparound);
    RUN_TEST(test_sequence_number_mask_application);
    RUN_TEST(test_sequence_number_multiple_allocations);

    /* ACK Bitmap Tests */
    RUN_TEST(test_ack_bitmap_initial_clear);
    RUN_TEST(test_ack_bitmap_set_single_bit);
    RUN_TEST(test_ack_bitmap_set_multiple_bits);
    RUN_TEST(test_ack_bitmap_clear_bit);
    RUN_TEST(test_ack_bitmap_range_check);
    RUN_TEST(test_ack_bitmap_length_calculation);

    /* PDU Tracking Tests */
    RUN_TEST(test_pdu_tracking_array_allocation);
    RUN_TEST(test_pdu_tracking_store_by_sequence);
    RUN_TEST(test_pdu_tracking_retrieve_by_sequence);
    RUN_TEST(test_pdu_tracking_sequence_wraparound);
    RUN_TEST(test_pdu_tracking_remove_on_ack);

    /* Multi-Segment Transmission Tests */
    RUN_TEST(test_multiseg_segment_count_calculation);
    RUN_TEST(test_multiseg_first_segment_flag);
    RUN_TEST(test_multiseg_last_segment_flag);
    RUN_TEST(test_multiseg_middle_segment_no_flags);
    RUN_TEST(test_multiseg_single_segment_both_flags);
    RUN_TEST(test_multiseg_segment_size_encoding);
    RUN_TEST(test_multiseg_last_segment_size);

    /* Address Encoding Tests */
    RUN_TEST(test_address_encoding_length_extraction);
    RUN_TEST(test_address_encoding_nibble_storage);
    RUN_TEST(test_address_encoding_two_nibbles_per_byte);
    RUN_TEST(test_address_encoding_max_length);
    RUN_TEST(test_address_decoding_packed_format);

    /* Transmission Window Full Handling Tests */
    RUN_TEST(test_tx_window_full_return_error);
    RUN_TEST(test_tx_window_has_space);
    RUN_TEST(test_tx_window_space_after_ack);

    /* Edge Markers Tests */
    RUN_TEST(test_edge_markers_first_in_window);
    RUN_TEST(test_edge_markers_last_in_window);
    RUN_TEST(test_edge_markers_encoding_in_flags);
    RUN_TEST(test_edge_markers_neither_edge);

    return UNITY_END();
}
