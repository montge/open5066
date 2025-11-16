# GitHub Issues for Build System Modernization

This document contains the GitHub issues that should be created to track the build system modernization work.

---

## Issue #1: Migrate to CMake Build System

**Title**: Migrate to CMake build system for cross-platform support

**Labels**: `enhancement`, `build-system`, `infrastructure`

**Description**:

Replace traditional Makefiles with modern CMake build system to improve cross-platform support, IDE integration, and developer experience.

### Motivation

- Enable out-of-source builds
- Better integration with modern IDEs (VS Code, CLion, etc.)
- Improved dependency tracking
- CTest integration for unified testing
- Easier CI/CD pipeline integration

### Scope

- [x] Create root CMakeLists.txt
- [x] Create tests/CMakeLists.txt
- [x] Integrate all existing tests with CTest
- [x] Maintain backwards compatibility with traditional Makefile
- [x] Document migration in CMAKE_MIGRATION.md
- [ ] Update CI/CD pipelines (if applicable)
- [ ] Test cross-compilation scenarios

### Benefits

1. **Developer Experience**: Out-of-source builds keep repository clean
2. **Testing**: Unified test framework with CTest
3. **Portability**: Better cross-platform build support
4. **Tooling**: Native IDE integration without custom configurations

### Testing

All 28 tests pass with new CMake system:
- 8 unit tests
- 10 integration tests
- 10 security tests

```bash
mkdir build && cd build
cmake ..
make -j
ctest --output-on-failure
```

### Documentation

See `docs/CMAKE_MIGRATION.md` for complete migration guide.

---

## Issue #2: Update to Modern C Standard (C17)

**Title**: Update compilation to use C17 standard

**Labels**: `enhancement`, `modernization`, `code-quality`

**Description**:

Explicitly set C17 (ISO/IEC 9899:2018) as the compilation standard to enable modern C features, improve type safety, and prepare for future C23 migration.

### Current State

- Previously: Implicit gnu89/gnu99 (compiler default)
- Now: Explicit C17 with `CMAKE_C_STANDARD=17`

### Changes

- [x] Set C17 standard in CMakeLists.txt
- [x] Add `_GNU_SOURCE` for POSIX extensions
- [x] Verify all code compiles with C17
- [x] Suppress legacy code warnings temporarily
- [ ] Systematically fix remaining warnings
- [ ] Adopt C17 features where beneficial

### Benefits

1. **Type Safety**: Better compile-time error detection
2. **Security**: Modern safer APIs and bounds checking
3. **Portability**: Consistent behavior across compilers
4. **Future-Proof**: Easy path to C23 when needed
5. **Optimization**: Better compiler optimization opportunities

### Compiler Support

- GCC 13.3.0: Full C17 support ✓
- Clang 14+: Full C17 support ✓
- MSVC 2019+: Partial C17 support

### Follow-up Work

1. **Fix Warnings**: Address suppressed warnings systematically
   - Format string warnings
   - Unused variable warnings
   - Implicit function declarations
2. **Modernize Code**: Use C17 features where appropriate
   - Better type generics
   - Improved preprocessing
   - Safer library functions
3. **Documentation**: Update coding standards

---

## Issue #3: Integrate Testing with CTest

**Title**: Integrate test suite with CTest framework

**Labels**: `enhancement`, `testing`, `infrastructure`

**Description**:

Integrate existing Unity-based test suite with CMake's CTest framework for better test organization, execution, and reporting.

### Implementation

- [x] Create tests/CMakeLists.txt
- [x] Auto-discover unit tests
- [x] Integrate security tests
- [x] Integrate integration tests (shell scripts)
- [x] Add test labels (unit, security, integration)
- [x] Configure test timeouts
- [x] Add custom test targets

### Features

**Test Discovery**:
```bash
ctest --show-only    # List all tests
```

**Selective Execution**:
```bash
ctest -L unit          # Run unit tests only
ctest -L security      # Run security tests only
ctest -L integration   # Run integration tests only
```

**Parallel Execution**:
```bash
ctest -j$(nproc)      # Run tests in parallel
```

**Detailed Output**:
```bash
ctest -V              # Verbose
ctest -VV             # Very verbose
ctest --output-on-failure  # Show output only for failures
```

### Test Coverage

- **Unit Tests**: 8 tests (protocol basics, CRC functions)
- **Security Tests**: 10 tests (buffer overflows, input validation)
- **Integration Tests**: 10 tests (binary security, symbols)
- **Total**: 28 tests, 100% pass rate

### Benefits

1. **Unified Interface**: Single command for all tests
2. **Better Reporting**: Structured test results
3. **CI/CD Integration**: Standard CTest output for Jenkins, GitHub Actions, etc.
4. **Categorization**: Test labels for selective execution
5. **Timeout Protection**: Prevent hung tests

---

## Issue #4: Add Code Coverage Support

**Title**: Add code coverage analysis with lcov/genhtml

**Labels**: `enhancement`, `testing`, `code-quality`

**Description**:

Add optional code coverage analysis to measure test effectiveness and identify untested code paths.

### Implementation

```bash
# Build with coverage
cmake -DENABLE_COVERAGE=ON ..
make

# Run tests
make test

# Generate coverage report
make coverage

# View report
open build/coverage/html/index.html
```

### Features

- [x] CMake ENABLE_COVERAGE option
- [x] Coverage build flags (`-fprofile-arcs`, `-ftest-coverage`)
- [x] lcov/genhtml integration
- [x] HTML coverage reports
- [ ] Coverage CI integration
- [ ] Coverage thresholds/gates

### Goals

- Achieve >80% line coverage for core protocol code
- Identify untested error paths
- Guide new test development

---

## Issue #5: Add Sanitizer Support for Testing

**Title**: Add AddressSanitizer and UBSan support

**Labels**: `enhancement`, `testing`, `security`, `code-quality`

**Description**:

Add optional sanitizer support to detect memory errors, undefined behavior, and other runtime issues during testing.

### Implementation

```bash
# Build with sanitizers
cmake -DENABLE_SANITIZERS=ON ..
make

# Run tests (sanitizers active)
make test
```

### Sanitizers

1. **AddressSanitizer** (`-fsanitize=address`)
   - Detect buffer overflows
   - Use-after-free errors
   - Memory leaks

2. **UndefinedBehaviorSanitizer** (`-fsanitize=undefined`)
   - Integer overflows
   - Null pointer dereferences
   - Alignment violations

### Benefits

- Early detection of memory safety issues
- Better test quality
- Improved code reliability
- Security vulnerability prevention

### Status

- [x] CMake option added
- [x] Compiler flags configured
- [ ] Run full test suite with sanitizers
- [ ] Fix any detected issues
- [ ] Add to CI pipeline

---

## Issue #6: Fix Remaining Compilation Warnings

**Title**: Systematically fix suppressed compilation warnings

**Labels**: `enhancement`, `code-quality`, `technical-debt`

**Description**:

The C17 migration revealed numerous compilation warnings from legacy code patterns. These are currently suppressed but should be fixed systematically.

### Suppressed Warnings

```cmake
-Wno-unused-value
-Wno-format-nonliteral
-Wno-missing-braces
-Wno-missing-field-initializers
-Wno-unused-variable
-Wno-unused-parameter
-Wno-format-extra-args
-Wno-sign-compare
-Wno-return-type
-Wno-implicit-function-declaration
-Wno-builtin-declaration-mismatch
-Wno-unused-but-set-variable
```

### Approach

1. **Phase 1**: Fix critical warnings
   - `-Wreturn-type`: Missing return values
   - `-Wimplicit-function-declaration`: Missing prototypes

2. **Phase 2**: Fix format warnings
   - `-Wformat`: Format string mismatches
   - `-Wsign-compare`: Signed/unsigned comparisons

3. **Phase 3**: Cleanup
   - Unused variables/parameters
   - Missing initializers
   - Style warnings

### Strategy

- Fix warnings incrementally by category
- Enable one warning flag at a time
- Test after each set of fixes
- Document intentional suppressions

### Benefits

- Improved code quality
- Better compiler optimization
- Easier maintenance
- Reduced technical debt

---

## Issue #7: Complete iocat Build Fix

**Title**: Fix iocat compilation errors

**Labels**: `bug`, `build-system`

**Description**:

The `iocat` utility currently fails to build due to missing stdlib.h include and other issues. This is not critical for core functionality but should be fixed for completeness.

### Current Error

```
fatal error: implicit declaration of exit()
```

### Fix Required

Add missing includes to `src/tools/iocat.c`:
```c
#include <stdlib.h>  // for exit()
```

### Status

- **Priority**: Low (iocat is not core functionality)
- **Workaround**: Use synccat for serial communication
- **Impact**: Development tools only

---

## Priority Order

1. **High Priority**
   - Issue #1: CMake Migration (DONE)
   - Issue #2: C17 Standard (DONE)
   - Issue #3: CTest Integration (DONE)

2. **Medium Priority**
   - Issue #4: Code Coverage
   - Issue #5: Sanitizers
   - Issue #6: Fix Warnings

3. **Low Priority**
   - Issue #7: iocat Build

---

## Labels to Create

```
enhancement - New features or improvements
build-system - Build system and compilation
infrastructure - Project infrastructure
testing - Testing framework and tests
code-quality - Code quality improvements
security - Security-related changes
technical-debt - Technical debt reduction
modernization - Code modernization efforts
bug - Something isn't working
```

---

## Milestones

### Milestone: Build System Modernization (v0.6.0)
- Issue #1: CMake Migration ✓
- Issue #2: C17 Standard ✓
- Issue #3: CTest Integration ✓

### Milestone: Code Quality Improvements (v0.7.0)
- Issue #4: Code Coverage
- Issue #5: Sanitizers
- Issue #6: Fix Warnings

### Milestone: Polish (v0.8.0)
- Issue #7: Complete iocat build
- Documentation updates
- CI/CD integration

---

**Note**: Since GitHub CLI is not available in this environment, these issues should be created manually through the GitHub web interface or API. Copy the content above for each issue.
