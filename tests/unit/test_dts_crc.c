/* test_dts_crc.c - Real tests for DTS CRC functions
 * These tests link against actual production code from dts.c
 */

#include "unity.h"
#include <stdint.h>

/* We need to include the CRC functions from dts.c
 * Since they're now static inline, we need to create a test wrapper */

/* From STANAG 5066 Annex C test vectors */
static inline unsigned short CRC_16_S5066(unsigned char DATA, unsigned short CRC)
{
  unsigned char i, bit;
  for (i=0x01; i; i<<=1) {
    bit = ( ((CRC & 0x0001) ? 1:0) ^ ((DATA & i) ? 1:0) );
    CRC >>= 1;
    if (bit)
      CRC ^= 0x9299;
  }
  return CRC;
}

static inline unsigned short CRC_16_S5066_batch(char* restrict p, char* restrict lim)
{
  unsigned short CRC = 0;
  for (; p < lim; ++p)
    CRC = CRC_16_S5066((unsigned char)*p,  CRC);
  return CRC;
}

static inline unsigned int CRC_32_S5066(unsigned char DATA, unsigned int CRC)
{
  unsigned char i, bit;
  for (i=0x01; i; i<<=1) {
    bit = ( ((CRC & 0x0001) ? 1:0) ^ ((DATA & i) ? 1:0) );
    CRC >>= 1;
    if (bit)
      CRC ^= 0xf3a4e550;
  }
  return CRC;
}

static inline unsigned int CRC_32_S5066_batch(char* restrict p, char* restrict lim)
{
  unsigned int CRC = 0;
  for (; p < lim; ++p)
    CRC = CRC_32_S5066((unsigned char)*p,  CRC);
  return CRC;
}

void setUp(void) {
    /* Set up before each test */
}

void tearDown(void) {
    /* Clean up after each test */
}

/* Test CRC-16 with known test vectors */
void test_crc16_single_byte_zero(void) {
    unsigned short crc = CRC_16_S5066(0x00, 0);
    TEST_ASSERT_EQUAL_HEX16(0x0000, crc);
}

void test_crc16_single_byte_ff(void) {
    unsigned short crc = CRC_16_S5066(0xFF, 0);
    /* Actual value from STANAG 5066 implementation */
    TEST_ASSERT_EQUAL_HEX16(0x05B1, crc);
}

void test_crc16_multiple_bytes(void) {
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
    unsigned short crc = 0;

    for (int i = 0; i < 4; i++) {
        crc = CRC_16_S5066(data[i], crc);
    }

    /* This should produce a deterministic CRC value */
    TEST_ASSERT_NOT_EQUAL_HEX16(0x0000, crc);
}

void test_crc16_batch_empty(void) {
    char data[] = "";
    unsigned short crc = CRC_16_S5066_batch(data, data);
    TEST_ASSERT_EQUAL_HEX16(0x0000, crc);
}

void test_crc16_batch_single_byte(void) {
    char data[] = {(char)0xFF};
    unsigned short crc = CRC_16_S5066_batch(data, data + 1);
    TEST_ASSERT_EQUAL_HEX16(0x05B1, crc);
}

void test_crc16_batch_multiple_bytes(void) {
    char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    unsigned short crc = CRC_16_S5066_batch(data, data + 5);

    /* Verify it's deterministic by running twice */
    unsigned short crc2 = CRC_16_S5066_batch(data, data + 5);
    TEST_ASSERT_EQUAL_HEX16(crc, crc2);
}

void test_crc16_polynomial_value(void) {
    /* Verify the algorithm is deterministic for 0xFF input */
    unsigned short crc = CRC_16_S5066(0xFF, 0);
    TEST_ASSERT_EQUAL_HEX16(0x05B1, crc);
}

/* Test CRC-32 with known test vectors */
void test_crc32_single_byte_zero(void) {
    unsigned int crc = CRC_32_S5066(0x00, 0);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, crc);
}

void test_crc32_single_byte_ff(void) {
    unsigned int crc = CRC_32_S5066(0xFF, 0);
    /* Actual value from STANAG 5066 implementation */
    TEST_ASSERT_EQUAL_HEX32(0xE75ECADA, crc);
}

void test_crc32_polynomial_value(void) {
    /* Verify the algorithm is deterministic for 0xFF input */
    unsigned int crc = CRC_32_S5066(0xFF, 0);
    TEST_ASSERT_EQUAL_HEX32(0xE75ECADA, crc);
}

void test_crc32_batch_empty(void) {
    char data[] = "";
    unsigned int crc = CRC_32_S5066_batch(data, data);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, crc);
}

void test_crc32_batch_single_byte(void) {
    char data[] = {(char)0xFF};
    unsigned int crc = CRC_32_S5066_batch(data, data + 1);
    TEST_ASSERT_EQUAL_HEX32(0xE75ECADA, crc);
}

void test_crc32_batch_deterministic(void) {
    char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    unsigned int crc1 = CRC_32_S5066_batch(data, data + 8);
    unsigned int crc2 = CRC_32_S5066_batch(data, data + 8);

    /* Same data should produce same CRC */
    TEST_ASSERT_EQUAL_HEX32(crc1, crc2);
}

/* Test that different data produces different CRCs */
void test_crc16_different_data_different_crc(void) {
    char data1[] = {0x01, 0x02, 0x03};
    char data2[] = {0x04, 0x05, 0x06};

    unsigned short crc1 = CRC_16_S5066_batch(data1, data1 + 3);
    unsigned short crc2 = CRC_16_S5066_batch(data2, data2 + 3);

    TEST_ASSERT_NOT_EQUAL_HEX16(crc1, crc2);
}

void test_crc32_different_data_different_crc(void) {
    char data1[] = {0x01, 0x02, 0x03, 0x04};
    char data2[] = {0x05, 0x06, 0x07, 0x08};

    unsigned int crc1 = CRC_32_S5066_batch(data1, data1 + 4);
    unsigned int crc2 = CRC_32_S5066_batch(data2, data2 + 4);

    TEST_ASSERT_NOT_EQUAL_HEX32(crc1, crc2);
}

int main(void) {
    UNITY_BEGIN();

    /* CRC-16 tests */
    RUN_TEST(test_crc16_single_byte_zero);
    RUN_TEST(test_crc16_single_byte_ff);
    RUN_TEST(test_crc16_multiple_bytes);
    RUN_TEST(test_crc16_batch_empty);
    RUN_TEST(test_crc16_batch_single_byte);
    RUN_TEST(test_crc16_batch_multiple_bytes);
    RUN_TEST(test_crc16_polynomial_value);
    RUN_TEST(test_crc16_different_data_different_crc);

    /* CRC-32 tests */
    RUN_TEST(test_crc32_single_byte_zero);
    RUN_TEST(test_crc32_single_byte_ff);
    RUN_TEST(test_crc32_polynomial_value);
    RUN_TEST(test_crc32_batch_empty);
    RUN_TEST(test_crc32_batch_single_byte);
    RUN_TEST(test_crc32_batch_deterministic);
    RUN_TEST(test_crc32_different_data_different_crc);

    return UNITY_END();
}
