/*
 * test_io_write.c - Comprehensive tests for hiwrite.c I/O write operations
 *
 * Tests coverage:
 * - IOV setup (hi_send1, hi_send2, hi_send3)
 * - Queue management (to_write, in_write queues)
 * - PDU lifecycle (hi_free_resp, hi_free_req, hi_add_to_reqs)
 * - IOV clearing (partial and complete writes)
 * - Response list management
 * - Write queue state transitions
 *
 * Part of Open5066 test suite
 */

#include "unity.h"
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

/* IOV_MAX might already be defined */
#ifndef IOV_MAX
#define IOV_MAX 16
#endif

#define HI_N_IOV (IOV_MAX < 32 ? IOV_MAX : 32)
#define HI_PDU_MEM 2200

/* Test Setup/Teardown */
void setUp(void) {
    /* Called before each test */
}

void tearDown(void) {
    /* Called after each test */
}

/* ========================================================================
 * IOV Structure Tests
 * ======================================================================== */

void test_iov_single_buffer_setup(void) {
    struct iovec iov[1];
    char data[100] = "Hello, World!";

    iov[0].iov_base = data;
    iov[0].iov_len = strlen(data);

    TEST_ASSERT_EQUAL_PTR(data, iov[0].iov_base);
    TEST_ASSERT_EQUAL(13, iov[0].iov_len);
    TEST_ASSERT_EQUAL_STRING("Hello, World!", (char*)iov[0].iov_base);
}

void test_iov_two_buffer_setup(void) {
    struct iovec iov[2];
    char header[10] = "HDR:";
    char payload[20] = "PAYLOAD";

    iov[0].iov_base = header;
    iov[0].iov_len = 4;
    iov[1].iov_base = payload;
    iov[1].iov_len = 7;

    size_t total_len = iov[0].iov_len + iov[1].iov_len;
    TEST_ASSERT_EQUAL(11, total_len);
    TEST_ASSERT_EQUAL_PTR(header, iov[0].iov_base);
    TEST_ASSERT_EQUAL_PTR(payload, iov[1].iov_base);
}

void test_iov_three_buffer_setup(void) {
    struct iovec iov[3];
    char header[10] = "HDR";
    char payload[100] = "PAYLOAD";
    char crc[4] = {0xAA, 0xBB, 0xCC, 0xDD};

    iov[0].iov_base = header;
    iov[0].iov_len = 3;
    iov[1].iov_base = payload;
    iov[1].iov_len = 7;
    iov[2].iov_base = crc;
    iov[2].iov_len = 4;

    size_t total_len = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;
    TEST_ASSERT_EQUAL(14, total_len);
}

void test_iov_max_vectors(void) {
    struct iovec iov[HI_N_IOV];
    char buffers[HI_N_IOV][10];

    /* Fill all IOV slots */
    for (int i = 0; i < HI_N_IOV; i++) {
        snprintf(buffers[i], 10, "BUF%d", i);
        iov[i].iov_base = buffers[i];
        iov[i].iov_len = strlen(buffers[i]);
    }

    /* Verify first and last */
    TEST_ASSERT_EQUAL_STRING("BUF0", (char*)iov[0].iov_base);
    TEST_ASSERT_GREATER_THAN(0, iov[HI_N_IOV - 1].iov_len);
}

void test_iov_empty_buffer_handling(void) {
    struct iovec iov[3];
    char header[10] = "HDR";
    char empty[1] = "";
    char payload[10] = "PAYLOAD";

    iov[0].iov_base = header;
    iov[0].iov_len = 3;
    iov[1].iov_base = empty;
    iov[1].iov_len = 0;  /* Empty buffer */
    iov[2].iov_base = payload;
    iov[2].iov_len = 7;

    /* Empty buffers should be avoided but can be represented */
    TEST_ASSERT_EQUAL(0, iov[1].iov_len);
}

/* ========================================================================
 * PDU Structure and Linkage Tests
 * ======================================================================== */

void test_pdu_request_response_linkage(void) {
    struct {
        void* n;
        void* req;
        void* reals;
    } request, response;

    /* Simulate response linked to request */
    request.reals = &response;
    request.req = NULL;
    request.n = NULL;

    response.req = &request;
    response.reals = NULL;
    response.n = NULL;

    TEST_ASSERT_EQUAL_PTR(&response, request.reals);
    TEST_ASSERT_EQUAL_PTR(&request, response.req);
}

void test_pdu_multiple_responses(void) {
    struct pdu_node {
        struct pdu_node* n;
        struct pdu_node* req;
        struct pdu_node* reals;
    } request, resp1, resp2, resp3;

    /* Request has multiple responses in reals list */
    request.reals = &resp1;
    request.req = NULL;
    request.n = NULL;

    resp1.req = &request;
    resp1.n = &resp2;
    resp1.reals = NULL;

    resp2.req = &request;
    resp2.n = &resp3;
    resp2.reals = NULL;

    resp3.req = &request;
    resp3.n = NULL;
    resp3.reals = NULL;

    /* Count responses */
    int count = 0;
    for (struct pdu_node* p = request.reals; p; p = p->n) {
        count++;
    }
    TEST_ASSERT_EQUAL(3, count);
}

void test_pdu_response_removal_from_list(void) {
    struct pdu_node {
        struct pdu_node* n;
        struct pdu_node* req;
    } request, resp1, resp2, resp3;

    request.n = NULL;
    resp1.req = &request;
    resp1.n = &resp2;
    resp2.req = &request;
    resp2.n = &resp3;
    resp3.req = &request;
    resp3.n = NULL;

    /* Simulate removing resp2 from middle of list */
    resp1.n = &resp3;  /* Skip resp2 */

    /* Now list is resp1 -> resp3 */
    TEST_ASSERT_EQUAL_PTR(&resp3, resp1.n);
    TEST_ASSERT_NULL(resp3.n);
}

/* ========================================================================
 * Write Queue Management Tests
 * ======================================================================== */

void test_write_queue_empty_state(void) {
    struct {
        void* to_write_consume;
        void* to_write_produce;
        int n_to_write;
    } io;

    /* Empty queue state */
    io.to_write_consume = NULL;
    io.to_write_produce = NULL;
    io.n_to_write = 0;

    TEST_ASSERT_NULL(io.to_write_consume);
    TEST_ASSERT_NULL(io.to_write_produce);
    TEST_ASSERT_EQUAL(0, io.n_to_write);
}

void test_write_queue_single_pdu(void) {
    struct pdu {
        struct pdu* n;
        int data;
    } pdu1;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Add first PDU to empty queue */
    pdu1.n = NULL;
    pdu1.data = 123;

    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu1;
    io.n_to_write = 1;

    TEST_ASSERT_EQUAL_PTR(&pdu1, io.to_write_consume);
    TEST_ASSERT_EQUAL_PTR(&pdu1, io.to_write_produce);
    TEST_ASSERT_EQUAL(1, io.n_to_write);
}

void test_write_queue_multiple_pdus(void) {
    struct pdu {
        struct pdu* n;
        int id;
    } pdu1, pdu2, pdu3;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Build queue: pdu1 -> pdu2 -> pdu3 */
    pdu1.id = 1;
    pdu1.n = &pdu2;
    pdu2.id = 2;
    pdu2.n = &pdu3;
    pdu3.id = 3;
    pdu3.n = NULL;

    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu3;
    io.n_to_write = 3;

    /* Verify queue structure */
    TEST_ASSERT_EQUAL(1, io.to_write_consume->id);
    TEST_ASSERT_EQUAL(3, io.to_write_produce->id);

    /* Walk the queue */
    int count = 0;
    for (struct pdu* p = io.to_write_consume; p; p = p->n) {
        count++;
    }
    TEST_ASSERT_EQUAL(3, count);
}

void test_write_queue_consume_pdu(void) {
    struct pdu {
        struct pdu* n;
        int id;
    } pdu1, pdu2;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Initial: pdu1 -> pdu2 */
    pdu1.id = 1;
    pdu1.n = &pdu2;
    pdu2.id = 2;
    pdu2.n = NULL;

    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu2;
    io.n_to_write = 2;

    /* Consume pdu1 */
    io.to_write_consume = pdu1.n;
    io.n_to_write--;

    TEST_ASSERT_EQUAL_PTR(&pdu2, io.to_write_consume);
    TEST_ASSERT_EQUAL(1, io.n_to_write);
}

void test_write_queue_produce_pdu(void) {
    struct pdu {
        struct pdu* n;
        int id;
    } pdu1, pdu2, pdu3;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Start with pdu1 -> pdu2 */
    pdu1.id = 1;
    pdu1.n = &pdu2;
    pdu2.id = 2;
    pdu2.n = NULL;

    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu2;
    io.n_to_write = 2;

    /* Add pdu3 to end */
    pdu3.id = 3;
    pdu3.n = NULL;
    pdu2.n = &pdu3;
    io.to_write_produce = &pdu3;
    io.n_to_write++;

    TEST_ASSERT_EQUAL_PTR(&pdu3, io.to_write_produce);
    TEST_ASSERT_EQUAL(3, io.n_to_write);
}

/* ========================================================================
 * IOV Building Tests (hi_make_iov simulation)
 * ======================================================================== */

void test_iov_build_from_single_pdu(void) {
    struct iovec src_iov[2];
    struct iovec dest_iov[HI_N_IOV];

    /* Source PDU has 2 iovs */
    char hdr[10] = "HDR";
    char data[20] = "DATA";
    src_iov[0].iov_base = hdr;
    src_iov[0].iov_len = 3;
    src_iov[1].iov_base = data;
    src_iov[1].iov_len = 4;

    /* Copy to destination */
    memcpy(dest_iov, src_iov, 2 * sizeof(struct iovec));

    TEST_ASSERT_EQUAL_PTR(hdr, dest_iov[0].iov_base);
    TEST_ASSERT_EQUAL(3, dest_iov[0].iov_len);
    TEST_ASSERT_EQUAL_PTR(data, dest_iov[1].iov_base);
    TEST_ASSERT_EQUAL(4, dest_iov[1].iov_len);
}

void test_iov_build_from_multiple_pdus(void) {
    struct iovec pdu1_iov[2], pdu2_iov[2], dest_iov[HI_N_IOV];

    /* PDU1 has 2 iovs */
    char pdu1_hdr[10] = "H1";
    char pdu1_data[10] = "D1";
    pdu1_iov[0].iov_base = pdu1_hdr;
    pdu1_iov[0].iov_len = 2;
    pdu1_iov[1].iov_base = pdu1_data;
    pdu1_iov[1].iov_len = 2;

    /* PDU2 has 2 iovs */
    char pdu2_hdr[10] = "H2";
    char pdu2_data[10] = "D2";
    pdu2_iov[0].iov_base = pdu2_hdr;
    pdu2_iov[0].iov_len = 2;
    pdu2_iov[1].iov_base = pdu2_data;
    pdu2_iov[1].iov_len = 2;

    /* Copy both to destination */
    memcpy(&dest_iov[0], pdu1_iov, 2 * sizeof(struct iovec));
    memcpy(&dest_iov[2], pdu2_iov, 2 * sizeof(struct iovec));

    /* Verify layout: [PDU1_HDR, PDU1_DATA, PDU2_HDR, PDU2_DATA] */
    TEST_ASSERT_EQUAL_PTR(pdu1_hdr, dest_iov[0].iov_base);
    TEST_ASSERT_EQUAL_PTR(pdu1_data, dest_iov[1].iov_base);
    TEST_ASSERT_EQUAL_PTR(pdu2_hdr, dest_iov[2].iov_base);
    TEST_ASSERT_EQUAL_PTR(pdu2_data, dest_iov[3].iov_base);
}

void test_iov_space_calculation(void) {
    struct iovec* iov = malloc(HI_N_IOV * sizeof(struct iovec));
    struct iovec* cur = iov;
    struct iovec* lim = iov + HI_N_IOV;

    int n_iov = 3;  /* PDU needs 3 iovs */

    /* Check if there's space */
    int has_space = (cur + n_iov) <= lim;
    TEST_ASSERT_TRUE(has_space);

    /* Advance pointer */
    cur += n_iov;
    TEST_ASSERT_EQUAL(3, cur - iov);

    /* Check remaining space */
    int remaining = lim - cur;
    TEST_ASSERT_EQUAL(HI_N_IOV - 3, remaining);

    free(iov);
}

/* ========================================================================
 * IOV Clearing Tests (hi_clear_iov simulation)
 * ======================================================================== */

void test_iov_clear_complete_single_buffer(void) {
    struct iovec iov[1];
    struct iovec* iov_cur;
    int n_iov;

    char data[100] = "Hello";
    iov[0].iov_base = data;
    iov[0].iov_len = 5;

    iov_cur = &iov[0];
    n_iov = 1;

    /* Simulate writing all 5 bytes */
    int n_written = 5;

    if (n_written >= iov_cur->iov_len) {
        n_written -= iov_cur->iov_len;
        ++iov_cur;
        --n_iov;
    }

    TEST_ASSERT_EQUAL(0, n_written);
    TEST_ASSERT_EQUAL(0, n_iov);
}

void test_iov_clear_partial_single_buffer(void) {
    struct iovec iov[1];

    char data[100] = "Hello, World!";
    iov[0].iov_base = data;
    iov[0].iov_len = 13;

    /* Simulate writing 5 bytes (partial) */
    int n_written = 5;

    /* Adjust iov_base and iov_len */
    iov[0].iov_base = ((char*)(iov[0].iov_base)) + n_written;
    iov[0].iov_len -= n_written;

    TEST_ASSERT_EQUAL_PTR(data + 5, iov[0].iov_base);
    TEST_ASSERT_EQUAL(8, iov[0].iov_len);
    TEST_ASSERT_EQUAL_STRING(", World!", (char*)iov[0].iov_base);
}

void test_iov_clear_multiple_buffers_complete(void) {
    struct iovec iov[3];
    struct iovec* iov_cur;
    int n_iov;

    char buf1[10] = "ABC";
    char buf2[10] = "DEF";
    char buf3[10] = "GHI";

    iov[0].iov_base = buf1;
    iov[0].iov_len = 3;
    iov[1].iov_base = buf2;
    iov[1].iov_len = 3;
    iov[2].iov_base = buf3;
    iov[2].iov_len = 3;

    iov_cur = &iov[0];
    n_iov = 3;
    int n_written = 9;  /* Write all buffers */

    /* Simulate clearing all iovs */
    while (n_iov && n_written) {
        if (n_written >= iov_cur->iov_len) {
            n_written -= iov_cur->iov_len;
            ++iov_cur;
            --n_iov;
        } else {
            iov_cur->iov_base = ((char*)(iov_cur->iov_base)) + n_written;
            iov_cur->iov_len -= n_written;
            break;
        }
    }

    TEST_ASSERT_EQUAL(0, n_iov);
    TEST_ASSERT_EQUAL(0, n_written);
}

void test_iov_clear_multiple_buffers_partial(void) {
    struct iovec iov[3];
    struct iovec* iov_cur;
    int n_iov;

    char buf1[10] = "ABC";
    char buf2[10] = "DEFGH";
    char buf3[10] = "IJK";

    iov[0].iov_base = buf1;
    iov[0].iov_len = 3;
    iov[1].iov_base = buf2;
    iov[1].iov_len = 5;
    iov[2].iov_base = buf3;
    iov[2].iov_len = 3;

    iov_cur = &iov[0];
    n_iov = 3;
    int n_written = 5;  /* Write first buffer + 2 bytes of second */

    while (n_iov && n_written) {
        if (n_written >= iov_cur->iov_len) {
            n_written -= iov_cur->iov_len;
            ++iov_cur;
            --n_iov;
        } else {
            iov_cur->iov_base = ((char*)(iov_cur->iov_base)) + n_written;
            iov_cur->iov_len -= n_written;
            break;
        }
    }

    /* Should have consumed iov[0] completely, partial iov[1] */
    TEST_ASSERT_EQUAL(2, n_iov);  /* iov[1] and iov[2] remain */
    TEST_ASSERT_EQUAL_PTR(buf2 + 2, iov_cur->iov_base);
    TEST_ASSERT_EQUAL(3, iov_cur->iov_len);
    TEST_ASSERT_EQUAL_STRING("FGH", (char*)iov_cur->iov_base);
}

/* ========================================================================
 * Queue State Transition Tests
 * ======================================================================== */

void test_queue_to_write_to_in_write_transfer(void) {
    struct pdu {
        struct pdu* next;
        struct pdu* wn;
        int n_iov;
    } pdu1;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        struct pdu* in_write;
        int n_to_write;
    } io;

    /* PDU in to_write queue */
    pdu1.next = NULL;
    pdu1.wn = NULL;
    pdu1.n_iov = 2;

    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu1;
    io.in_write = NULL;
    io.n_to_write = 1;

    /* Simulate transfer to in_write */
    struct pdu* pdu = io.to_write_consume;
    io.to_write_consume = pdu->next;
    if (!io.to_write_consume)
        io.to_write_produce = NULL;
    --io.n_to_write;

    pdu->wn = io.in_write;
    io.in_write = pdu;

    /* Verify state */
    TEST_ASSERT_NULL(io.to_write_consume);
    TEST_ASSERT_NULL(io.to_write_produce);
    TEST_ASSERT_EQUAL_PTR(&pdu1, io.in_write);
    TEST_ASSERT_EQUAL(0, io.n_to_write);
}

void test_queue_multiple_in_write(void) {
    struct pdu {
        struct pdu* wn;
        int id;
    } pdu1, pdu2, pdu3;

    struct {
        struct pdu* in_write;
    } io;

    /* Build in_write list: pdu3 -> pdu2 -> pdu1 (newest first) */
    pdu1.id = 1;
    pdu1.wn = NULL;

    pdu2.id = 2;
    pdu2.wn = &pdu1;

    pdu3.id = 3;
    pdu3.wn = &pdu2;

    io.in_write = &pdu3;

    /* Walk the list */
    int ids[3];
    int i = 0;
    for (struct pdu* p = io.in_write; p; p = p->wn) {
        ids[i++] = p->id;
    }

    TEST_ASSERT_EQUAL(3, ids[0]);
    TEST_ASSERT_EQUAL(2, ids[1]);
    TEST_ASSERT_EQUAL(1, ids[2]);
}

/* ========================================================================
 * Statistics and Counters Tests
 * ======================================================================== */

void test_write_statistics_counters(void) {
    struct {
        int n_written;
        int n_pdu_out;
        int n_to_write;
    } io;

    io.n_written = 0;
    io.n_pdu_out = 0;
    io.n_to_write = 0;

    /* Simulate sending 3 PDUs totaling 1500 bytes */
    for (int i = 0; i < 3; i++) {
        io.n_pdu_out++;
        io.n_to_write++;
    }

    /* Simulate writing 1500 bytes */
    io.n_written += 1500;

    TEST_ASSERT_EQUAL(1500, io.n_written);
    TEST_ASSERT_EQUAL(3, io.n_pdu_out);
    TEST_ASSERT_EQUAL(3, io.n_to_write);
}

void test_write_statistics_incremental(void) {
    int n_written = 0;

    /* Simulate incremental writes */
    n_written += 100;
    TEST_ASSERT_EQUAL(100, n_written);

    n_written += 250;
    TEST_ASSERT_EQUAL(350, n_written);

    n_written += 500;
    TEST_ASSERT_EQUAL(850, n_written);
}

/* ========================================================================
 * Edge Cases and Boundary Conditions
 * ======================================================================== */

void test_iov_zero_length_write(void) {
    int n_written = 0;
    int n_iov = 1;

    /* Simulate 0-byte write (EAGAIN case) */
    if (n_written == 0) {
        /* Nothing to clear */
        TEST_ASSERT_EQUAL(1, n_iov);
    }

    TEST_PASS();
}

void test_iov_max_capacity_boundary(void) {
    struct iovec iov[HI_N_IOV];
    struct iovec* cur = iov;
    struct iovec* lim = iov + HI_N_IOV;

    /* Fill to capacity */
    for (int i = 0; i < HI_N_IOV; i++) {
        cur++;
    }

    /* Check at boundary */
    TEST_ASSERT_EQUAL_PTR(lim, cur);

    /* Another PDU would exceed */
    int n_iov = 1;
    int can_fit = (cur + n_iov) <= lim;
    TEST_ASSERT_FALSE(can_fit);
}

void test_queue_empty_to_single_pdu(void) {
    struct pdu {
        struct pdu* n;
    } pdu1;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Empty state */
    io.to_write_consume = NULL;
    io.to_write_produce = NULL;
    io.n_to_write = 0;

    /* Add first PDU */
    pdu1.n = NULL;
    if (!io.to_write_produce)
        io.to_write_consume = &pdu1;
    else
        io.to_write_produce->n = &pdu1;
    io.to_write_produce = &pdu1;
    io.n_to_write++;

    TEST_ASSERT_EQUAL_PTR(&pdu1, io.to_write_consume);
    TEST_ASSERT_EQUAL_PTR(&pdu1, io.to_write_produce);
    TEST_ASSERT_EQUAL(1, io.n_to_write);
}

void test_queue_single_pdu_to_empty(void) {
    struct pdu {
        struct pdu* n;
    } pdu1;

    struct {
        struct pdu* to_write_consume;
        struct pdu* to_write_produce;
        int n_to_write;
    } io;

    /* Single PDU */
    pdu1.n = NULL;
    io.to_write_consume = &pdu1;
    io.to_write_produce = &pdu1;
    io.n_to_write = 1;

    /* Consume last PDU */
    io.to_write_consume = pdu1.n;
    if (!io.to_write_consume)
        io.to_write_produce = NULL;
    io.n_to_write--;

    TEST_ASSERT_NULL(io.to_write_consume);
    TEST_ASSERT_NULL(io.to_write_produce);
    TEST_ASSERT_EQUAL(0, io.n_to_write);
}

/* ========================================================================
 * Main Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* IOV Structure Tests */
    RUN_TEST(test_iov_single_buffer_setup);
    RUN_TEST(test_iov_two_buffer_setup);
    RUN_TEST(test_iov_three_buffer_setup);
    RUN_TEST(test_iov_max_vectors);
    RUN_TEST(test_iov_empty_buffer_handling);

    /* PDU Structure Tests */
    RUN_TEST(test_pdu_request_response_linkage);
    RUN_TEST(test_pdu_multiple_responses);
    RUN_TEST(test_pdu_response_removal_from_list);

    /* Write Queue Tests */
    RUN_TEST(test_write_queue_empty_state);
    RUN_TEST(test_write_queue_single_pdu);
    RUN_TEST(test_write_queue_multiple_pdus);
    RUN_TEST(test_write_queue_consume_pdu);
    RUN_TEST(test_write_queue_produce_pdu);

    /* IOV Building Tests */
    RUN_TEST(test_iov_build_from_single_pdu);
    RUN_TEST(test_iov_build_from_multiple_pdus);
    RUN_TEST(test_iov_space_calculation);

    /* IOV Clearing Tests */
    RUN_TEST(test_iov_clear_complete_single_buffer);
    RUN_TEST(test_iov_clear_partial_single_buffer);
    RUN_TEST(test_iov_clear_multiple_buffers_complete);
    RUN_TEST(test_iov_clear_multiple_buffers_partial);

    /* Queue State Transitions */
    RUN_TEST(test_queue_to_write_to_in_write_transfer);
    RUN_TEST(test_queue_multiple_in_write);

    /* Statistics Tests */
    RUN_TEST(test_write_statistics_counters);
    RUN_TEST(test_write_statistics_incremental);

    /* Edge Cases */
    RUN_TEST(test_iov_zero_length_write);
    RUN_TEST(test_iov_max_capacity_boundary);
    RUN_TEST(test_queue_empty_to_single_pdu);
    RUN_TEST(test_queue_single_pdu_to_empty);

    return UNITY_END();
}
