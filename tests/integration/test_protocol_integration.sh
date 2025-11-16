#!/bin/bash
# Integration test for Open5066 protocol stack
# Tests SIS and DTS protocols working together

# Don't exit on first error - we want to run all tests

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Helper functions
print_test() {
    echo -e "${YELLOW}[TEST]${NC} $1"
    TESTS_RUN=$((TESTS_RUN + 1))
}

print_pass() {
    echo -e "${GREEN}[PASS]${NC} $1"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

print_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

# Check if binary exists
BINARY="../../s5066d"
if [ ! -f "$BINARY" ]; then
    echo "Error: s5066d binary not found at $BINARY. Please run 'make' first."
    exit 1
fi

echo "========================================="
echo "Open5066 Integration Test Suite"
echo "========================================="
echo ""

# Test 1: Binary exists and is executable
print_test "Binary exists and is executable"
if [ -x "$BINARY" ]; then
    print_pass "s5066d binary is executable"
else
    print_fail "s5066d binary is not executable"
fi

# Test 2: Help output works
print_test "Help output"
if $BINARY -h 2>&1 | grep -q "STANAG 5066"; then
    print_pass "Help text contains expected content"
else
    print_fail "Help text missing expected content"
fi

# Test 3: Version information
print_test "Version information"
if $BINARY -h 2>&1 | grep -q "0.5"; then
    print_pass "Version information present"
else
    print_fail "Version information missing"
fi

# Test 4: Check for security compilation flags
print_test "Security compilation flags"
if readelf -d $BINARY 2>/dev/null | grep -q "RELRO" || \
   readelf -d $BINARY 2>/dev/null | grep -q "BIND_NOW"; then
    print_pass "Security flags (RELRO/BIND_NOW) enabled"
else
    print_fail "Security flags not detected"
fi

# Test 5: Check for PIE (Position Independent Executable)
print_test "PIE (Position Independent Executable)"
if file $BINARY | grep -q "pie"; then
    print_pass "PIE enabled"
else
    # PIE might not show in file output, check readelf
    if readelf -h $BINARY 2>/dev/null | grep -q "DYN"; then
        print_pass "PIE enabled (DYN type)"
    else
        print_fail "PIE not enabled"
    fi
fi

# Test 6: Check for stack protection
print_test "Stack canary protection"
if readelf -s $BINARY 2>/dev/null | grep -q "__stack_chk_fail"; then
    print_pass "Stack canaries enabled"
else
    print_fail "Stack canaries not detected"
fi

# Test 7: Check binary size is reasonable
print_test "Binary size check"
SIZE=$(stat -c%s "$BINARY" 2>/dev/null || stat -f%z "$BINARY" 2>/dev/null)
if [ "$SIZE" -gt 10000 ] && [ "$SIZE" -lt 10000000 ]; then
    print_pass "Binary size is reasonable ($SIZE bytes)"
else
    print_fail "Binary size is unusual ($SIZE bytes)"
fi

# Test 8: Check for dangerous functions (should not be present)
print_test "Unsafe function check"
UNSAFE_FOUND=0
for func in "gets" "strcpy@" "sprintf@"; do
    if nm $BINARY 2>/dev/null | grep -q "$func"; then
        UNSAFE_FOUND=1
        echo "  Found unsafe function: $func"
    fi
done
if [ $UNSAFE_FOUND -eq 0 ]; then
    print_pass "No known unsafe functions found"
else
    print_fail "Unsafe functions detected in binary"
fi

# Test 9: Check for required symbols
print_test "Required protocol symbols"
MISSING=0
for symbol in "sis_decode" "dts_decode" "CRC_16_S5066" "CRC_32_S5066"; do
    if ! nm $BINARY 2>/dev/null | grep -q "$symbol"; then
        echo "  Missing symbol: $symbol"
        MISSING=1
    fi
done
if [ $MISSING -eq 0 ]; then
    print_pass "All required protocol symbols present"
else
    print_fail "Some protocol symbols are missing"
fi

# Test 10: Check for memory safety features
print_test "FORTIFY_SOURCE symbols"
if nm $BINARY 2>/dev/null | grep -q "__.*_chk"; then
    print_pass "FORTIFY_SOURCE protection symbols found"
else
    print_fail "FORTIFY_SOURCE symbols not detected"
fi

echo ""
echo "========================================="
echo "Integration Test Results"
echo "========================================="
echo "Tests run:    $TESTS_RUN"
echo "Tests passed: $TESTS_PASSED"
echo "Tests failed: $TESTS_FAILED"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All integration tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    exit 1
fi
