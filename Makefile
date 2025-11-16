################################################################################
# DEPRECATED: This Makefile is deprecated and kept for backwards compatibility
#
# The project has migrated to CMake as the primary build system.
# Please use CMake for new development:
#
#   mkdir build && cd build
#   cmake ..
#   make
#   ctest
#
# See docs/CMAKE_MIGRATION.md for detailed migration guide.
# This Makefile will be removed in a future version.
################################################################################

# Makefile  -  How to build NATO STANAG 5066 Annex A, B, and C daemon
# Copyright (c) 2006 Sampo Kellomaki (sampo@iki.fi), All Rights Reserved.
# See file COPYING.
#
# Build so far only tested on Linux. This makefile needs gmake-3.78 or newer.
#
# Usage:   make              # Linux
#          make TARGET=sol8  # Sparc Solaris 8 native
#          make TARGET=xsol8 # Sparc Solaris 8 cross compile (on Linux?)

# Source directories
SRCDIR=src
CORE_DIR=$(SRCDIR)/core
PROTO_DIR=$(SRCDIR)/protocols
SERIAL_DIR=$(SRCDIR)/serial
INCLUDE_DIR=$(SRCDIR)/include
TOOLS_DIR=$(SRCDIR)/tools
BUILD_DIR=build
COVERAGE_DIR=coverage

vpath %.c $(CORE_DIR):$(PROTO_DIR):$(SERIAL_DIR):$(TOOLS_DIR)
vpath %.h $(CORE_DIR):$(INCLUDE_DIR):$(SERIAL_DIR):$(PROTO_DIR)

REL=0.5
VERSION=0x000005

CC=gcc
LD=gcc
CDIR=-I$(INCLUDE_DIR) -I$(CORE_DIR) -I$(SERIAL_DIR) -I$(PROTO_DIR)
CDEF=-D_REENTRANT -DDEBUG -DS5066D=$(VERSION) -DTEST_PING -DREL="\"$(REL)\""
#CDEF=-D_REENTRANT -DDEBUG -DS5066D=$(VERSION) -DCOMPILED_DATE=`date +%s`
LIBS=-lpthread

# Security and warning flags (enhanced)
SECURITY_FLAGS=-Wall -Wextra -Wformat=2 -Wformat-security \
               -fstack-protector-strong -D_FORTIFY_SOURCE=2 \
               -fPIE -Wl,-z,relro -Wl,-z,now
WARN_FLAGS=-Wno-unused-label -Wno-unknown-pragmas

# Coverage flags (use 'make COVERAGE=1' to enable)
ifdef COVERAGE
CFLAGS_COVERAGE=-fprofile-arcs -ftest-coverage
LDFLAGS_COVERAGE=-lgcov --coverage
endif

# Test boxes Nito
REMOTEHOST=quebec.cellmail.com
REMOTEDIR=w/src/s5066/

ifeq ($(TARGET),xsol8)

# Cross compilation for Solaris 8 target (on Linux host) Invoke as `make PLATFORM=xsol8'
# You must have the cross compiler installed in /apps/gcc/sol8 and in path. Similarily
# the cross binutils must be in path.
#    export PATH=/apps/gcc/sol8/bin:/apps/binutils/sol8/bin:$PATH
#    make TARGET=xsol8
SYSROOT=/apps/gcc/sol8/sysroot
CROSS_COMPILE=1
CC=sparc-sun-solaris2.8-gcc
LD=sparc-sun-solaris2.8-gcc
CDEF+=-DSUNOS -DBYTE_ORDER=4321 -DBIG_ENDIAN=4321
LIBS+=-lxnet -lsocket
# Disable security flags for cross-compilation
SECURITY_FLAGS=

else
ifeq ($(TARGET),sol8)

# Flags for Solaris 8 native compile (BIG_ENDIAN BYTE_ORDER)
#    make TARGET=sol8
CDEF+=-DSUNOS -DBYTE_ORDER=4321 -DBIG_ENDIAN=4321
LIBS+=-lxnet -lsocket
# Disable some security flags for Solaris 8
SECURITY_FLAGS=-Wall -Wextra -Wformat=2

else

# Flags for Linux 2.6 native compile
#    make
CDEF+=-DLINUX

endif
endif

CFLAGS=-c -g -O2 -fmessage-length=0 -fno-strict-aliasing \
       $(SECURITY_FLAGS) $(WARN_FLAGS) $(CDEF) $(CDIR) $(CFLAGS_COVERAGE)
LDFLAGS=$(LDFLAGS_COVERAGE)

S5066D_OBJ=$(BUILD_DIR)/s5066d.o $(BUILD_DIR)/hiios.o $(BUILD_DIR)/hiwrite.o \
           $(BUILD_DIR)/hiread.o $(BUILD_DIR)/util.o $(BUILD_DIR)/license.o \
           $(BUILD_DIR)/sis.o $(BUILD_DIR)/dts.o $(BUILD_DIR)/smtp.o \
           $(BUILD_DIR)/http.o $(BUILD_DIR)/testping.o $(BUILD_DIR)/serial_sync.o \
           $(BUILD_DIR)/globalcounter.o

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Pattern rule for object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

s5066d: $(S5066D_OBJ)
	$(LD) $(LDFLAGS) -o s5066d $(S5066D_OBJ) $(LIBS)

synccat: $(BUILD_DIR)/serial_sync.o $(BUILD_DIR)/globalcounter.o $(BUILD_DIR)/synccat.o
	$(LD) $(LDFLAGS) -o synccat $^ $(LIBS)

iocat: $(BUILD_DIR)/serial_sync.o $(BUILD_DIR)/globalcounter.o $(BUILD_DIR)/iocat.o $(BUILD_DIR)/license.o
	$(LD) $(LDFLAGS) -o iocat $^ $(LIBS)

sizeof:
	$(CC) -o sizeof $(TOOLS_DIR)/sizeof.c

$(BUILD_DIR)/license.o: license.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

license.c: COPYING_sis5066_h
	printf 'char* license = "' >license.c
	printf 'Copyright (c) 2006 Sampo Kellomaki (sampo@iki.fi), All Rights Reserved.\\n' >>license.c
	sed -e 's/$$/\\n\\/' COPYING >>license.c
	sed -e 's/$$/\\n\\/' COPYING_sis5066_h >>license.c
	echo '";' >>license.c

all: s5066d synccat iocat sizeof

.PHONY: clean cleaner dist help test coverage install

cleaner: clean
	rm -rf dep

clean:
	rm -rf $(BUILD_DIR)/*.o s5066d synccat iocat sizeof *~ .*~ .\#* license.c
	rm -rf $(COVERAGE_DIR)/* *.gcda *.gcno *.gcov

dist: cleaner
	rm -rf open5066-$(REL)
	mkdir -p open5066-$(REL)
	cp -r src Makefile COPYING* scripts docs open5066-$(REL)/
	tar czf open5066-$(REL).tgz open5066-$(REL)
	rm -rf open5066-$(REL)

help:
	@echo "Open5066 Build System"
	@echo "===================="
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build for Linux (default)"
	@echo "  make TARGET=sol8  - Build for Solaris 8"
	@echo "  make TARGET=xsol8 - Cross-compile for Sparc Solaris 8"
	@echo "  make all          - Build all binaries"
	@echo "  make test         - Run tests"
	@echo "  make coverage     - Generate code coverage report"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make install      - Install binaries (requires sudo)"
	@echo ""
	@echo "Coverage:"
	@echo "  make COVERAGE=1   - Build with coverage instrumentation"
	@echo ""
	@echo "Security:"
	@echo "  Security flags enabled by default (stack protection, FORTIFY_SOURCE, PIE, RELRO)"

test:
	@echo "Running tests..."
	@if [ -d tests ]; then \
		cd tests && $(MAKE) test; \
	else \
		echo "Test directory not found. Creating placeholder..."; \
		mkdir -p tests; \
		echo "No tests yet. Please add tests to tests/ directory."; \
	fi

coverage: clean
	@echo "Generating code coverage report..."
	@mkdir -p $(COVERAGE_DIR)
	$(MAKE) COVERAGE=1 all
	$(MAKE) COVERAGE=1 test
	@echo "Collecting coverage data..."
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory $(BUILD_DIR) --output-file $(COVERAGE_DIR)/coverage.info; \
		lcov --remove $(COVERAGE_DIR)/coverage.info '/usr/*' --output-file $(COVERAGE_DIR)/coverage.info; \
		lcov --list $(COVERAGE_DIR)/coverage.info; \
		if command -v genhtml >/dev/null 2>&1; then \
			genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html; \
			echo "Coverage report generated in $(COVERAGE_DIR)/html/index.html"; \
		fi \
	else \
		gcov -o $(BUILD_DIR) $(CORE_DIR)/*.c $(PROTO_DIR)/*.c; \
		mv *.gcov $(COVERAGE_DIR)/ 2>/dev/null || true; \
		echo "Coverage data generated. Install lcov for HTML reports."; \
	fi

install: s5066d synccat iocat
	@echo "Installing binaries to /usr/sbin..."
	install -m 755 s5066d /usr/sbin/
	install -m 755 synccat /usr/sbin/
	install -m 755 iocat /usr/sbin/
	@echo "Installation complete."

dep: license.c | $(BUILD_DIR)
	$(CC) $(CDEF) $(CDIR) -MM $(patsubst $(BUILD_DIR)/%.o,%.c,$(S5066D_OBJ)) > deps

-include deps

#EOF
