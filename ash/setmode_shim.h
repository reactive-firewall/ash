/* setmode_shim.h - Public API (drop-in)
 *
 * Provide prototypes for getmode/setmode compatible with BSD.
 *
 * To use: compile with -D_POSIX_C_SOURCE=200112L or similar where desired,
 * but the code here is careful to work in plain C89.
 */

#ifndef PORTABLE_SETMODE_H
#define PORTABLE_SETMODE_H

#ifdef __cplusplus
extern "C" {
#endif

#if __has_include(<sys/types.h>)
#include <sys/types.h> /* for mode_t */
#endif

#ifndef HAVE_GETMODE
/* If the system provides getmode/setmode, prefer those by defining HAVE_GETMODE
   and including the system headers at build time. */
mode_t getmode(const void *set, mode_t mode);
void *setmode(const char *str);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABLE_SETMODE_H */
