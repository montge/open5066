#!/bin/bash
# Script to create GitHub issues for tracking repository improvements
# Usage: GITHUB_TOKEN=your_token ./create_issues.sh

set -e

REPO_OWNER="montge"
REPO_NAME="open5066"
API_URL="https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/issues"

# Check if GITHUB_TOKEN is set
if [ -z "$GITHUB_TOKEN" ]; then
    echo "Error: GITHUB_TOKEN environment variable is not set"
    echo "Usage: GITHUB_TOKEN=your_token ./create_issues.sh"
    exit 1
fi

# Function to create an issue
create_issue() {
    local title="$1"
    local body="$2"
    local labels="$3"

    echo "Creating issue: $title"

    curl -X POST \
        -H "Authorization: token $GITHUB_TOKEN" \
        -H "Accept: application/vnd.github.v3+json" \
        "$API_URL" \
        -d @- <<EOF
{
  "title": "$title",
  "body": "$body",
  "labels": [$labels]
}
EOF

    echo ""
}

# Priority 1: Critical Security Issues
create_issue \
    "[Security] Replace unsafe string functions (strcpy, sprintf, etc.)" \
    "## Description
Found 31 instances of unsafe string functions across the codebase that need to be replaced with safe alternatives.

## Affected Files
- \`http.c:27\` - sprintf() without bounds checking
- \`s5066d.c\` - Multiple sscanf() calls (lines 148, 323, 325, 327, 329, 422)
- \`iocat.c\` - 18 instances
- \`errmac.h\`
- \`hiwrite.c\`

## Required Changes
| Unsafe | Safe Alternative |
|--------|------------------|
| strcpy() | strncpy() or strlcpy() |
| strcat() | strncat() or strlcat() |
| sprintf() | snprintf() |
| sscanf() | Add return value checking |
| gets() | fgets() |

## Acceptance Criteria
- [ ] All strcpy replaced with strncpy/strlcpy
- [ ] All strcat replaced with strncat/strlcat
- [ ] All sprintf replaced with snprintf
- [ ] All sscanf calls check return values
- [ ] No gets() calls remain
- [ ] All changes tested
- [ ] Buffer overflow tests added

## Priority
🔴 **Critical** - Security vulnerability" \
    '"security", "bug", "priority: high"'

create_issue \
    "[Testing] Implement comprehensive test suite" \
    "## Description
Currently the repository has 0% test coverage. Need to implement a comprehensive test suite.

## Requirements

### Test Infrastructure
- [x] Create \`tests/\` directory structure
- [ ] Set up Unity test framework (or similar)
- [ ] Create test Makefile
- [ ] Add test runner script

### Unit Tests (tests/unit/)
- [ ] \`test_hiios.c\` - I/O engine tests
- [ ] \`test_sis.c\` - SIS protocol tests
- [ ] \`test_dts.c\` - DTS protocol tests
- [ ] \`test_smtp.c\` - SMTP tests
- [ ] \`test_util.c\` - Utility function tests

### Integration Tests (tests/integration/)
- [ ] \`test_e2e_smtp.c\` - End-to-end SMTP over SIS/DTS
- [ ] \`test_nonarq.c\` - Non-ARQ transfers
- [ ] \`test_arq.c\` - ARQ transfers

### Security Tests (tests/security/)
- [ ] \`test_buffer_overflow.c\` - Buffer overflow tests
- [ ] \`test_injection.c\` - Injection attack tests
- [ ] Fuzzing tests

### Performance Tests (tests/performance/)
- [ ] \`test_throughput.c\` - Throughput benchmarks
- [ ] \`test_latency.c\` - Latency benchmarks

## Target
- Achieve >70% code coverage
- All critical paths tested
- Security tests for all network inputs

## Priority
🔴 **Critical**" \
    '"testing", "enhancement", "priority: high"'

create_issue \
    "[CI/CD] Verify and enhance GitHub Actions workflows" \
    "## Description
GitHub Actions workflows have been created but need verification and enhancement.

## Workflows Created
- [x] \`build.yml\` - Build automation
- [x] \`test.yml\` - Test automation
- [x] \`security.yml\` - Security scans
- [x] \`coverage.yml\` - Coverage reports

## Tasks
- [ ] Verify all workflows run successfully
- [ ] Fix any workflow errors
- [ ] Add sanitizer support (AddressSanitizer, ThreadSanitizer, UndefinedBehaviorSanitizer)
- [ ] Configure Codecov integration
- [ ] Add status badges to README
- [ ] Set up branch protection rules
- [ ] Configure required status checks

## Additional Enhancements
- [ ] Add release automation
- [ ] Add documentation generation workflow
- [ ] Add Docker build workflow (future)
- [ ] Add performance benchmark tracking

## Priority
🟡 **High**" \
    '"ci/cd", "infrastructure", "priority: medium"'

create_issue \
    "[Documentation] Create API documentation" \
    "## Description
Need to create comprehensive API documentation for the codebase.

## Requirements
- [ ] Set up Doxygen
- [ ] Add Doxygen comments to all public functions
- [ ] Add Doxygen comments to all public structs
- [ ] Generate HTML API documentation
- [ ] Create \`docs/API.md\` overview
- [ ] Create \`docs/ARCHITECTURE.md\` with system diagrams
- [ ] Add code examples to documentation

## Sections Needed
- Core I/O Engine API
- SIS Protocol API
- DTS Protocol API
- SMTP Protocol API
- Utility Functions API

## Priority
🟢 **Medium**" \
    '"documentation", "enhancement", "priority: medium"'

create_issue \
    "[Refactoring] Complete ARQ transfer implementation" \
    "## Description
ARQ transfers are currently only 10% complete. This is a critical feature for reliable data transfer.

## Current Status
- Non-ARQ transfers: 95% complete ✅
- ARQ transfers: 10% complete ⚠️

## Requirements
- [ ] Implement ARQ state machine
- [ ] Add retransmission logic
- [ ] Implement acknowledgment handling
- [ ] Add timeout and retry mechanisms
- [ ] Implement flow control
- [ ] Add comprehensive tests

## Reference
See STANAG 5066 Annex C specification for ARQ requirements

## Priority
🟡 **High**" \
    '"enhancement", "feature", "priority: high"'

create_issue \
    "[Security] Implement input validation for all network protocols" \
    "## Description
All network-facing protocols need comprehensive input validation to prevent attacks.

## Protocols to Validate
- [ ] SIS protocol parser (\`sis.c\`)
- [ ] DTS protocol parser (\`dts.c\`)
- [ ] SMTP protocol parser (\`smtp.c\`)
- [ ] HTTP protocol parser (\`http.c\`)

## Validation Required
- [ ] Length checks for all fields
- [ ] Type validation
- [ ] Range checks
- [ ] Null termination verification
- [ ] Format validation

## Security Considerations
- Buffer overflow prevention
- Integer overflow prevention
- Format string attack prevention
- Injection attack prevention

## Testing
- [ ] Add fuzzing tests
- [ ] Add malformed input tests
- [ ] Add boundary condition tests

## Priority
🔴 **Critical** - Security issue" \
    '"security", "enhancement", "priority: high"'

create_issue \
    "[Infrastructure] Set up static analysis tools" \
    "## Description
Configure and integrate static analysis tools into the build and CI pipeline.

## Tools to Configure
- [x] cppcheck (added to CI)
- [x] clang-tidy (added to CI)
- [x] CodeQL (added to CI)
- [ ] scan-build (Clang Static Analyzer)
- [ ] Coverity Scan
- [ ] SonarQube/SonarCloud

## Integration Points
- [ ] Add to Makefile
- [ ] Run in pre-commit hooks
- [ ] Block PRs on critical issues
- [ ] Generate reports

## Configuration
- [ ] Create .clang-tidy configuration
- [ ] Configure warning levels
- [ ] Set up suppression rules for false positives

## Priority
🟡 **High**" \
    '"infrastructure", "quality", "priority: medium"'

create_issue \
    "[Feature] Implement crypto module" \
    "## Description
Crypto module is currently 0% complete. This is essential for secure communications.

## Requirements
- [ ] Choose cryptographic library (OpenSSL, libsodium, etc.)
- [ ] Implement AES256 cipher support
- [ ] Implement key management
- [ ] Add encryption/decryption for DTS traffic
- [ ] Implement secure key exchange
- [ ] Add HMAC for message authentication

## Security Considerations
- ⚠️ **Never implement custom cryptography**
- Use well-established libraries
- Follow cryptographic best practices
- Implement perfect forward secrecy if possible

## Testing
- [ ] Unit tests for crypto functions
- [ ] Integration tests with DTS
- [ ] Security audit

## Priority
🟢 **Medium** - Important but not urgent" \
    '"feature", "enhancement", "security", "priority: medium"'

create_issue \
    "[Quality] Add compiler sanitizer support" \
    "## Description
Add support for compiler sanitizers to catch memory errors, undefined behavior, and race conditions.

## Sanitizers to Add
- [ ] AddressSanitizer (ASan) - Memory errors
- [ ] ThreadSanitizer (TSan) - Race conditions
- [ ] UndefinedBehaviorSanitizer (UBSan) - Undefined behavior
- [ ] LeakSanitizer (LSan) - Memory leaks

## Implementation
- [ ] Add sanitizer flags to Makefile
- [ ] Create separate build targets for each sanitizer
- [ ] Integrate into CI/CD pipeline
- [ ] Document usage in README

## Usage Example
\`\`\`bash
make SANITIZER=address    # Build with AddressSanitizer
make SANITIZER=thread     # Build with ThreadSanitizer
make SANITIZER=undefined  # Build with UBSan
\`\`\`

## Priority
🟡 **High** - Prevents bugs" \
    '"quality", "infrastructure", "priority: medium"'

create_issue \
    "[Documentation] Convert README to proper Markdown and improve structure" \
    "## Description
The original README is plain text. Need to create better structured documentation.

## Tasks
- [x] Create \`README.md\` with badges and quick start
- [ ] Create \`docs/ARCHITECTURE.md\` with system architecture
- [ ] Create \`docs/PROTOCOL.md\` with protocol details
- [ ] Create \`docs/DEPLOYMENT.md\` with deployment guide
- [ ] Add architecture diagrams (use mermaid or graphviz)
- [ ] Add sequence diagrams for protocol flows

## Structure
\`\`\`
docs/
├── README.md          # Main documentation
├── ARCHITECTURE.md    # System architecture
├── PROTOCOL.md        # Protocol details
├── DEPLOYMENT.md      # Deployment guide
├── API.md            # API reference
├── CONTRIBUTING.md   # Contribution guide (✓)
├── SECURITY.md       # Security policy (✓)
└── manual/
    └── open5066.pdf  # Original manual
\`\`\`

## Priority
🟢 **Medium**" \
    '"documentation", "enhancement", "priority: low"'

create_issue \
    "[Monitoring] Add observability and metrics" \
    "## Description
Add observability features for monitoring the daemon in production.

## Features to Add
- [ ] Structured logging (JSON format option)
- [ ] Metrics collection (Prometheus-compatible)
- [ ] Health check endpoint
- [ ] Status endpoint with runtime statistics
- [ ] Performance counters
- [ ] Connection tracking

## Metrics to Track
- Active connections
- Bytes sent/received
- Protocol errors
- ARQ retransmissions
- Queue depths
- Latency measurements

## Priority
🟢 **Low** - Nice to have" \
    '"feature", "enhancement", "priority: low"'

echo "======================================"
echo "All issues created successfully!"
echo "======================================"
echo ""
echo "View issues at: https://github.com/${REPO_OWNER}/${REPO_NAME}/issues"
