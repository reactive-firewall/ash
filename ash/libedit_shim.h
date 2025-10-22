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

#ifndef LIBEDIT_SHIM_H
#define LIBEDIT_SHIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__clang__) && __clang__
#if __has_include(<editline/editline.h>)
#include <editline/editline.h> // Include for EditLine
#endif /* !__has_include(<editline/editline.h>) */
#endif /* !defined(__clang__) && __clang__ */

#if defined(__clang__) && __clang__
#if __has_include(<histedit.h>)
#include <histedit.h> // Include for EditLine
#endif /* !__has_include(<histedit.h>) */
#endif /* !defined(__clang__) && __clang__ */

#if defined(__clang__) && __clang__
#if __has_include(<readline/readline.h>)
#ifndef _READLINE_H_
#include <readline/readline.h>
#if __has_include(<readline/history.h>)
#include <readline/history.h>
#endif
#endif /* !_READLINE_H_ */
#endif /* !__has_include(<readline/readline.h>) */

#ifndef __weak
#if __has_attribute(weak)
#define __weak	__attribute__((weak))
#elif __has_attribute(__weak__)
#define __weak	__attribute__((__weak__))
#else
#define __weak
#endif /* !__has_attribute_weak */
#endif /* !__weak */
#else
#ifndef __weak
#warning "compiling libedit_shim without __weak support. This may break things."
#define __weak
#endif
#endif /* !defined(__clang__) && __clang__ */

#ifndef _HISTEDIT_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	// You can add any necessary fields here if needed
} EditLine;

__weak EditLine* el_init(const char* name, FILE* input, FILE* output);
__weak void el_end(EditLine* el);
__weak int el_get(EditLine *, int op, ...);
__weak int el_set(EditLine* el, int op, ...);
__weak int el_parse(EditLine* el, const char* str);
__weak int el_source(EditLine* el, const char* filename);
__weak char* el_gets(EditLine* el, int* len);
__weak int el_resize(EditLine* el, int size);
__weak int el_fn_complete(EditLine* el);
#ifdef __cplusplus
}
#endif
#endif


#endif /* !LIBEDIT_SHIM */
