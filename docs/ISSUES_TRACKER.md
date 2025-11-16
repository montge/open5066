# Issues Tracker - Repository Improvements

This document tracks all the improvement tasks identified during the initial repository assessment.

## How to Create Issues

We've provided a script to create all these issues automatically on GitHub:

```bash
# Set your GitHub token
export GITHUB_TOKEN=your_github_personal_access_token

# Run the script
./scripts/create_issues.sh
```

Alternatively, you can manually create these issues on GitHub.

---

## Priority 1: Critical (Security & Testing)

### 1. Replace Unsafe String Functions
**Priority**: 🔴 Critical
**Labels**: security, bug, priority: high

**Description**: Found 31 instances of unsafe string functions (strcpy, sprintf, etc.) that need to be replaced with safe alternatives.

**Affected Files**:
- `http.c:27` - sprintf() without bounds checking
- `s5066d.c` - Multiple sscanf() calls
- `iocat.c` - 18 instances
- `errmac.h`, `hiwrite.c`

**Replacements**:
- strcpy() → strncpy() or strlcpy()
- strcat() → strncat() or strlcat()
- sprintf() → snprintf()
- gets() → fgets()

---

### 2. Implement Comprehensive Test Suite
**Priority**: 🔴 Critical
**Labels**: testing, enhancement, priority: high

**Current Coverage**: 0%
**Target Coverage**: >70%

**Test Structure**:
```
tests/
├── unit/           # Unit tests
├── integration/    # Integration tests
├── security/       # Security tests
└── performance/    # Performance benchmarks
```

**Required Tests**:
- [ ] I/O engine tests
- [ ] Protocol parser tests (SIS, DTS, SMTP)
- [ ] Utility function tests
- [ ] End-to-end integration tests
- [ ] Buffer overflow tests
- [ ] Fuzzing tests

---

### 3. Implement Input Validation
**Priority**: 🔴 Critical
**Labels**: security, enhancement, priority: high

**Protocols to Validate**:
- [ ] SIS protocol parser
- [ ] DTS protocol parser
- [ ] SMTP protocol parser
- [ ] HTTP protocol parser

**Validation Requirements**:
- Length checks for all fields
- Type and range validation
- Null termination verification
- Format validation

---

## Priority 2: High (Infrastructure & Quality)

### 4. Verify and Enhance GitHub Actions Workflows
**Priority**: 🟡 High
**Labels**: ci/cd, infrastructure, priority: medium

**Tasks**:
- [ ] Verify all workflows run successfully
- [ ] Add sanitizer support (ASan, TSan, UBSan)
- [ ] Configure Codecov integration
- [ ] Set up branch protection rules
- [ ] Add status badges

---

### 5. Complete ARQ Transfer Implementation
**Priority**: 🟡 High
**Labels**: enhancement, feature, priority: high

**Current Status**: 10% complete
**Target**: 100% complete

**Requirements**:
- [ ] ARQ state machine
- [ ] Retransmission logic
- [ ] Acknowledgment handling
- [ ] Timeout and retry mechanisms
- [ ] Flow control

---

### 6. Set Up Static Analysis Tools
**Priority**: 🟡 High
**Labels**: infrastructure, quality, priority: medium

**Tools**:
- [x] cppcheck (in CI)
- [x] clang-tidy (in CI)
- [x] CodeQL (in CI)
- [ ] scan-build
- [ ] Coverity Scan
- [ ] SonarQube

---

### 7. Add Compiler Sanitizer Support
**Priority**: 🟡 High
**Labels**: quality, infrastructure, priority: medium

**Sanitizers to Add**:
- [ ] AddressSanitizer (memory errors)
- [ ] ThreadSanitizer (race conditions)
- [ ] UndefinedBehaviorSanitizer
- [ ] LeakSanitizer

**Usage**:
```bash
make SANITIZER=address
make SANITIZER=thread
make SANITIZER=undefined
```

---

## Priority 3: Medium (Features & Documentation)

### 8. Create API Documentation
**Priority**: 🟢 Medium
**Labels**: documentation, enhancement, priority: medium

**Requirements**:
- [ ] Set up Doxygen
- [ ] Add Doxygen comments to public APIs
- [ ] Generate HTML documentation
- [ ] Create docs/API.md
- [ ] Create docs/ARCHITECTURE.md

---

### 9. Implement Crypto Module
**Priority**: 🟢 Medium
**Labels**: feature, enhancement, security, priority: medium

**Current Status**: 0% complete

**Requirements**:
- [ ] Choose crypto library (OpenSSL/libsodium)
- [ ] Implement AES256 support
- [ ] Add key management
- [ ] Implement secure key exchange
- [ ] Add message authentication (HMAC)

**Important**: Never implement custom cryptography!

---

### 10. Improve Documentation Structure
**Priority**: 🟢 Medium
**Labels**: documentation, enhancement, priority: low

**Tasks**:
- [x] Create README.md
- [ ] Create docs/ARCHITECTURE.md
- [ ] Create docs/PROTOCOL.md
- [ ] Create docs/DEPLOYMENT.md
- [ ] Add architecture diagrams

---

## Priority 4: Low (Nice to Have)

### 11. Add Observability and Metrics
**Priority**: 🟢 Low
**Labels**: feature, enhancement, priority: low

**Features**:
- [ ] Structured logging (JSON format)
- [ ] Metrics collection (Prometheus-compatible)
- [ ] Health check endpoint
- [ ] Runtime statistics endpoint
- [ ] Performance counters

**Metrics to Track**:
- Active connections
- Bytes sent/received
- Protocol errors
- ARQ retransmissions
- Latency measurements

---

## Summary Statistics

| Priority | Count | Status |
|----------|-------|--------|
| 🔴 Critical | 3 | Security & Testing |
| 🟡 High | 4 | Infrastructure & Quality |
| 🟢 Medium | 3 | Features & Documentation |
| 🟢 Low | 1 | Nice to Have |
| **Total** | **11** | |

## Progress Tracking

### Completed ✅
- [x] Repository restructuring
- [x] Makefile improvements
- [x] Security compiler flags
- [x] Code coverage infrastructure
- [x] GitHub Actions workflows
- [x] SECURITY.md
- [x] CONTRIBUTING.md
- [x] CHANGELOG.md
- [x] Fixed dependabot.yml
- [x] README.md

### In Progress 🔧
- [ ] Test suite implementation
- [ ] Security hardening
- [ ] ARQ implementation

### Planned 📋
- [ ] API documentation
- [ ] Crypto module
- [ ] Observability features

---

## Quick Reference

### Security Improvements
1. Replace unsafe functions (**CRITICAL**)
2. Add input validation (**CRITICAL**)
3. Implement crypto module (Medium)

### Testing & Quality
1. Implement test suite (**CRITICAL**)
2. Add sanitizers (High)
3. Static analysis tools (High)

### Infrastructure
1. Enhance CI/CD (High)
2. API documentation (Medium)

### Features
1. Complete ARQ implementation (High)
2. Add observability (Low)

---

**Last Updated**: November 2025
**Created By**: Initial repository assessment
