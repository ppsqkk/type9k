#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "efn.h"
#include "type9k.h"
#include "vector.h"

static char *filename;
static FILE *fp;
static struct vector_vc *v;

int main(int argc, char **argv)
{
	set_prog_name("type9k");

	/* Options processing */
	if (argc != 2)
		eprintf(0, "usage: %s [FILE]", prog_name());
	filename = argv[1];

	init();

	exit(0);
}

void init(void)
{
	/* Open file */
	if ((fp = fopen(filename, "r")) == NULL)
		eprintf(1, "couldn't open %s", filename);
	atexit(end_file);

	/* Initialize vector */
	if ((v = vector_vc_init()) == NULL)
		eprintf(0, "couldn't initialize vector");
	atexit(end_vector);
}

void end_file(void)
{
	if (fclose(fp) == EOF)
		xprintf(1, "couldn't close %s", filename);
}

void end_vector(void)
{
	vector_vc_destroy(v);
}
