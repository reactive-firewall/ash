/*
* MIT License
*
* Copyright (c) 2025 Mr. Walls
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef MUSL_SHIM_H
#define MUSL_SHIM_H

#if defined(__clang__) && __clang__
#if __has_include(<sys/cdefs.h>)
#include <sys/cdefs.h> // Include for __unused trickery
#endif /* !__has_include(<sys/cdefs.h>) */
#endif /* !defined(__clang__) && __clang__ */

#if defined(__unused)
#define _UNUSED_ATTR __unused
#else
#if defined(__has_attribute)

/*
 * Musl '__unused' Patch & Compatibility notice:
 * Using GNU style __unused AND _UNUSED_ATTR distinction.
 * Rationale: This is to avoid shadowing __unused identifiers in GNUC headers.
 * FreeBSD/Darwin/baremusl use '__unused' exclusively as the '__attribute__((__unused__))'
 * and not as an identifier. Further as a compatibility focused implementation baremusl's
 * cdefs.h will omit defining '__unused' when '__GNUC__' is defined, so not to clobber the
 * GCC '__unused' behavior that musl libc expects. So to apply the __unused__ attribute without
 * constant conditional preprocessing everywhere the '_UNUSED_ATTR' is defined here for simpler
 * patching.
 *
 * PORTING TIP:
 * When porting to musl libc, pure FreeBSD code may need to have a patch for:
 * 1. includes of "<sys/cdefs.h>" should be immediately followed by an include of this header.
 * 2. every "__unused" -> "_UNUSED_ATTR" (drop-in compatible)
 *
 */
#if !defined(__GNUC__)
#warning "Using GNU style __unused AND _UNUSED_ATTR distinction. This may break pure BSD source."
#else
#warning "Using GNU style __unused AND _UNUSED_ATTR distinction compatibility mode."
#endif

#if __has_attribute(__unused__)
#ifndef _UNUSED_ATTR
/// Use this to declare `__attribute__((__unused__))` in the code.
#define	_UNUSED_ATTR	__attribute__((__unused__))
#endif /* !_UNUSED_ATTR */
#else /* !__attribute__ ((__unused__)) */
#ifndef _UNUSED_ATTR
#warning "No compiler support for the unused attribute."
#define _UNUSED_ATTR
#endif /* !_UNUSED_ATTR */
#endif /* END ((__unused__)) */

#endif /* !defined(__has_attribute) */

#endif /* ! defined(__unused) */

#endif /* !MUSL_SHIM_H */
