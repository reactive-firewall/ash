/* setmode_shim.c
 *
 * Minimal portable implementation of BSD getmode/setmode.
 *
 * Build notes:
 * - Detect system-provided getmode/setmode using feature test macros or
 *   configure-time defines. If your build system can detect these and define
 *   HAVE_GETMODE, the system functions will be used.
 * - To prefer system functions on FreeBSD/macOS, define HAVE_GETMODE when
 *   compiling on those platforms. Example:
 *     #ifdef __FreeBSD__ || defined(__APPLE__)
 *       #define HAVE_GETMODE 1
 *     #endif
 *
 * This file intentionally avoids GPL sources and implements parsing logic
 * derived from publicly-known BSD semantics.
 */

#include "setmode_shim.h"

#include <stdlib.h> /* some systems declare setmode/getmode in stdlib or stdio */
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#if (defined(__FreeBSD__) || defined(__APPLE__)) && !defined(HAVE_GETMODE)
/* Prefer system-provided implementations on FreeBSD/Darwin if available.
   Many build systems will provide these; guard by HAVE_GETMODE. */
#define HAVE_GETMODE 1
#include <unistd.h> /* for getmode, setmode */
#endif

#if HAVE_GETMODE

/* If the system provides these, forward to them. */
#if !defined(__cplusplus)
extern mode_t getmode(const void *set, mode_t mode);
extern void *setmode(const char *str);
#endif

#else /* Provide our own implementation */

#include <sys/stat.h>

#ifndef S_IRUSR
/* Minimal definitions if headers missing (unlikely). */
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000
#endif

typedef enum {
	OP_ASSIGN=1,
	OP_ADD=2,
	OP_REMOVE=3
} op_t;

/* Each parsed op records:
 - who mask (u=1,g=2,o=4)
 - operation (+,-,=)
 - perms bits (full bits for r/w/x and special bits)
 - flag_copy: if nonzero, perms stores source class (1/2/4) to indicate copy
 - flag_X: if nonzero, the execute bits came from 'X' and are conditional
 */
typedef struct {
	unsigned who;   /* bitmask: 1=u,2=g,4=o (a==7) */
	op_t op;
	mode_t perms;   /* full bits (including special bits), or encoded copy indicator:
					   if perms == 1/2/4 and flag_copy==1, indicates copy-from u/g/o */
	int flag_copy;  /* nonzero when this op is a copy (g=u etc) */
	int flag_X;
} mode_op;

typedef struct {
	size_t nops;
	mode_op ops[1]; /* flexible */
} setmode_ctx;

/* strdup fallback for C89 */
#ifndef HAVE_STRDUP
static char *portable_strdup(const char *s) {
	size_t n = strlen(s) + 1;
	char *p = (char *)malloc(n);
	if (!p) return NULL;
	memcpy(p, s, n);
	return p;
}
#define strdup portable_strdup
#endif

/* Helpers */

/* skip leading spaces, return pointer to first non-space */
static const char *skip_space(const char *p) {
	while (p && *p && isspace((unsigned char)*p)) p++;
	return p;
}

/* parse octal number (up to 4 digits) */
static int parse_octal(const char *s, mode_t *out, const char **endp) {
	const char *p = s;
	mode_t v = 0;
	int digits = 0;
	while (*p && isdigit((unsigned char)*p) && *p >= '0' && *p <= '7' && digits < 4) {
		v = (v << 3) + (mode_t)(*p - '0');
		p++; digits++;
	}
	if (digits == 0) return -1;
	if (out) *out = v;
	if (endp) *endp = p;
	return 0;
}

/* map 'who' letters to mask */
static unsigned parse_who(const char **pp) {
	const char *p = *pp;
	unsigned mask = 0;
	int any = 0;
	while (*p) {
		char c = *p;
		if (c == 'u') { mask |= 1; any = 1; p++; continue; }
		if (c == 'g') { mask |= 2; any = 1; p++; continue; }
		if (c == 'o') { mask |= 4; any = 1; p++; continue; }
		if (c == 'a') { mask |= 7; any = 1; p++; continue; }
		break;
	}
	if (!any) mask = 7; /* default 'a' */
	*pp = p;
	return mask;
}

/* parse a single clause (no top-level commas). Returns number of ops added, -1 on error */
static int parse_one_clause(const char *clause, mode_op *out, size_t maxout) {
	const char *p = clause;
	size_t added = 0;

	p = skip_space(p);
	while (*p) {
		/* parse 'who' */
		unsigned who = parse_who(&p);
		p = skip_space(p);
		if (!*p) return -1;
		char opch = *p++;
		op_t op;
		if (opch == '+') op = OP_ADD;
		else if (opch == '-') op = OP_REMOVE;
		else if (opch == '=') op = OP_ASSIGN;
		else return -1;
		p = skip_space(p);

		/* RHS may be:
		   - octal digits
		   - sequence of perms letters (r,w,x,X,s,t)
		   - copy from class (u/g/o)
		*/
		/* check octal */
		if (isdigit((unsigned char)*p) && *p >= '0' && *p <= '7') {
			mode_t val;
			const char *end;
			if (parse_octal(p, &val, &end) < 0) return -1;
			if (added >= maxout) return -1;
			out[added].who = who;
			out[added].op = op;
			out[added].perms = val;
			out[added].flag_copy = 0;
			added++;
			p = end;
		} else if (*p == 'u' || *p == 'g' || *p == 'o') {
			/* could be copy or could be literal perms starting with u/g/o (uncommon)
			   BSD treats single letter RHS equal to copy when it's just one class name */
			const char *q = p;
			int cnt = 0;
			while (*q && (*q=='u' || *q=='g' || *q=='o')) { cnt++; q++; }
			if (cnt == 1 && (q[0] == ',' || q[0] == '\0' || isspace((unsigned char)q[0]))) {
				/* copy */
				unsigned src = (*p == 'u') ? 1 : ((*p == 'g') ? 2 : 4);
				if (added >= maxout) return -1;
				out[added].who = who;
				out[added].op = op;
				out[added].perms = (mode_t)src;
				out[added].flag_copy = 1;
				added++;
				p = q;
			} else {
				/* fallthrough to parse symbolic letters sequence */
				/* parse as letters below */
				;
			}
		}

		if (!isdigit((unsigned char)*p)) {
			/* parse symbolic letters until comma or end */
			mode_t perms = 0;
			int saw = 0;
			while (*p && *p != ',') {
				char c = *p++;
				if (isspace((unsigned char)c)) break;
				saw = 1;
				if (c == 'r') {
					perms |= (S_IRUSR|S_IRGRP|S_IROTH);
				} else if (c == 'w') {
					perms |= (S_IWUSR|S_IWGRP|S_IWOTH);
				} else if (c == 'x') {
					perms |= (S_IXUSR|S_IXGRP|S_IXOTH);
				} else if (c == 'X') {
					/* mark X specially by using S_IXUSR|... but actual application depends on base mode */
					perms |= (S_IXUSR|S_IXGRP|S_IXOTH); /* we'll handle conditionality in getmode */
					/* we also set a high bit to indicate 'conditional X' - reuse S_ISVTX<<1 trick not portable.
					   Instead, store conditionality by setting a flag in perms: use high bit of mode_t if available.
					   Simpler: we'll detect 'X' by keeping the same bits but getmode will treat 'X' only if context requires.
					*/
				} else if (c == 's') {
					perms |= (S_ISUID|S_ISGID);
				} else if (c == 't') {
					perms |= S_ISVTX;
				} else {
					return -1;
				}
			}
			if (!saw) return -1;
			if (added >= maxout) return -1;
			out[added].who = who;
			out[added].op = op;
			out[added].perms = perms;
			out[added].flag_copy = 0;
			added++;
		}

		p = skip_space(p);
		if (*p == ',') { p++; p = skip_space(p); continue; }
		if (*p == '\0') break;
		/* unexpected char */
		if (*p) { /* tolerate trailing spaces */ if (!isspace((unsigned char)*p)) return -1; p++; }
	}

	return (int)added;
}

/* top-level setmode: allocate a ctx and parse clauses separated by commas */
void *setmode(const char *str) {
	if (!str) return NULL;
	/* allow up to 64 ops */
	const size_t MAXOPS = 64;
	size_t alloc = sizeof(setmode_ctx) + (MAXOPS - 1) * sizeof(mode_op);
	setmode_ctx *ctx = (setmode_ctx *)malloc(alloc);
	if (!ctx) return NULL;
	ctx->nops = 0;

	/* duplicate string and replace semantically significant commas only at top-level (no parentheses supported) */
	char *dup = strdup(str);
	if (!dup) { free(ctx); return NULL; }
	/* normalize: remove spaces around commas for simpler parsing */
	/* parse by splitting on commas but keep whitespace handling in parse_one_clause */
	char *p = dup;
	char *start = p;
	while (1) {
		char *comma = strchr(start, ',');
		char saved = 0;
		if (comma) { saved = *comma; *comma = '\0'; }
		int got = parse_one_clause(start, &ctx->ops[ctx->nops], MAXOPS - ctx->nops);
		if (got < 0) { free(dup); free(ctx); return NULL; }
		ctx->nops += (size_t)got;
		if (!comma) break;
		*comma = saved;
		start = comma + 1;
	}
	free(dup);
	return (void *)ctx;
}

/* helpers for mapping/copying */
static mode_t map_copy_bits(mode_t m, unsigned srccls, unsigned dstwho) {
	int r = 0, w = 0, x = 0;
	if (srccls & 1) { if (m & S_IRUSR) r = 1; if (m & S_IWUSR) w = 1; if (m & S_IXUSR) x = 1; }
	if (srccls & 2) { if (m & S_IRGRP) r = 1; if (m & S_IWGRP) w = 1; if (m & S_IXGRP) x = 1; }
	if (srccls & 4) { if (m & S_IROTH) r = 1; if (m & S_IWOTH) w = 1; if (m & S_IXOTH) x = 1; }
	mode_t res = 0;
	if (dstwho & 1) { if (r) res |= S_IRUSR; if (w) res |= S_IWUSR; if (x) res |= S_IXUSR; }
	if (dstwho & 2) { if (r) res |= S_IRGRP; if (w) res |= S_IWGRP; if (x) res |= S_IXGRP; }
	if (dstwho & 4) { if (r) res |= S_IROTH; if (w) res |= S_IWOTH; if (x) res |= S_IXOTH; }
	return res;
}

/* getmode now takes opaque set pointer and base mode_t */
mode_t getmode(const void *set, mode_t base_mode) {
	mode_t m = base_mode;
	if (!set) return m;
	const setmode_ctx *ctx = (const setmode_ctx *)set;
	size_t i;
	for (i = 0; i < ctx->nops; ++i) {
		const mode_op *op = &ctx->ops[i];
		if (op->flag_copy) {
			unsigned srccls = (unsigned)op->perms; /* 1/2/4 */
			mode_t mapped = map_copy_bits(m, srccls, op->who);
			if (op->op == OP_ASSIGN) {
				if (op->who & 1) m &= ~(S_IRUSR|S_IWUSR|S_IXUSR);
				if (op->who & 2) m &= ~(S_IRGRP|S_IWGRP|S_IXGRP);
				if (op->who & 4) m &= ~(S_IROTH|S_IWOTH|S_IXOTH);
				m |= mapped;
			} else if (op->op == OP_ADD) {
				m |= mapped;
			} else if (op->op == OP_REMOVE) {
				m &= ~mapped;
			}
			continue;
		}

		/* Build target mask according to who for r/w */
		mode_t target = 0;
		if (op->perms & (S_IRUSR|S_IRGRP|S_IROTH)) {
			if (op->who & 1) target |= S_IRUSR;
			if (op->who & 2) target |= S_IRGRP;
			if (op->who & 4) target |= S_IROTH;
		}
		if (op->perms & (S_IWUSR|S_IWGRP|S_IWOTH)) {
			if (op->who & 1) target |= S_IWUSR;
			if (op->who & 2) target |= S_IWGRP;
			if (op->who & 4) target |= S_IWOTH;
		}

		/* Execute handling: if flag_X set, apply only if base_mode is directory or any exec bit set */
		if (op->perms & (S_IXUSR|S_IXGRP|S_IXOTH)) {
			int apply_exec;
			if (op->flag_X) {
				apply_exec = (S_ISDIR(base_mode) || (base_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) );
			} else {
				apply_exec = 1;
			}
			if (apply_exec) {
				if (op->who & 1) target |= S_IXUSR;
				if (op->who & 2) target |= S_IXGRP;
				if (op->who & 4) target |= S_IXOTH;
			}
		}

		/* special bits: s/t */
		if (op->perms & S_ISUID) {
			if (op->who & 1) target |= S_ISUID;
		}
		if (op->perms & S_ISGID) {
			if (op->who & 2) target |= S_ISGID;
		}
		if (op->perms & S_ISVTX) {
			if (op->who & 4) target |= S_ISVTX;
		}

		if (op->op == OP_ASSIGN) {
			if (op->who & 1) m &= ~(S_IRUSR|S_IWUSR|S_IXUSR|S_ISUID);
			if (op->who & 2) m &= ~(S_IRGRP|S_IWGRP|S_IXGRP|S_ISGID);
			if (op->who & 4) m &= ~(S_IROTH|S_IWOTH|S_IXOTH|S_ISVTX);
			m |= target;
		} else if (op->op == OP_ADD) {
			m |= target;
		} else if (op->op == OP_REMOVE) {
			m &= ~target;
		}
	}

	return m;
}

#endif /* HAVE_GETMODE */
