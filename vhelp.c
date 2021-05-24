#include "vector.h"
#include "vhelp.h"

/*
 * Read a line into v from fp, including the '\n'.
 *
 * Return 1 on error or EOF, or 0 otherwise.
 */
int read_line(struct vector_char *v, FILE *fp)
{
	int c;

	if (v == NULL || fp == NULL)
		return 1;
	while ((c = fgetc(fp)) != EOF) {
		if (vector_char_add(v, c) == 1)
			return 1;
		if (c == '\n')
			break;
	}
	return v->cur == 0;
}
