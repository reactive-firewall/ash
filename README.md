# A Shell - an Almquist /bin/sh implementation

A permissive, portable implementation of the Almquist shell (ash)

> [!INFORMATION]
> Derived from the Version 7 Almquist Shell sources as forked by FreeBSD (No Association).

## Badges

| Flavor |
| ------ |
| [![BSDLike (sh)](https://github.com/reactive-firewall/ash/actions/workflows/BSDLike-CI.yml/badge.svg)](https://github.com/reactive-firewall/ash/actions/workflows/BSDLike-CI.yml) |
| [![MuslLike (Docker)](https://github.com/reactive-firewall/ash/actions/workflows/Build-Docker-Ash.yaml/badge.svg?branch=MuslLike-Ash)](https://github.com/reactive-firewall/ash/actions/workflows/Build-Docker-Ash.yaml) |
| [![XNULike (Xcode)](https://github.com/reactive-firewall/ash/actions/workflows/CI-BUILD.yml/badge.svg?branch=XNULike-Ash)](https://github.com/reactive-firewall/ash/actions/workflows/CI-BUILD.yml) |

## Overview

**A Shell (ash)** is an Almquist `/bin/sh` implementation that preserves the historic behavior of
the Almquist shell while adding portability shims and build logic to support modern libc targets
(BSD-style libc/XNU and musl). It is intended to be a source-buildable, permissively licensed shell
suitable for embedding or system use.

## Note on lineage and POSIX

The Almquist shell (ash) was developed independently by Kenneth Almquist. Early shells and standards evolved around the same time as POSIX sh. The 1988 POSIX shell standard and historic ash sources influenced and overlap in behavior, but this project should be described as an implementation of the Almquist shell (ash) rather than merely "POSIX clone." It preserves the Almquist semantics as found in the Version 7 / FreeBSD lineage while being mindful of POSIX behavior where appropriate.

## Key goals

- Implement the Almquist (ash) /bin/sh semantics.
- Preserve classic ash/FreeBSD behavior where practical.
- Provide portability across:
  - BSD libc and derivatives (including XNU)
  - musl libc on Linux
- Avoid reliance on non-portable build tools where possible; provide dedicated branches and build logic per flavor.
- Maintain permissive licensing for downstream use.

## Special Flavors / Branches

- `FreeBSD-Ash` — archival branch with preserved FreeBSD ash sources.
- `MuslLike-Ash` — build logic and Docker setup targeting musl libc.
- `XNULike-Ash` — macOS / XNU-specific `xcodebuild` adjustments.

Choose the branch that matches your target platform.

## Building

Assume a POSIX-compatible environment and a C toolchain. Branch-specific instructions may vary.

1. Clone:
    ```sh
    git clone https://github.com/reactive-firewall/ash.git
    cd ash
    ```
2. Build (via script):
    ```sh
    ./build-ash.sh
    ```

> [!TIP]
> Unless the optional environment variable `DESTDIR` is set, the build script will place the
> build `sh` artifact at the relative path `./obj/ash/sh`.

## Compatibility

TL;DR; - This is an implementation of the 1989 Almquist shell by Kenneth Almquist. For background
on what it means to be "sh compatible", see:
[what does it mean to be sh compatible](https://unix.stackexchange.com/questions/145522/what-does-it-mean-to-be-sh-compatible/145524#145524)

## Compatibility notes

- This project implements the Almquist shell; it aims to reproduce _BSD-like_ ash semantics rather
  than strictly follow newer POSIX/GNU extensions.
- Platform-specific shims are included to handle libc/API differences; some behavior may vary
  slightly across flavors. (Please open a GitHub issue for any undocumented differences found)
- Tests and CI cover common behavior across flavors, but edge-case differences
  (signal handling, job control nuances, path lookup) may exist between builds targeting musl,
  BSD libc, and XNU.
- If strict POSIX-conformance is required for a target, validate with your POSIX test-suite; this
  project prioritizes portability first and faithful BSD ash semantics second.

### Flavor specific builds

Because of the significant differences in musl's `libc` and XNU/Darwin expectation of `mach-o`,
special build logic has been added to dedicated branches to ensure _some_ limited support with
these two use-cases.

## License

- Core: BSD-3-Clause. Individual files may carry other permissive licenses (MIT, Apache-2.0, etc.)—check file headers.
- Third-party headers or dependencies retain their original licenses (FreeBSD headers, XNU libSystem/APSL, musl headers, etc.).

Full BSD-3-Clause text:
---
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.

This code is derived from software contributed to Berkeley by
Kenneth Almquist.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

```plaintext
THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```
---

### Third-party licenses / notes

Depending on your exact build environment various headers may or may-not be relied on when building
`ash` and may have their own licenses. Some of those are mentioned here for connivance.
**(NO-ASSOCIATION)**

* FreeBSD System headers (when used) -- Typically also BSD-3-Clause
* XNU/Darwin libSystem headers (when used) -- [APSL](http://www.opensource.apple.com/apsl/)
* musl's `libc` headers (when used) -- Typically MIT

See individual source headers and branch documentation for details.

## Support

Open issues and PRs on GitHub.
