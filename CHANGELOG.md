# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Reorganized repository structure with proper src/, tests/, docs/ directories
- Enhanced Makefile with security flags and code coverage support
- Code coverage infrastructure (gcov/lcov)
- Security hardening compiler flags (stack protection, PIE, RELRO, FORTIFY_SOURCE)
- SECURITY.md with security policy and vulnerability reporting process
- CONTRIBUTING.md with development guidelines
- CHANGELOG.md for tracking project changes
- GitHub Actions CI/CD workflows for automated testing
- Fixed Dependabot configuration for automated dependency updates
- Build directory for separating build artifacts from source
- Coverage reports with HTML output

### Changed
- Moved all source files to organized src/ subdirectories
- Updated Makefile to support new directory structure
- Enhanced .gitignore for build artifacts and coverage files
- Improved compiler warnings and security flags
- Updated README location to docs/README

### Security
- Enabled stack protector (-fstack-protector-strong)
- Enabled format security warnings (-Wformat-security)
- Enabled source fortification (-D_FORTIFY_SOURCE=2)
- Enabled Position Independent Executable (PIE)
- Enabled full RELRO (RELocation Read-Only)

## [0.5] - 2006-05-03

### Added
- Initial implementation of SIS (Subnetwork Interface Sublayer) - 75% complete
- Initial implementation of DTS (Data Transfer Sublayer) over TCP - 60% complete
- Non-ARQ transfers with repetitions - 95% complete
- SMTP protocol support for mail transfer
- HTTP/1.0 support for debugging
- Test ping protocol for I/O engine testing
- I/O engine with epoll (Linux) and /dev/poll (Solaris) support
- Multi-platform support (Linux 2.6+, Solaris 8)
- Cross-compilation support for Sparc Solaris 8
- Synchronous serial support
- Basic daemon infrastructure

### Known Limitations
- ARQ transfers only 10% complete
- No routing support
- No crypto module (0% complete)
- No soft-modem implementation
- No ALE support
- No rank/priority support
- No expedited transfer modes
- Many protocol features incomplete

## [0.4] - 2006-05-03

### Added
- First release capable of full HMTP over DTS to remote roundtrip
- Basic SIS and DTS protocol implementation
- SMTP relay functionality
- Command-line interface and daemon mode

---

## Version History

| Version | Date       | Status | Notes |
|---------|------------|--------|-------|
| 0.5     | 2006-05-03 | Released | Current release |
| 0.4     | 2006-05-03 | Released | First functional release |
| 0.1-0.3 | 2006-04    | Dev    | Early development |

## Project Status

**Development Stage:** Early Alpha

This is a work in progress. The project is in very early phase with major feature enhancements and debugging ongoing. Unless you have C debugging skills and are willing to contribute, you probably should not be using this software in production yet.

### Feature Completion Status

| Feature | Status | Completion |
|---------|--------|------------|
| I/O Engine | Active | 80% |
| SIS Primitives | Active | 75% |
| DTS over TCP | Active | 60% |
| Non-ARQ Transfers | Working | 95% |
| ARQ Transfers | In Progress | 10% |
| Routing | Not Started | 0% |
| Crypto Module | Not Started | 0% |
| Soft-modem | Not Started | 0% |
| ALE | Not Started | 0% |

---

For a detailed list of changes, see the [git commit history](https://github.com/montge/open5066/commits/).
