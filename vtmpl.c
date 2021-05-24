/*
 * Generates definitions for any vector (resizable array) type.
 *
 * The files that include this one should define the following general
 * "parameters" to customize generated definitions:
 *
 * - TYPE: the actual name of the type (e.g. char *)
 * - TNAME: the identifier to be used as part of the vector type name
 *   (e.g. char_pointer, which will produce definitions for the type
 *   struct vector_char_pointer)
 *
 * The following vector-specific "parameters" should also be defined:
 *
 * - VINIT: number of slots to allocate when a vector is initialized
 * - VGROW: when out of space, multiply the number of available slots by this
 *
 * This file doesn't generate declarations; see vtmpl.h for that. Usually, you
 * will want the "parameters" (TYPE, TNAME, etc.) to be the same for both this
 * file and for vtmpl.h.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define PASTE(a, b) a ## b
#define XPASTE(a, b) PASTE(a, b)
#define RENAME(a, b) XPASTE(XPASTE(a, _), b)
#define VNAME RENAME(vector, TNAME)

/*
 * Always initialize a vector before using it.
 *
 * Returns a newly allocated vector on success, or NULL on error.
 */
struct VNAME *RENAME(VNAME, init)(void)
{
	struct VNAME *v;

	if ((v = malloc(sizeof(v[0]))) == NULL)
		return NULL;
	if (VINIT > SIZE_MAX / sizeof(v->dat[0]) ||
	    (v->dat = malloc(VINIT * sizeof(v->dat[0]))) == NULL) {
		free(v);
		return NULL;
	}
	v->cur = 0;
	v->max = VINIT;
	return v;
}

/* Returns 0 on success, or 1 on error. */
int RENAME(VNAME, add)(struct VNAME *v, TYPE new)
{
	if (v == NULL)
		return 1;
	if (v->cur >= v->max) {
		if (v->max > SIZE_MAX / sizeof(v->dat[0]) / VGROW ||
		    (v->dat = realloc(v->dat, v->max * sizeof(v->dat[0]) * VGROW)) == NULL)
		return 1;
		v->max *= VGROW;
	}
	v->dat[v->cur++] = new;
	return 0;
}

/* Always destroy a vector after using it. */
void RENAME(VNAME, destroy)(struct VNAME *v)
{
	if (v == NULL)
		return;
	free(v->dat);
	free(v);
}

#undef PASTE
#undef XPASTE
#undef RENAME
#undef VNAME
