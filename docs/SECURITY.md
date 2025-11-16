# Security Policy

## Supported Versions

We release patches for security vulnerabilities for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 0.5.x   | :white_check_mark: |
| < 0.5   | :x:                |

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please report security vulnerabilities by email to the project maintainers. You should receive a response within 48 hours. If for some reason you do not, please follow up via email to ensure we received your original message.

Please include the following information in your report:

* Type of issue (e.g. buffer overflow, SQL injection, cross-site scripting, etc.)
* Full paths of source file(s) related to the manifestation of the issue
* The location of the affected source code (tag/branch/commit or direct URL)
* Any special configuration required to reproduce the issue
* Step-by-step instructions to reproduce the issue
* Proof-of-concept or exploit code (if possible)
* Impact of the issue, including how an attacker might exploit the issue

## Security Considerations

This software implements the NATO STANAG 5066 protocol stack for HF radio communications and is designed for military and critical infrastructure use. Security considerations include:

### Network Security
- The daemon runs as a network service and processes data from potentially untrusted sources
- Input validation is critical for all network protocols (SIS, DTS, SMTP)
- Buffer overflow protection is enabled via compiler flags

### Known Security Features
- Stack protection (`-fstack-protector-strong`)
- Format string protection (`-Wformat-security`)
- Position Independent Executable (PIE)
- Full RELRO (Read-Only Relocations)
- Source fortification (`-D_FORTIFY_SOURCE=2`)

### Areas Under Active Security Hardening
- [ ] Replacement of unsafe string functions (strcpy, sprintf, etc.)
- [ ] Input validation for all protocol parsers
- [ ] Fuzzing of network protocol parsers
- [ ] Static analysis with multiple tools
- [ ] Memory sanitizer testing
- [ ] Penetration testing

### Cryptography
The planned crypto module (currently 0% complete) will use established cryptographic libraries. We will **never** implement custom cryptography.

## Security Testing

We encourage security researchers to:
- Perform security testing on the codebase
- Report vulnerabilities responsibly
- Contribute security improvements via pull requests

### Bug Bounty
We do not currently have a bug bounty program.

## Security Updates

Security updates will be released as soon as possible after a vulnerability is confirmed and patched. Updates will be announced via:
- GitHub Security Advisories
- Release notes
- Git tags

## Acknowledgments

We thank the following security researchers for their responsible disclosure:
- (None yet)
