/*
 * Generates declarations for any vector (resizable array) type.
 *
 * The files that include this one should define the following general
 * "parameters" to customize generated code:
 *
 * - TYPE: the actual name of the type (e.g. char *)
 * - TNAME: the identifier to be used as part of the vector type name
 *   (e.g. char_pointer, which will produce declarations for the type
 *   struct vector_char_pointer)
 *
 * This file doesn't generate definitions; see vtmpl.c for that. Usually, you
 * will want the "parameters" (TYPE, TNAME, etc.) to be the same for both this
 * file and for vtmpl.c.
 */

#include <stddef.h>

#define PASTE(a, b) a ## b
#define XPASTE(a, b) PASTE(a, b)
#define RENAME(a, b) XPASTE(XPASTE(a, _), b)
#define VNAME RENAME(vector, TNAME)

struct VNAME {
	size_t cur;	/* Current length */
	size_t max;	/* Maximum length (usually not needed) */
	TYPE *dat;	/* Array of data */
};

struct VNAME *RENAME(VNAME, init)(void);
int RENAME(VNAME, add)(struct VNAME *v, TYPE new);
void RENAME(VNAME, destroy)(struct VNAME *v);

#undef PASTE
#undef XPASTE
#undef RENAME
#undef VNAME
