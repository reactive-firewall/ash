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

#ifndef _READLINE_H_
#include "libedit_shim.h"

EditLine* el_init(const char* name, FILE* input, FILE* output) {
	// Allocate memory for the EditLine structure
	EditLine* el = malloc(sizeof(EditLine));
	if (!el) {
		return NULL; // Memory allocation failed
	}

	// Initialize readline with the provided input and output
	rl_instream = input;
	rl_outstream = output;

	// Set the prompt (optional)
	rl_prompt = "prompt> ";

	// Initialize readline and enable history support
	rl_initialize();
	using_history();

	return el;
}

void el_end(EditLine* el) {
	// Clean up resources
	if (el) {
		free(el);
	}
}

int el_get(EditLine* el, int op) {
	// Return the requested operation; for now, we can ignore it
	return 0; // Placeholder
}

int el_set(EditLine* el, int op, ...) {
	// Handle setting options; for now, we can ignore it
	return 0; // Placeholder
}

int el_parse(EditLine* el, const char* str) {
	// Parse a command string; for now, we can ignore it
	return 0; // Placeholder
}

int el_source(EditLine* el, const char* filename) {
	// Source a file; for now, we can ignore it
	return 0; // Placeholder
}

char* el_gets(EditLine* el, int* len) {
	// Use readline to get a line
	char* line = readline(rl_prompt);
	if (line) {
		*len = strlen(line);
		add_history(line); // Add to history
	} else {
		*len = 0; // No input
	}
	return line; // Return the line read
}

int el_resize(EditLine* el, int size) {
	// Resize functionality; for now, we can ignore it
	return 0; // Placeholder
}

int el_fn_complete(EditLine* el) {
	// Implement completion logic; for now, we can ignore it
	return 0; // Placeholder
}


#endif /* !_READLINE_H_ */
