################################################################################
# DEPRECATED: This Makefile is deprecated
#
# Tests are now built and run using CMake/CTest:
#
#   cd build
#   cmake ..
#   make              # Builds all tests
#   ctest             # Runs all tests
#
# See docs/CMAKE_MIGRATION.md for details.
################################################################################

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
test: unit integration security

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
	@echo "Running integration tests..."
	@cd integration && ./test_protocol_integration.sh

security: security/test_protocol_security
	@echo "Running security tests..."
	@./security/test_protocol_security

security/test_protocol_security: security/test_protocol_security.c $(UNITY_SRC)
	@echo "Building $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f unit/test_* security/test_protocol_security
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
