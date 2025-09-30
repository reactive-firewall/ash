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

#ifndef EACCESS_H
#define EACCESS_H

#include <sys/types.h>
#include <unistd.h>
// note faccessat is declared in <sys/unistd.h> on some systems like macOS and iOS
// and may be guarded by _SYS_UNISTD_H_

#if defined(__APPLE__) || defined(__linux__) // macOS or Linux
#include <sys/unistd.h> // Include for faccessat
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L // POSIX.1-2008 or later
#define HAVE_FACCESSAT 1
#else
#define HAVE_FACCESSAT 0 // Assume faccessat is not available
#endif
#else
#ifndef faccessat
#define HAVE_FACCESSAT 0 // Assume faccessat is not available
#else
#define HAVE_FACCESSAT 1
#endif /* !faccessat */
#endif /* !defined(__APPLE__) && !defined(__linux__) */

#include <fcntl.h>
#include <errno.h>

// Check if eaccess is already defined
#ifndef eaccess
// Function declaration
int eaccess(const char *path, int mode);
#endif /* !eaccess */
#endif /* !EACCESS_H */
