/*
 * funopen_shim.h
 *
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

#ifndef FUNOPEN_SHIM_H
#define FUNOPEN_SHIM_H

/* funopen_shim.h
 * Provide funopen/fropen/fwopen on systems that lack them.
 *
 * Usage: #include <stdio.h> then #include "funopen_shim.h"
 *
 * This header tries:
 * 1) If funopen already declared, do nothing.
 * 2) If fopencookie (glibc/musl extension) exists, implement funopen using it.
 * 3) Otherwise provide a POSIX pipe + thread fallback (best-effort).
 *
 * Note: This shim is pragmatic, not a byte-for-byte behavior match for
 * FreeBSD's funopen. Tests recommended for your use-case.
 */

#if defined(__has_include)
#if __has_include(<stdio.h>)
#include <stdio.h>
#endif
#if __has_include(<stdlib.h>)
#include <stdlib.h>
#endif
#if __has_include(<stdlib.h>)
#include <errno.h>
#endif
#if __has_include(<string.h>)
#include <string.h>
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || (defined(FUNOPEN_SHIM_IMPLEMENTED) && FUNOPEN_SHIM_IMPLEMENTED)
/* BSD-like system: funopen already present in stdio.h. */
#else
#if !defined(funopen) && !defined(_funopen)

#warning "Using non-BSD funopen compatibility mode. This may break things."

#if defined(F_PERM)
// Check if we can use provided definitions
#if !defined(__SRD) && defined(F_NOWR)
#define __SRD (F_PERM | F_NOWR)       // Readable
#endif
#if !defined(__SWR) && defined(F_NORD)
#define __SWR (F_PERM | F_NORD)      // Writable
#endif
#if !defined(__SRW)
#define __SRW (F_PERM)      // Read/Write
#endif
#else
// just provide definitions
#if !defined(__SRD)
#define __SRD 0x0004      // Readable
#endif
#if !defined(__SWR)
#define __SWR 0x0008      // Writable
#endif
#if !defined(__SRW)
#define __SRW 0x0010      // Read/Write
#endif
#endif

#ifndef FILE_T_FD_FIELD
#if defined(__sFILE)
#define FILE_T_FD_FIELD	_file
#elif defined(_IO_FILE)
#define FILE_T_FD_FIELD	fd
#else
#define FILE_T_FD_FIELD	_file
#endif
#endif /* !FILE_T_FD_FIELD */

#ifndef FILE_T_FLAGS_FIELD
#if defined(__sFILE)
#define FILE_T_FLAGS_FIELD	_flags
#elif defined(_IO_FILE)
#define FILE_T_FLAGS_FIELD	flags
#else
#define FILE_T_FLAGS_FIELD	_flags
#endif
#endif /* !FILE_T_FLAGS_FIELD */

#ifndef FILE_T_COOKIE_FIELD
#if defined(__sFILE)
#define FILE_T_COOKIE_FIELD	_cookie
#elif defined(_IO_FILE)
#define FILE_T_COOKIE_FIELD	cookie
#else
#define FILE_T_COOKIE_FIELD	_cookie
#endif
#endif /* !FILE_T_COOKIE_FIELD */

#ifndef FILE_T_CLOSE_FUN_FIELD
#if defined(__sFILE)
#define FILE_T_CLOSE_FUN_FIELD	_close
#elif defined(_IO_FILE)
#define FILE_T_CLOSE_FUN_FIELD	close
#else
#define FILE_T_CLOSE_FUN_FIELD	_close
#endif
#endif /* !FILE_T_CLOSE_FUN_FIELD */

#ifndef FILE_T_READ_FUN_FIELD
#if defined(__sFILE)
#define FILE_T_READ_FUN_FIELD	_read
#elif defined(_IO_FILE)
#define FILE_T_READ_FUN_FIELD	read
#else
#define FILE_T_READ_FUN_FIELD	_read
#endif
#endif /* !FILE_T_READ_FUN_FIELD */

#ifndef FILE_T_SEEK_FUN_FIELD
#if defined(__sFILE)
#define FILE_T_SEEK_FUN_FIELD	_seek
#elif defined(_IO_FILE)
#define FILE_T_SEEK_FUN_FIELD	seek
#else
#define FILE_T_SEEK_FUN_FIELD	_seek
#endif
#endif /* !FILE_T_SEEK_FUN_FIELD */

#ifndef FILE_T_WRITE_FUN_FIELD
#if defined(__sFILE)
#define FILE_T_WRITE_FUN_FIELD	_write
#elif defined(_IO_FILE)
#define FILE_T_WRITE_FUN_FIELD	write
#else
#define FILE_T_WRITE_FUN_FIELD	_write
#endif
#endif /* !FILE_T_WRITE_FUN_FIELD */

#ifndef FILE_T_BUFFER_FIELD
#if defined(__sFILE) || defined(__sbuf)
#define FILE_T_BUFFER_FIELD	_bf
#elif defined(_IO_FILE)
#define FILE_T_BUFFER_FIELD	buf
#else
#define FILE_T_BUFFER_FIELD	_buffer
#endif
#endif /* !FILE_T_BUFFER_FIELD */

#ifndef FILE_T_BUF_SIZE_FIELD
#if defined(__sFILE)
/* _lbfsize is 0 or -_bf._size ... so sign matters */
#define FILE_T_BUF_SIZE_FIELD	_lbfsize
#elif defined(_IO_FILE)
#define FILE_T_BUF_SIZE_FIELD	buf_size
#else
#define FILE_T_BUF_SIZE_FIELD	_buffer_size
#endif
#endif /* !FILE_T_BUF_SIZE_FIELD */


#if !defined(FILE)

/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 *
 * Certain members of __sFILE are accessed directly via macros or
 * inline functions.  To preserve ABI compat, these members must not
 * be disturbed.  These members are marked below with (*).
 */


#if !(defined(_IO_FILE) || defined(__sFILE))
#warning "No system support for the FILE type. This will probably break things."
#if !defined(_IO_FILE)
///>important
///> The `fwopen_shim.h` file contains code that is under the MIT Licence, As described here:
///> https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
///> and is hearby Acknowledged.
/// See https://git.musl-libc.org/cgit/musl/tree/src/internal/stdio_impl.h
///
#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark MUSL FILE Compatibility
#endif /* !__clang__ */

// BELOW FROM https://git.musl-libc.org/cgit/musl/tree/src/internal/stdio_impl.h
struct _IO_FILE {
	unsigned flags;
	unsigned char *rpos, *rend;
	int (*close)(FILE *);
	unsigned char *wend, *wpos;
	unsigned char *mustbezero_1;
	unsigned char *wbase;
	size_t (*read)(FILE *, unsigned char *, size_t);
	size_t (*write)(FILE *, const unsigned char *, size_t);
	off_t (*seek)(FILE *, off_t, int);
	unsigned char *buf;
	size_t buf_size;
	FILE *prev, *next;
	int fd;
	int pipe_pid;
	long lockcount;
	int mode;
	volatile int lock;
	int lbf;
	void *cookie;
	off_t off;
	char *getln_buf;
	void *mustbezero_2;
	unsigned char *shend;
	off_t shlim, shcnt;
	FILE *prev_locked, *next_locked;
	struct __locale_struct *locale;
};
// ABOVE FROM https://git.musl-libc.org/cgit/musl/tree/src/internal/stdio_impl.h

#undef FILE_T_FD_FIELD
#define FILE_T_FD_FIELD	fd
#undef FILE_T_FLAGS_FIELD
#define FILE_T_FLAGS_FIELD	flags
#undef FILE_T_COOKIE_FIELD
#define FILE_T_COOKIE_FIELD	cookie
#undef FILE_T_CLOSE_FUN_FIELD
#define FILE_T_CLOSE_FUN_FIELD	close
#undef FILE_T_READ_FUN_FIELD
#define FILE_T_READ_FUN_FIELD	read
#undef FILE_T_SEEK_FUN_FIELD
#define FILE_T_SEEK_FUN_FIELD	seek
#undef FILE_T_WRITE_FUN_FIELD
#define FILE_T_WRITE_FUN_FIELD	write
#undef FILE_T_BUFFER_FIELD
#define FILE_T_BUFFER_FIELD	buf
#undef FILE_T_BUF_SIZE_FIELD
#define FILE_T_BUF_SIZE_FIELD	buf_size

#endif /* !defined(_IO_FILE) */
#endif /* !(defined(_IO_FILE) || defined(__sFILE)) */
#endif /* !defined(FILE) */

#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark funopen Shim
#endif /* !__clang__ */

// Simple malloc function for FILE allocation
FILE *create_file_structure() {
#if !(defined(_IO_FILE) || defined(__sFILE))
	FILE *fp = tmpfile();
	if (!fp) {
		/* tmpfile may fail; set errno accordingly */
		return NULL;
	}
#elif defined(_IO_FILE)
	FILE *fp = (FILE *)malloc(sizeof(_IO_FILE));
	if (fp) {
		memset(fp, 0, sizeof(_IO_FILE)); // Initialize to zero
	}
#elif defined(__sFILE)
	FILE *fp = (FILE *)malloc(sizeof(__sFILE));
	if (fp) {
		memset(fp, 0, sizeof(__sFILE)); // Initialize to zero
	}
#else
#error "will need to get clever"
#endif
	return fp;
}

#if !defined(FUNOPEN_SHIM_IMPLEMENTED)
#define FUNOPEN_SHIM_IMPLEMENTED 1
FILE *funopen(const void *cookie,
			  int (*readfn)(void *, char *, int),
			  int (*writefn)(void *, const char *, int),
			  fpos_t (*seekfn)(void *, fpos_t, int),
			  int (*closefn)(void *))
{
	FILE *fp;
	int flags;

	if (readfn == NULL) {
		if (writefn == NULL) {        // Illegal case
			errno = EINVAL;
			return NULL;
		} else {
			flags = __SWR;            // Write only
		}
	} else {
		if (writefn == NULL) {
			flags = __SRD;            // Read only
		} else {
			flags = __SRW;            // Read/Write
		}
	}

	if ((fp = create_file_structure()) == NULL) {
		return NULL;                  // Allocation failed
	}
#if defined(__clang__)
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wincompatible-function-pointer-types\"")
#elif defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wincompatible-function-pointer-types\"")
#else
	/* START will throw compiler warning */
#endif
	fp->FILE_T_FD_FIELD = flags;
	fp->FILE_T_COOKIE_FIELD = (void *)cookie;
	fp->FILE_T_READ_FUN_FIELD = readfn;
	fp->FILE_T_WRITE_FUN_FIELD = writefn;
	fp->FILE_T_SEEK_FUN_FIELD = seekfn;
	fp->FILE_T_CLOSE_FUN_FIELD = closefn;
#if defined(__clang__)
_Pragma("clang diagnostic pop")
#elif defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#else
	/* END will throw compiler warning */
#endif
	return fp;                        // Return the initialized FILE structure
}
#endif

#endif
#endif /* funopen not defined */

/* Provide fropen/fwopen macros like BSD */
#ifndef fropen
#define fropen(cookie, fn) funopen(cookie, fn, 0, 0, 0)
#endif
#ifndef fwopen
#define fwopen(cookie, fn) funopen(cookie, 0, fn, 0, 0)
#endif

#endif /* FUNOPEN_SHIM_H */
