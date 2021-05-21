#ifndef CURSE_H
#define CURSE_H

#include <stddef.h>

#include <ncurses.h>

#include "vector.h"

int wpc(WINDOW *w, char c);
int wvector_vc_dump(WINDOW *w, const struct vector_vc *v, size_t ii, size_t ij,
		    size_t *ei, size_t *ej);

#endif
