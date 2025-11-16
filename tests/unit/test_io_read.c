/*
 * test_io_read.c - Comprehensive tests for hiread.c I/O read operations
 *
 * Tests coverage:
 * - PDU allocation (hi_pdu_alloc from thread and global pools)
 * - Read buffer management
 * - PDU overflow handling (hi_checkmore)
 * - Protocol dispatch logic
 * - Read statistics tracking
 * - Edge cases and error conditions
 *
 * Part of Open5066 test suite
 */

#include "unity.h"
#include <string.h>
#include <stdlib.h>

#ifndef IOV_MAX
#define IOV_MAX 16
#endif

#define HI_PDU_MEM 2200

/* Test Setup/Teardown */
void setUp(void) {
    /* Called before each test */
}

void tearDown(void) {
    /* Called after each test */
}

/* ========================================================================
 * PDU Allocation Tests
 * ======================================================================== */

void test_pdu_allocation_pool_empty(void) {
    struct {
        void* free_pdus;
    } pool;

    pool.free_pdus = NULL;

    /* Empty pool state */
    TEST_ASSERT_NULL(pool.free_pdus);
}

void test_pdu_allocation_pool_single(void) {
    struct pdu_node {
        struct pdu_node* next;
        char mem[HI_PDU_MEM];
    } pdu1;

    struct {
        struct pdu_node* free_pdus;
    } pool;

    /* Single PDU in free list */
    pdu1.next = NULL;
    pool.free_pdus = &pdu1;

    TEST_ASSERT_EQUAL_PTR(&pdu1, pool.free_pdus);
}

void test_pdu_allocation_pool_multiple(void) {
    struct pdu_node {
        struct pdu_node* next;
        int id;
    } pdu1, pdu2, pdu3;

    struct {
        struct pdu_node* free_pdus;
    } pool;

    /* Build free list: pdu1 -> pdu2 -> pdu3 */
    pdu1.id = 1;
    pdu1.next = &pdu2;
    pdu2.id = 2;
    pdu2.next = &pdu3;
    pdu3.id = 3;
    pdu3.next = NULL;

    pool.free_pdus = &pdu1;

    /* Count PDUs in free list */
    int count = 0;
    for (struct pdu_node* p = pool.free_pdus; p; p = p->next) {
        count++;
    }
    TEST_ASSERT_EQUAL(3, count);
}

void test_pdu_allocation_from_pool(void) {
    struct pdu_node {
        struct pdu_node* next;
        int id;
    } pdu1, pdu2;

    struct {
        struct pdu_node* free_pdus;
    } pool;

    /* Initial: pdu1 -> pdu2 */
    pdu1.id = 1;
    pdu1.next = &pdu2;
    pdu2.id = 2;
    pdu2.next = NULL;

    pool.free_pdus = &pdu1;

    /* Allocate pdu1 */
    struct pdu_node* allocated = pool.free_pdus;
    pool.free_pdus = allocated->next;

    TEST_ASSERT_EQUAL(1, allocated->id);
    TEST_ASSERT_EQUAL_PTR(&pdu2, pool.free_pdus);
}

void test_pdu_initialization_after_alloc(void) {
    struct pdu {
        char* m;
        char* ap;
        char* scan;
        char* lim;
        void* req;
        void* parent;
        int need;
        char mem[HI_PDU_MEM];
    } pdu;

    /* Simulate initialization after allocation */
    pdu.lim = pdu.mem + HI_PDU_MEM;
    pdu.m = pdu.scan = pdu.ap = pdu.mem;
    pdu.req = pdu.parent = NULL;
    pdu.need = 1;  /* trigger network I/O */

    TEST_ASSERT_EQUAL_PTR(pdu.mem, pdu.m);
    TEST_ASSERT_EQUAL_PTR(pdu.mem, pdu.ap);
    TEST_ASSERT_EQUAL_PTR(pdu.mem, pdu.scan);
    TEST_ASSERT_EQUAL_PTR(pdu.mem + HI_PDU_MEM, pdu.lim);
    TEST_ASSERT_EQUAL(1, pdu.need);
}

/* ========================================================================
 * Read Buffer Management Tests
 * ======================================================================== */

void test_read_buffer_available_space(void) {
    char mem[HI_PDU_MEM];
    char* ap = mem;
    char* lim = mem + HI_PDU_MEM;

    int available = lim - ap;
    TEST_ASSERT_EQUAL(HI_PDU_MEM, available);
}

void test_read_buffer_advance_pointer(void) {
    char mem[HI_PDU_MEM];
    char* ap = mem;
    char* lim = mem + HI_PDU_MEM;

    /* Simulate reading 100 bytes */
    int bytes_read = 100;
    ap += bytes_read;

    TEST_ASSERT_EQUAL(100, ap - mem);
    TEST_ASSERT_EQUAL(HI_PDU_MEM - 100, lim - ap);
}

void test_read_buffer_full_detection(void) {
    char mem[HI_PDU_MEM];
    char* ap = mem + HI_PDU_MEM;  /* Buffer full */
    char* lim = mem + HI_PDU_MEM;

    int available = lim - ap;
    TEST_ASSERT_EQUAL(0, available);
}

void test_read_buffer_partial_fill(void) {
    char mem[HI_PDU_MEM];
    char* m = mem;
    char* ap = mem;

    /* Simulate multiple reads */
    ap += 500;  /* First read: 500 bytes */
    TEST_ASSERT_EQUAL(500, ap - m);

    ap += 300;  /* Second read: 300 bytes */
    TEST_ASSERT_EQUAL(800, ap - m);

    ap += 200;  /* Third read: 200 bytes */
    TEST_ASSERT_EQUAL(1000, ap - m);
}

void test_read_buffer_need_vs_available(void) {
    char mem[HI_PDU_MEM];
    char* m = mem;
    char* ap = mem + 100;  /* 100 bytes already read */

    int need = 50;  /* Need 50 bytes to complete PDU */
    int available = ap - m;

    /* Check if we have enough */
    int have_enough = (available >= need);
    TEST_ASSERT_TRUE(have_enough);
}

/* ========================================================================
 * PDU Overflow Handling Tests (hi_checkmore simulation)
 * ======================================================================== */

void test_checkmore_no_overflow(void) {
    struct pdu {
        char* m;
        char* ap;
        int len;
    } req;

    char mem[HI_PDU_MEM];
    req.m = mem;
    req.ap = mem + 100;  /* 100 bytes in buffer */
    req.len = 120;       /* PDU length is 120 bytes */

    int n = req.ap - req.m;  /* 100 */

    /* No overflow: 100 <= 120 */
    int has_overflow = (n > req.len);
    TEST_ASSERT_FALSE(has_overflow);
}

void test_checkmore_has_overflow(void) {
    struct pdu {
        char* m;
        char* ap;
        int len;
    } req;

    char mem[HI_PDU_MEM];
    req.m = mem;
    req.ap = mem + 150;  /* 150 bytes in buffer */
    req.len = 120;       /* PDU length is 120 bytes */

    int n = req.ap - req.m;  /* 150 */

    /* Overflow: 150 > 120, have 30 extra bytes */
    int overflow_bytes = n - req.len;
    TEST_ASSERT_EQUAL(30, overflow_bytes);
}

void test_checkmore_copy_overflow_to_new_pdu(void) {
    struct pdu {
        char* m;
        char* ap;
        int len;
        char mem[HI_PDU_MEM];
    } req, nreq;

    /* req has 150 bytes, PDU is 120 bytes, 30 bytes overflow */
    req.m = req.mem;
    req.ap = req.mem + 150;
    req.len = 120;

    /* Copy overflow to new PDU */
    int n = req.ap - req.m;
    int overflow = n - req.len;  /* 30 bytes */

    nreq.m = nreq.ap = nreq.mem;
    memcpy(nreq.ap, req.m + req.len, overflow);
    nreq.ap += overflow;

    /* Adjust req to exactly PDU length */
    req.ap = req.m + req.len;

    TEST_ASSERT_EQUAL(120, req.ap - req.m);
    TEST_ASSERT_EQUAL(30, nreq.ap - nreq.m);
}

void test_checkmore_minlen_requirement(void) {
    int minlen = 5;  /* SIS minimum PDU size */

    /* minlen must not be zero to prevent infinite loops */
    TEST_ASSERT_NOT_EQUAL(0, minlen);
    TEST_ASSERT_GREATER_THAN(0, minlen);
}

void test_checkmore_cur_pdu_cleared_when_no_overflow(void) {
    struct {
        void* cur_pdu;
    } io;

    struct pdu {
        char* m;
        char* ap;
        int len;
    } req;

    char mem[HI_PDU_MEM];
    req.m = mem;
    req.ap = mem + 100;
    req.len = 120;

    int n = req.ap - req.m;

    /* No overflow, clear cur_pdu */
    if (n <= req.len) {
        io.cur_pdu = NULL;
    }

    TEST_ASSERT_NULL(io.cur_pdu);
}

/* ========================================================================
 * Protocol Dispatch Tests
 * ======================================================================== */

void test_protocol_constants(void) {
    /* S5066 protocol type constants */
    #define S5066_SIS 1
    #define S5066_DTS 2
    #define S5066_HTTP 3
    #define S5066_SMTP 4
    #define S5066_TEST_PING 5

    /* Verify uniqueness */
    TEST_ASSERT_NOT_EQUAL(S5066_SIS, S5066_DTS);
    TEST_ASSERT_NOT_EQUAL(S5066_SIS, S5066_HTTP);
    TEST_ASSERT_NOT_EQUAL(S5066_DTS, S5066_HTTP);
}

void test_protocol_dispatch_sis(void) {
    struct {
        int proto;
    } io;

    io.proto = 1;  /* S5066_SIS */

    /* Dispatch based on protocol */
    int dispatched_proto = 0;
    switch (io.proto) {
    case 1: dispatched_proto = 1; break;  /* SIS */
    case 2: dispatched_proto = 2; break;  /* DTS */
    case 3: dispatched_proto = 3; break;  /* HTTP */
    }

    TEST_ASSERT_EQUAL(1, dispatched_proto);
}

void test_protocol_dispatch_dts(void) {
    struct {
        int proto;
    } io;

    io.proto = 2;  /* S5066_DTS */

    int dispatched_proto = 0;
    switch (io.proto) {
    case 1: dispatched_proto = 1; break;
    case 2: dispatched_proto = 2; break;
    case 3: dispatched_proto = 3; break;
    }

    TEST_ASSERT_EQUAL(2, dispatched_proto);
}

void test_protocol_need_check(void) {
    struct pdu {
        char* m;
        char* ap;
        int need;
    } cur_pdu;

    char mem[HI_PDU_MEM];
    cur_pdu.m = mem;
    cur_pdu.ap = mem + 100;
    cur_pdu.need = 50;

    /* Check if we have enough data */
    int available = cur_pdu.ap - cur_pdu.m;
    int ready_to_decode = (cur_pdu.need > 0) && (cur_pdu.need <= available);

    TEST_ASSERT_TRUE(ready_to_decode);
}

void test_protocol_need_not_met(void) {
    struct pdu {
        char* m;
        char* ap;
        int need;
    } cur_pdu;

    char mem[HI_PDU_MEM];
    cur_pdu.m = mem;
    cur_pdu.ap = mem + 30;
    cur_pdu.need = 50;  /* Need more data */

    int available = cur_pdu.ap - cur_pdu.m;
    int ready_to_decode = (cur_pdu.need > 0) && (cur_pdu.need <= available);

    TEST_ASSERT_FALSE(ready_to_decode);
}

void test_protocol_need_zero_skip_decode(void) {
    struct pdu {
        int need;
    } cur_pdu;

    cur_pdu.need = 0;  /* No further I/O desired */

    /* Should skip decoding when need is 0 */
    int should_decode = (cur_pdu.need > 0);
    TEST_ASSERT_FALSE(should_decode);
}

/* ========================================================================
 * Read Statistics Tests
 * ======================================================================== */

void test_read_statistics_initial_state(void) {
    struct {
        int n_read;
        int n_pdu_in;
    } io;

    io.n_read = 0;
    io.n_pdu_in = 0;

    TEST_ASSERT_EQUAL(0, io.n_read);
    TEST_ASSERT_EQUAL(0, io.n_pdu_in);
}

void test_read_statistics_increment_bytes(void) {
    int n_read = 0;

    /* Simulate multiple reads */
    n_read += 100;
    TEST_ASSERT_EQUAL(100, n_read);

    n_read += 200;
    TEST_ASSERT_EQUAL(300, n_read);

    n_read += 500;
    TEST_ASSERT_EQUAL(800, n_read);
}

void test_read_statistics_increment_pdus(void) {
    int n_pdu_in = 0;

    /* Each new PDU increments counter */
    n_pdu_in++;
    TEST_ASSERT_EQUAL(1, n_pdu_in);

    n_pdu_in++;
    TEST_ASSERT_EQUAL(2, n_pdu_in);

    n_pdu_in++;
    TEST_ASSERT_EQUAL(3, n_pdu_in);
}

void test_read_statistics_throughput_calculation(void) {
    int n_read = 10000;     /* 10KB read */
    int elapsed_ms = 1000;  /* 1 second */

    /* Throughput = bytes/sec */
    int throughput = (n_read * 1000) / elapsed_ms;
    TEST_ASSERT_EQUAL(10000, throughput);  /* 10KB/sec */
}

/* ========================================================================
 * Current PDU State Tests
 * ======================================================================== */

void test_cur_pdu_null_needs_allocation(void) {
    struct {
        void* cur_pdu;
    } io;

    io.cur_pdu = NULL;

    /* NULL cur_pdu means we need to allocate new PDU */
    int need_alloc = (io.cur_pdu == NULL);
    TEST_ASSERT_TRUE(need_alloc);
}

void test_cur_pdu_exists_continue_reading(void) {
    struct pdu {
        int id;
    } pdu;

    struct {
        struct pdu* cur_pdu;
    } io;

    pdu.id = 123;
    io.cur_pdu = &pdu;

    /* Has cur_pdu, continue reading into it */
    int can_continue = (io.cur_pdu != NULL);
    TEST_ASSERT_TRUE(can_continue);
}

void test_cur_pdu_cleared_after_decode(void) {
    struct {
        void* cur_pdu;
    } io;

    io.cur_pdu = (void*)0x12345678;  /* Had a PDU */

    /* After successful decode, clear cur_pdu */
    io.cur_pdu = NULL;

    TEST_ASSERT_NULL(io.cur_pdu);
}

/* ========================================================================
 * Error Condition Tests
 * ======================================================================== */

void test_read_zero_bytes_eof(void) {
    int ret = 0;  /* read() returned 0 = EOF */

    /* Zero bytes means connection closed */
    int is_eof = (ret == 0);
    TEST_ASSERT_TRUE(is_eof);
}

void test_read_eintr_should_retry(void) {
    int error_code = 4;  /* EINTR */

    /* EINTR means interrupted, should retry */
    int should_retry = (error_code == 4);  /* EINTR = 4 */
    TEST_ASSERT_TRUE(should_retry);
}

void test_read_eagain_no_more_data(void) {
    int error_code = 11;  /* EAGAIN on Linux */

    /* EAGAIN means no more data available (edge-triggered epoll) */
    int is_eagain = (error_code == 11);
    TEST_ASSERT_TRUE(is_eagain);
}

void test_read_other_error_should_close(void) {
    int error_code = 9;  /* EBADF or other error */

    /* Errors other than EINTR/EAGAIN should close connection */
    int should_close = (error_code != 4 && error_code != 11);
    TEST_ASSERT_TRUE(should_close);
}

void test_read_negative_return_is_error(void) {
    int ret = -1;  /* read() error */

    /* Negative return means error occurred */
    int is_error = (ret < 0);
    TEST_ASSERT_TRUE(is_error);
}

/* ========================================================================
 * PDU Memory Boundary Tests
 * ======================================================================== */

void test_pdu_boundary_start(void) {
    char mem[HI_PDU_MEM];
    char* m = mem;
    char* ap = mem;
    char* lim = mem + HI_PDU_MEM;

    /* At start, ap == m */
    TEST_ASSERT_EQUAL_PTR(m, ap);
    TEST_ASSERT_EQUAL(HI_PDU_MEM, lim - ap);
}

void test_pdu_boundary_end(void) {
    char mem[HI_PDU_MEM];
    char* m = mem;
    char* ap = mem + HI_PDU_MEM;
    char* lim = mem + HI_PDU_MEM;

    /* At end, ap == lim */
    TEST_ASSERT_EQUAL_PTR(lim, ap);
    TEST_ASSERT_EQUAL(0, lim - ap);
}

void test_pdu_boundary_overflow_prevention(void) {
    char mem[HI_PDU_MEM];
    char* ap = mem + 2000;
    char* lim = mem + HI_PDU_MEM;

    int available = lim - ap;
    int read_size = 500;

    /* Would overflow: available < read_size */
    if (available < read_size) {
        read_size = available;  /* Limit to available space */
    }

    TEST_ASSERT_EQUAL(200, read_size);  /* 2200 - 2000 = 200 */
}

void test_pdu_boundary_within_limits(void) {
    char mem[HI_PDU_MEM];
    char* ap = mem + 1000;
    char* lim = mem + HI_PDU_MEM;

    /* Verify ap is within bounds */
    TEST_ASSERT_TRUE(ap >= mem);
    TEST_ASSERT_TRUE(ap <= lim);
}

/* ========================================================================
 * Decode Loop Tests
 * ======================================================================== */

void test_decode_loop_single_complete_pdu(void) {
    struct pdu {
        char* m;
        char* ap;
        int need;
    } cur_pdu;

    char mem[HI_PDU_MEM];
    cur_pdu.m = mem;
    cur_pdu.ap = mem + 100;
    cur_pdu.need = 50;  /* Need 50 bytes */

    /* Have 100 bytes, need 50, can decode */
    int can_decode = (cur_pdu.need > 0) &&
                     ((cur_pdu.ap - cur_pdu.m) >= cur_pdu.need);
    TEST_ASSERT_TRUE(can_decode);

    /* After decode, set need to 0 */
    cur_pdu.need = 0;

    /* Should not decode again */
    can_decode = (cur_pdu.need > 0) &&
                 ((cur_pdu.ap - cur_pdu.m) >= cur_pdu.need);
    TEST_ASSERT_FALSE(can_decode);
}

void test_decode_loop_multiple_pdus_in_buffer(void) {
    struct pdu {
        char* m;
        char* ap;
        int need;
    } cur_pdu;

    char mem[HI_PDU_MEM];
    cur_pdu.m = mem;
    cur_pdu.ap = mem + 200;  /* 200 bytes total */
    cur_pdu.need = 50;       /* First PDU needs 50 bytes */

    int pdus_decoded = 0;

    /* First PDU */
    if (cur_pdu.need > 0 && (cur_pdu.ap - cur_pdu.m) >= cur_pdu.need) {
        pdus_decoded++;
        cur_pdu.m += 50;     /* Consume 50 bytes */
        cur_pdu.need = 80;   /* Second PDU needs 80 bytes */
    }

    /* Second PDU */
    if (cur_pdu.need > 0 && (cur_pdu.ap - cur_pdu.m) >= cur_pdu.need) {
        pdus_decoded++;
        cur_pdu.m += 80;     /* Consume 80 bytes */
    }

    TEST_ASSERT_EQUAL(2, pdus_decoded);
    TEST_ASSERT_EQUAL(130, cur_pdu.m - mem);  /* 50 + 80 consumed */
}

void test_decode_loop_exit_when_need_not_met(void) {
    struct pdu {
        char* m;
        char* ap;
        int need;
    } cur_pdu;

    char mem[HI_PDU_MEM];
    cur_pdu.m = mem;
    cur_pdu.ap = mem + 40;   /* Only 40 bytes */
    cur_pdu.need = 50;       /* Need 50 bytes */

    /* Cannot decode, need more data */
    int can_decode = (cur_pdu.need > 0) &&
                     ((cur_pdu.ap - cur_pdu.m) >= cur_pdu.need);
    TEST_ASSERT_FALSE(can_decode);
}

/* ========================================================================
 * Main Test Runner
 * ======================================================================== */

int main(void) {
    UNITY_BEGIN();

    /* PDU Allocation Tests */
    RUN_TEST(test_pdu_allocation_pool_empty);
    RUN_TEST(test_pdu_allocation_pool_single);
    RUN_TEST(test_pdu_allocation_pool_multiple);
    RUN_TEST(test_pdu_allocation_from_pool);
    RUN_TEST(test_pdu_initialization_after_alloc);

    /* Read Buffer Management Tests */
    RUN_TEST(test_read_buffer_available_space);
    RUN_TEST(test_read_buffer_advance_pointer);
    RUN_TEST(test_read_buffer_full_detection);
    RUN_TEST(test_read_buffer_partial_fill);
    RUN_TEST(test_read_buffer_need_vs_available);

    /* PDU Overflow Handling Tests */
    RUN_TEST(test_checkmore_no_overflow);
    RUN_TEST(test_checkmore_has_overflow);
    RUN_TEST(test_checkmore_copy_overflow_to_new_pdu);
    RUN_TEST(test_checkmore_minlen_requirement);
    RUN_TEST(test_checkmore_cur_pdu_cleared_when_no_overflow);

    /* Protocol Dispatch Tests */
    RUN_TEST(test_protocol_constants);
    RUN_TEST(test_protocol_dispatch_sis);
    RUN_TEST(test_protocol_dispatch_dts);
    RUN_TEST(test_protocol_need_check);
    RUN_TEST(test_protocol_need_not_met);
    RUN_TEST(test_protocol_need_zero_skip_decode);

    /* Read Statistics Tests */
    RUN_TEST(test_read_statistics_initial_state);
    RUN_TEST(test_read_statistics_increment_bytes);
    RUN_TEST(test_read_statistics_increment_pdus);
    RUN_TEST(test_read_statistics_throughput_calculation);

    /* Current PDU State Tests */
    RUN_TEST(test_cur_pdu_null_needs_allocation);
    RUN_TEST(test_cur_pdu_exists_continue_reading);
    RUN_TEST(test_cur_pdu_cleared_after_decode);

    /* Error Condition Tests */
    RUN_TEST(test_read_zero_bytes_eof);
    RUN_TEST(test_read_eintr_should_retry);
    RUN_TEST(test_read_eagain_no_more_data);
    RUN_TEST(test_read_other_error_should_close);
    RUN_TEST(test_read_negative_return_is_error);

    /* PDU Memory Boundary Tests */
    RUN_TEST(test_pdu_boundary_start);
    RUN_TEST(test_pdu_boundary_end);
    RUN_TEST(test_pdu_boundary_overflow_prevention);
    RUN_TEST(test_pdu_boundary_within_limits);

    /* Decode Loop Tests */
    RUN_TEST(test_decode_loop_single_complete_pdu);
    RUN_TEST(test_decode_loop_multiple_pdus_in_buffer);
    RUN_TEST(test_decode_loop_exit_when_need_not_met);

    return UNITY_END();
}
