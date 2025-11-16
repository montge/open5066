# Open5066 - NATO STANAG 5066 Protocol Stack

[![Build](https://github.com/montge/open5066/workflows/Build/badge.svg)](https://github.com/montge/open5066/actions)
[![Security](https://github.com/montge/open5066/workflows/Security%20Scans/badge.svg)](https://github.com/montge/open5066/actions)
[![Tests](https://github.com/montge/open5066/workflows/Tests/badge.svg)](https://github.com/montge/open5066/actions)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

> **⚠️ Early Development Warning**
> This project is in very early development stage (v0.5). Unless you have C debugging skills and are willing to contribute, you probably should not be using this software in production yet.

## Overview

Open5066 is an open source implementation of NATO NC3A STANAG 5066 protocol stack for HF radio communications. It implements:

- **SIS (Annex A)** - Subnetwork Interface Sublayer (75% complete)
- **DTS (Annex C)** - Data Transfer Sublayer over TCP (60% complete)
- **Application Layer (Annex F)** - HMTP and other protocols (partial)

The concrete embodiment is the `s5066d` daemon.

## Quick Start

### Installation

**Using CMake (Recommended):**
```bash
git clone https://github.com/montge/open5066.git
cd open5066
mkdir build && cd build
cmake ..
make
sudo make install
```

**Using legacy Makefile (Deprecated):**
```bash
make  # Old build system - see deprecation notice in Makefile
```

### Basic Usage

**Node 1:**
```bash
s5066d -p sis::5066 -p dts::5067 dts:node2:5067
```

**Node 2:**
```bash
s5066d -p sis::5066 -p dts::5067 dts:node1:5067
```

See [docs/README](docs/README) for comprehensive documentation.

## Features

| Component | Status | Completion |
|-----------|--------|------------|
| I/O Engine (epoll/poll) | ✅ Active | 80% |
| SIS Primitives | ✅ Active | 75% |
| DTS over TCP | ✅ Active | 60% |
| Non-ARQ Transfers | ✅ Working | 95% |
| ARQ Transfers | 🔧 In Progress | 10% |
| Routing | ❌ Planned | 0% |
| Crypto Module | ❌ Planned | 0% |
| Soft-modem | ❌ Planned | 0% |

## Project Structure

```
open5066/
├── src/
│   ├── core/        # Core daemon and I/O engine
│   ├── protocols/   # Protocol implementations (SIS, DTS, SMTP)
│   ├── serial/      # Serial communication
│   ├── include/     # Public headers
│   └── tools/       # Utility programs
├── tests/           # Test suite
├── docs/            # Documentation
└── scripts/         # Build and test scripts
```

## Building

### Using CMake (Primary Build System)

**Standard Build:**
```bash
mkdir build && cd build
cmake ..
make                    # Build all targets
ctest                   # Run all tests
```

**Build Types:**
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..     # Optimized build with LTO
cmake -DCMAKE_BUILD_TYPE=Debug ..       # Debug symbols
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. # Optimized + debug symbols
```

**With Coverage:**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
ctest
make coverage
```

**With Sanitizers:**
```bash
cmake -DENABLE_SANITIZERS=ON ..
make
ctest
```

### Using Legacy Makefile (Deprecated)

**⚠️ The Makefile build system is deprecated. Use CMake instead.**

```bash
make              # Build for Linux (deprecated)
make test         # Run tests (deprecated)
make coverage     # Generate coverage report (deprecated)
```

See docs/CMAKE_MIGRATION.md for migration guide.

## Development

We welcome contributions! Please see:
- [CONTRIBUTING.md](docs/CONTRIBUTING.md) - Contribution guidelines
- [SECURITY.md](docs/SECURITY.md) - Security policy
- [CHANGELOG.md](CHANGELOG.md) - Version history

### Code Quality

- **Security**: All builds include stack protection, PIE, RELRO, and FORTIFY_SOURCE
- **Testing**: Unit, integration, and security tests
- **CI/CD**: Automated builds, tests, and security scans via GitHub Actions
- **Coverage**: Code coverage tracking with lcov

### Running Tests

**With CMake/CTest (Recommended):**
```bash
cd build
ctest                         # Run all tests
ctest -V                      # Verbose output
ctest -L unit                 # Run only unit tests
ctest -L integration          # Run only integration tests
ctest --rerun-failed          # Rerun failed tests
```

**With legacy Makefile (Deprecated):**
```bash
make test              # Run all tests (deprecated)
make coverage          # Generate coverage report (deprecated)
```

## Documentation

- [Full README](docs/README) - Comprehensive documentation
- [PDF Manual](docs/manual/open5066.pdf) - Technical manual
- [API Documentation](docs/API.md) - API reference (WIP)
- [Architecture](docs/ARCHITECTURE.md) - System architecture (WIP)

## Platform Support

- **Linux** 2.6+ (primary platform, uses epoll)
- **Solaris** 8+ (uses /dev/poll)
- Other UNIX systems (requires porting effort)

## Dependencies

- GCC or compatible C compiler
- GNU Make 3.78+
- pthread library
- Optional: lcov (for coverage reports)

## Security

This software is designed for military and critical infrastructure use. We take security seriously:

- ✅ Compiler security hardening enabled by default
- ✅ Static analysis via cppcheck and clang-tidy
- ✅ CodeQL security scanning
- ⚠️ Active work on replacing unsafe string functions
- ⚠️ Input validation improvements ongoing

**Report security vulnerabilities**: See [SECURITY.md](docs/SECURITY.md)

## License

- **Main code**: GNU General Public License v2 (GPL-2)
- **NATO C3 Agency components**: BSD-style license

### Using Open5066 in Commercial/Proprietary Software

Open5066 can be legally integrated with proprietary software using several approaches:

1. **Network Service Architecture** (recommended) - Run s5066d daemon, connect proprietary applications via TCP/IP
2. **Internal Use Only** - Deploy internally without distribution
3. **Commercial Support Model** - Offer services and support around GPL software
4. **Separate Process Communication** - Use IPC mechanisms for clean separation

**For detailed guidance**, see [GPL Compliance Guide](docs/GPL_COMPLIANCE.md)

**License files**: [COPYING](COPYING) (GPL-2) and [COPYING_sis5066_h](COPYING_sis5066_h) (BSD)

## Credits

- **Original Author**: Sampo Kellomäki (sampo@iki.fi)
- **This Fork**: Enhanced with modern development practices

## Status

**Version**: 0.5 (Released: May 2006)
**Development Stage**: Early Alpha
**Philosophy**: "Release early, release often"

## Links

- [GitHub Repository](https://github.com/montge/open5066)
- [Issues](https://github.com/montge/open5066/issues)
- [CI/CD Pipeline](https://github.com/montge/open5066/actions)

---

**Last Updated**: November 2025
**Maintained**: Yes, active development ongoing
