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
#include "eaccess_shim.h"

int eaccess(const char *path, int mode) {
	if (path == NULL) {
		errno = EINVAL; // Invalid argument
		return -1;
	}
	// Check if faccessat is available
#if defined(HAVE_FACCESSAT) && HAVE_FACCESSAT
	// Use faccessat with AT_FDCWD and AT_EACCESS to check access based on EUID
	int result = faccessat(AT_FDCWD, path, mode, AT_EACCESS);
	if (result == -1) {
		errno = EACCES; // File not accessible
		return -1; // File not accessible
	}
	return 0; // File is accessible
#else
	// Emit a warning if faccessat is not available
#warning "faccessat is not available; falling back to access() for eaccess()"
	return access(path, mode); // Fallback to access
#endif
}

#endif /* !EACCESS_H */
