# Test Makefile for Open5066
# Uses Unity test framework

CC=gcc
CFLAGS=-g -Wall -Wextra -I../src/include -I../src/core -I../src/serial -I../src/protocols -Iunit
LDFLAGS=-lpthread

# Test files
UNITY_SRC=unit/unity.c
UNIT_TESTS=$(wildcard unit/test_*.c)
UNIT_TEST_BINS=$(UNIT_TESTS:.c=)

# Source files to test
SRC_DIR=../src
CORE_SRC=$(SRC_DIR)/core/util.c
SERIAL_SRC=$(SRC_DIR)/serial/globalcounter.c

.PHONY: all test clean unit integration security

all: test

# Build and run all tests
test: unit

# Unit tests
unit: $(UNIT_TEST_BINS)
	@echo "Running unit tests..."
	@for test in $(UNIT_TEST_BINS); do \
		echo ""; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All unit tests passed!"

# Pattern rule for building unit tests (without core dependencies for now)
unit/test_%: unit/test_%.c $(UNITY_SRC)
	@echo "Building $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

integration:
	@echo "Integration tests not yet implemented"

security:
	@echo "Security tests not yet implemented"

clean:
	rm -f unit/test_* integration/test_* security/test_*
	rm -f *.o

help:
	@echo "Open5066 Test Suite"
	@echo "==================="
	@echo ""
	@echo "Targets:"
	@echo "  make test        - Run all tests"
	@echo "  make unit        - Run unit tests only"
	@echo "  make integration - Run integration tests"
	@echo "  make security    - Run security tests"
	@echo "  make clean       - Remove test binaries"
