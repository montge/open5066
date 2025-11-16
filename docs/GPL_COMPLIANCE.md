# GPL v2 Compliance and Integration Guide

## Overview

Open5066 is licensed under **GNU General Public License v2 (GPL-2)**, with some NATO C3 Agency components under BSD-style licenses. This document explains:

1. What GPL v2 means for users and developers
2. How proprietary/non-GPL software can legally use Open5066
3. Compliance requirements and best practices
4. Business models compatible with GPL v2

**Disclaimer**: This document provides general information only. Consult with a qualified attorney for legal advice specific to your situation.

---

## Understanding GPL v2

### What GPL v2 Grants You

GPL v2 grants you **four fundamental freedoms**:

1. **Freedom to use**: Run the program for any purpose
2. **Freedom to study**: Access and study the source code
3. **Freedom to modify**: Modify the program to suit your needs
4. **Freedom to distribute**: Share the original or modified versions

### What GPL v2 Requires

If you **distribute** GPL v2 software (original or modified):

1. **Provide source code**: Make complete source code available
2. **License under GPL v2**: Derived works must also be GPL v2
3. **Preserve copyright notices**: Maintain attribution and license text
4. **Document changes**: Note any modifications made

**Key Point**: GPL v2 obligations are triggered by **distribution**, not internal use.

---

## Legal Integration Options for Proprietary Software

### 1. Network Service Architecture (Recommended)

**Approach**: Run Open5066 as a **network service**, with proprietary software communicating via network protocols.

**How It Works**:
- Open5066 runs as a standalone daemon (s5066d)
- Proprietary application connects via TCP/IP
- Communication uses STANAG 5066 SIS/DTS protocols
- No GPL obligations on proprietary application

**Example Architecture**:
```
┌─────────────────────────┐
│  Proprietary Application│
│  (No GPL obligations)   │
└───────────┬─────────────┘
            │ TCP/IP (SIS/DTS protocol)
            │
┌───────────▼─────────────┐
│  s5066d Daemon          │
│  (GPL v2)               │
└───────────┬─────────────┘
            │ HF Radio Link
            ▼
```

**Legal Rationale**:
- GPL v2 does NOT have the "network copyleft" provision of AGPL
- Network protocols are interfaces, not derived works
- FSF explicitly acknowledges this: "If the modules are included in the same executable file, they are definitely combined in one program. If modules are designed to run linked together in a shared address space, that almost surely means combining them into one program."
- Network communication across TCP sockets is NOT "combining into one program"

**Use Cases**:
- Military C2 systems with proprietary user interfaces
- Commercial radio systems using STANAG 5066
- Integration with existing proprietary infrastructure

### 2. Separate Process Communication

**Approach**: Run Open5066 as a **separate process**, with IPC via pipes, sockets, or message queues.

**How It Works**:
- Proprietary software spawns s5066d as subprocess
- Communication via stdin/stdout, named pipes, or UNIX sockets
- Clear process boundary between GPL and proprietary code

**Example**:
```bash
# Proprietary software launches s5066d
./proprietary_app | s5066d -p sis::5066 -p dts::5067
```

**Legal Rationale**:
- Separate processes with well-defined communication interface
- GPL applies to s5066d process only
- IPC mechanisms are not "linking" in the GPL sense

**Use Cases**:
- Desktop applications with STANAG 5066 support
- Batch processing systems
- Automation and scripting

### 3. Internal Use Only (No Distribution)

**Approach**: Use Open5066 internally within your organization **without distributing** to others.

**How It Works**:
- Deploy Open5066 on internal servers
- Modify as needed for internal requirements
- **No distribution** to customers or external parties

**Legal Rationale**:
- GPL v2 obligations are triggered by **distribution**
- Internal use within a single organization is NOT distribution
- No requirement to share modifications if not distributing

**Important**: "Distribution" includes:
- Selling or giving software to customers
- Providing software to contractors/partners
- Cloud services accessible to external users

**Use Cases**:
- Government/military internal networks
- Research and development
- Internal corporate infrastructure

### 4. Commercial Support and Services Model

**Approach**: Offer **services and support** for Open5066, not selling the software itself.

**How It Works**:
- Open5066 remains GPL v2 (free software)
- Charge for: installation, configuration, training, support, maintenance
- Provide proprietary **value-added services** around GPL software

**Examples**:
- **Support contracts**: 24/7 technical support, SLAs, bug fixes
- **Professional services**: Custom integration, deployment assistance
- **Training**: Operator training, developer workshops
- **Consulting**: Architecture design, performance tuning
- **Managed services**: Hosted STANAG 5066 infrastructure

**Legal Rationale**:
- GPL allows selling support and services
- Many successful businesses built on this model (Red Hat, Canonical, etc.)
- Software remains free, expertise and services are paid

**Use Cases**:
- System integrators
- Defense contractors
- Radio manufacturers

### 5. Dual Licensing (Limited Applicability)

**Approach**: Offer software under **both GPL v2 and a proprietary license**.

**How It Works**:
- GPL v2 version available to everyone
- Pay for proprietary license allowing non-GPL use
- Proprietary licensees can integrate into closed-source products

**Important Limitation**: Only the **copyright holder** can offer dual licensing.
- **Open5066 status**: This is a fork with multiple contributors
- **Original author**: Sampo Kellomäki
- **This fork maintainer**: Cannot unilaterally change licensing
- **Path forward**: Would require agreement from all copyright holders

**Why This Is Difficult**:
- Open5066 is a derivative of original work
- Multiple contributors may have added code
- Would need permission from all contributors
- NATO C3 Agency components have BSD license (more permissive)

**Use Cases** (if licensing could be arranged):
- Proprietary radio systems
- Commercial military equipment
- Closed-source embedded systems

### 6. Clean-Room Reimplementation

**Approach**: Create an **independent implementation** of STANAG 5066 protocol **without using Open5066 code**.

**How It Works**:
- Study STANAG 5066 specification (NATO Standardization Agreement)
- Implement protocol from specification, NOT from Open5066 code
- Your implementation can use any license you choose
- No GPL obligations because no GPL code used

**Legal Rationale**:
- Protocols and specifications are not copyrightable
- Independent implementations don't create derived works
- Must NOT look at Open5066 source code during implementation

**Use Cases**:
- Proprietary radio systems with STANAG 5066 support
- Commercial implementations
- Embedded firmware

**Challenges**:
- Expensive (months/years of development)
- Risk of accidentally copying (contamination)
- May infringe if code is too similar
- STANAG 5066 specification may have access restrictions

### 7. Dynamic Linking (Controversial)

**Approach**: Link to Open5066 as a **dynamic library** at runtime.

**Legal Status**: **HIGHLY CONTROVERSIAL** - seek legal advice

**Arguments For**:
- Some interpret GPL as allowing dynamic linking to separate works
- LGPL (Lesser GPL) exists specifically to allow this
- System library exception might apply

**Arguments Against**:
- FSF position: Dynamic linking creates derived work
- GPL v2 says "work based on the Program"
- Courts have not definitively settled this

**Recommendation**: **Avoid this approach** unless you have explicit legal counsel approval.

---

## Recommended Approaches by Use Case

### Military/Government Systems

**Best Option**: **Network Service Architecture (#1)**

**Why**:
- Clear legal separation
- Standard STANAG 5066 protocol interfaces
- Proprietary C2 systems connect via TCP/IP
- No GPL obligations on classified systems
- Aligns with defense system architecture patterns

**Example**:
```
Classified C2 System (Proprietary)
    ↕ TCP/IP (SIS/DTS)
Open5066 Daemon (GPL) ← HF Radio
```

### Commercial Radio Equipment

**Best Options**:
1. **Network Service Architecture (#1)** - if system supports TCP/IP
2. **Clean-Room Reimplementation (#6)** - for embedded firmware
3. **Commercial Support Model (#4)** - if selling radio + software bundle

**Why**:
- Network service allows proprietary control software
- Clean-room allows fully proprietary implementation
- Support model allows selling "hardware + software + support" bundles

### System Integrators

**Best Options**:
1. **Commercial Support and Services (#4)**
2. **Network Service Architecture (#1)**
3. **Internal Use Only (#3)** - for government contracts

**Why**:
- Support model is proven business approach
- Network architecture allows integration with diverse systems
- Government contracts often involve internal deployment

### Research and Development

**Best Options**:
1. **Internal Use Only (#3)**
2. **Contribute improvements back to project**

**Why**:
- No distribution = no GPL obligations
- Contributing back builds reputation
- Access to community improvements

---

## Compliance Best Practices

### If You Must Distribute Modified Open5066

If you choose to distribute Open5066 or modified versions, follow these steps:

1. **Preserve License Files**:
   ```
   COPYING                  # GPL v2 license text
   COPYING_sis5066_h        # NATO C3 Agency BSD license
   ```

2. **Provide Source Code**:
   - All source files, including modifications
   - Build scripts and configuration
   - Installation instructions
   - Must be "complete and corresponding source"

3. **Document Changes**:
   - Maintain CHANGELOG.md
   - Add copyright notices for your modifications
   - Use version control (Git) to track changes

4. **Offer Source Code**:
   - Include with binary distribution, OR
   - Provide written offer valid for 3 years, OR
   - Host on public Git repository (GitHub, GitLab)

5. **License Statement**:
   ```
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   ```

### Common GPL Violations to Avoid

❌ **Violation**: Distributing binaries without source code
✅ **Compliant**: Provide source alongside binaries or via public repository

❌ **Violation**: Adding proprietary modules that link with GPL code
✅ **Compliant**: Use network service architecture for separation

❌ **Violation**: Claiming proprietary copyright on modified GPL code
✅ **Compliant**: License modifications under GPL v2, note your contributions

❌ **Violation**: Stripping copyright notices and license text
✅ **Compliant**: Preserve all notices and COPYING files

---

## NATO C3 Agency Components

**License**: BSD-style license (see COPYING_sis5066_h)

**Components**:
- `src/include/sis5066.h` - SIS protocol definitions

**Implications**:
- BSD is **more permissive** than GPL
- Can use in proprietary software
- Must preserve copyright notice and disclaimer
- Can relicense under GPL (but not vice versa)

**Strategy**: If possible, isolate use of BSD components to allow more flexible licensing.

---

## Business Models Compatible with GPL

### Successful GPL-Based Businesses

Many companies build profitable businesses around GPL software:

1. **Red Hat / IBM**: $3+ billion annual revenue from GPL Linux
2. **Canonical**: Ubuntu support and services
3. **GitLab**: Open core model (GPL community + proprietary enterprise)
4. **WordPress**: GPL CMS, commercial hosting/plugins/themes

### Revenue Streams

- **Support contracts**: Annual support subscriptions
- **Professional services**: Integration, customization, training
- **Managed services**: SaaS offerings (no distribution)
- **Certification**: Official training and certification programs
- **Hardware bundles**: Sell hardware + software + support
- **Dual licensing**: If you own all copyrights
- **Open core**: GPL base + proprietary extensions (carefully structured)

---

## Frequently Asked Questions

### Q: Can I sell GPL software?

**A**: Yes! GPL explicitly permits selling software. However:
- Buyers can redistribute for free (GPL freedom)
- Must provide source code
- Cannot add restrictive licenses

Revenue usually comes from **value-added services**, not software sales.

### Q: Can I use Open5066 in my commercial product?

**A**: Yes, but:
- If you **distribute** the product, must comply with GPL (provide source)
- If using as **network service**, proprietary client software is OK
- If using **internally only**, no distribution obligations

### Q: What if I modify Open5066?

**A**: Modifications are allowed. If you distribute modified versions:
- Must license under GPL v2
- Must provide source code including modifications
- Must document changes

**Internal modifications** (no distribution) have no sharing requirement.

### Q: Can I keep my modifications secret?

**A**: Only if you DON'T distribute:
- Internal use only: Yes, can keep modifications secret
- Distribute to customers: No, must provide source
- Network service: Yes, no distribution requirement (unlike AGPL)

### Q: What about patents?

**A**: GPL v2 has limited patent provisions:
- Contributors implicitly grant patent license for their contributions
- GPL v3 has stronger patent language
- Consult patent attorney if patents are involved

### Q: Can I link Open5066 with proprietary libraries?

**A**: Controversial:
- FSF says: Creating derived work requires GPL
- System libraries: May have exception
- Network separation: Cleanest approach
- **Recommendation**: Use network service architecture

---

## Recommended Resources

### Legal Resources

- [GNU GPL v2 Full Text](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)
- [GNU GPL FAQ](https://www.gnu.org/licenses/gpl-faq.html)
- [FSF Licensing Guide](https://www.gnu.org/licenses/licenses.html)
- [Software Freedom Law Center](https://softwarefreedom.org/)

### Technical Resources

- [STANAG 5066 Overview](https://en.wikipedia.org/wiki/STANAG_5066)
- Open5066 Documentation: `docs/README`
- Architecture Guide: `docs/ARCHITECTURE.md`

### Business Models

- [Open Source Business Models](https://en.wikipedia.org/wiki/Business_models_for_open-source_software)
- [Red Hat Business Model](https://www.redhat.com/en/about/company)

---

## Contact and Legal Advice

**This document is not legal advice**. For legal questions:

1. **Consult qualified attorney** experienced in:
   - Software licensing
   - Open source compliance
   - Your jurisdiction's copyright law

2. **Consider organizations that can help**:
   - Software Freedom Law Center (SFLC)
   - Free Software Foundation (FSF)
   - Open Source Initiative (OSI)
   - Your company's legal counsel

3. **For Open5066-specific questions**:
   - File GitHub issue: https://github.com/montge/open5066/issues
   - Community discussion (non-legal questions)

---

## Summary: Quick Decision Tree

```
Do you want to use Open5066?
│
├─ Just running it internally?
│  └─ ✅ No GPL obligations, use freely
│
├─ Providing as network service to customers?
│  └─ ✅ OK! Clients can be proprietary
│     (recommend network service architecture)
│
├─ Distributing to customers?
│  │
│  ├─ Willing to provide source code?
│  │  └─ ✅ OK! Follow GPL v2 compliance steps
│  │
│  └─ Need to keep code proprietary?
│     │
│     ├─ Can use network architecture?
│     │  └─ ✅ OK! GPL daemon + proprietary client
│     │
│     ├─ Have budget for clean-room implementation?
│     │  └─ ✅ OK! Implement STANAG 5066 independently
│     │
│     └─ Otherwise?
│        └─ ⚠️  Consult attorney about options
```

---

## Conclusion

Open5066's GPL v2 license provides **maximum freedom** while ensuring the software remains free and open. For commercial use:

**Best Approach**: **Network service architecture**
- Deploy s5066d daemon (GPL)
- Connect via TCP/IP using STANAG 5066 protocols
- Proprietary applications have no GPL obligations
- Clean legal separation, proven architecture

**Alternative Approaches**:
- Support and services business model
- Internal use only
- Clean-room reimplementation (expensive)

GPL v2 is **business-friendly** when approached correctly. Many successful companies have built profitable businesses around GPL software.

---

**Last Updated**: 2025-11-16
**License**: This document is provided under CC0 (public domain) to maximize usefulness.
