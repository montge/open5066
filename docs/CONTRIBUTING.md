# Contributing to Open5066

Thank you for your interest in contributing to Open5066! This document provides guidelines for contributing to the project.

## Code of Conduct

By participating in this project, you agree to maintain a respectful and collaborative environment.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates. When creating a bug report, include:

* **Use a clear and descriptive title**
* **Describe the exact steps to reproduce the problem**
* **Provide specific examples** (code snippets, configuration files)
* **Describe the behavior you observed** and what you expected
* **Include system information** (OS, compiler version, etc.)
* **Attach relevant logs** with `-d -d` debug output if applicable

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion:

* **Use a clear and descriptive title**
* **Provide a detailed description** of the suggested enhancement
* **Explain why this enhancement would be useful**
* **List some examples** of where this would apply

### Security Vulnerabilities

**Do not report security vulnerabilities as GitHub issues.** Please see [SECURITY.md](SECURITY.md) for responsible disclosure procedures.

## Development Process

### Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/open5066.git`
3. Create a feature branch: `git checkout -b feature/my-new-feature`
4. Make your changes
5. Test your changes: `make clean && make && make test`
6. Commit your changes: `git commit -am 'Add some feature'`
7. Push to the branch: `git push origin feature/my-new-feature`
8. Create a Pull Request

### Building the Project

```bash
# Linux build
make

# Solaris 8 build
make TARGET=sol8

# With code coverage
make COVERAGE=1

# Run tests
make test

# Generate coverage report
make coverage
```

### Code Style

* **Follow existing code style** - consistency is important
* **Use ANSI C** (C89/C90 compatible)
* **Comment your code** - especially complex algorithms
* **Keep functions small** - aim for single responsibility
* **Use meaningful variable names**

#### Coding Conventions

* **Indentation**: Tabs (width 8)
* **Braces**: K&R style (opening brace on same line)
* **Line length**: Try to keep under 80 characters when practical
* **Comments**: Use `/* */` style, not `//`
* **Macros**: ALL_CAPS with underscores
* **Functions**: lowercase_with_underscores
* **Structs**: lowercase_with_underscores

Example:
```c
/* Good example of coding style */
struct my_struct {
	int field1;
	char* field2;
};

int my_function(struct my_struct* ms, int arg)
{
	if (!ms) {
		ERR("Invalid argument");
		return -1;
	}

	ms->field1 = arg;
	return 0;
}
```

### Security Requirements

All contributions must:

* **Use safe string functions** - `snprintf()` not `sprintf()`, `strncpy()` not `strcpy()`
* **Validate all inputs** - especially from network sources
* **Check return values** - especially for memory allocation
* **Avoid buffer overflows** - always check bounds
* **No compiler warnings** - code must compile cleanly with `-Wall -Wextra`

### Testing Requirements

* **Add tests for new features** - unit tests in `tests/unit/`
* **Ensure existing tests pass** - run `make test`
* **Test on both Linux and Solaris** if possible
* **Include integration tests** for protocol changes
* **Test with sanitizers** - AddressSanitizer, ThreadSanitizer

### Documentation

* **Update README** if adding user-facing features
* **Add code comments** for complex logic
* **Document protocol changes** with references to STANAG 5066 spec
* **Update man pages** if changing command-line interface

### Commit Messages

Write clear, descriptive commit messages:

```
Short (50 chars or less) summary

More detailed explanatory text, if necessary. Wrap it to about 72
characters. The blank line separating the summary from the body is
critical.

Further paragraphs come after blank lines.

- Bullet points are okay

Fixes #123
```

### Pull Request Process

1. **Update documentation** for any changed functionality
2. **Add tests** for new features
3. **Ensure CI passes** - all automated checks must pass
4. **Update CHANGELOG.md** with noteworthy changes
5. **Request review** from maintainers
6. **Address review comments** promptly

## Project Structure

```
open5066/
├── src/
│   ├── core/         # Core daemon and I/O engine
│   ├── protocols/    # Protocol implementations (SIS, DTS, SMTP, HTTP)
│   ├── serial/       # Serial communication
│   ├── include/      # Public header files
│   └── tools/        # Utility programs
├── tests/
│   ├── unit/         # Unit tests
│   ├── integration/  # Integration tests
│   ├── security/     # Security tests
│   └── performance/  # Performance benchmarks
├── docs/             # Documentation
└── scripts/          # Build and test scripts
```

## Development Priorities

Current development priorities (as of v0.5):

1. **Security hardening** - Replace unsafe functions, add input validation
2. **Test coverage** - Increase test coverage to >70%
3. **ARQ implementation** - Currently only 10% complete
4. **Crypto module** - Currently 0% complete
5. **Documentation** - API documentation, architecture diagrams

## Questions?

If you have questions:
* Check existing documentation in `docs/`
* Search existing GitHub issues
* Create a new issue with the "question" label

## License

By contributing to Open5066, you agree that your contributions will be licensed under the GNU General Public License v2.0, with the exception of NATO C3 Agency components which are under a BSD-style license. See COPYING and COPYING_sis5066_h for details.

---

Thank you for contributing to Open5066!
