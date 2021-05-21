#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "curse.h"
#include "efn.h"
#include "type9k.h"
#include "vector.h"
#include "vhelp.h"

static char *filename;
static FILE *fp;
static struct vector_vc *buf;

int main(int argc, char **argv)
{
	size_t i, j;
	int c;
	int ret;

	set_prog_name("type9k");

	/* TODO: Options processing */
	if (argc != 2)
		eprintf(0, "usage: %s FILE", prog_name());
	filename = argv[1];

	init();

	i = j = 0;
	c = '\0';
	ret = 1;
	do {
		/* TODO: Key processing */
		if (c == 'q')
			break;

		/* If we haven't reached the end */
		if (ret != 0) {
			clear();
			ret = wvector_vc_dump(stdscr, buf, i, j, &i, &j);
			if (ret == 2)
				eprintf(0, "couldn't print to window");
			refresh();
		}
	} while ((c = getch()) != ERR);

	exit(0);
}

void init(void)
{
	struct vector_char *line;

	/* Open file */
	if ((fp = fopen(filename, "r")) == NULL)
		eprintf(1, "couldn't open %s", filename);
	atexit(end_file);

	/* Initialize vector */
	if ((buf = vector_vc_init()) == NULL)
		eprintf(0, "couldn't initialize buffer");
	atexit(end_vector);

	/* Fill vector */
	do {
		if ((line = vector_char_init()) == NULL)
			eprintf(0, "couldn't initialize line");
		if (vector_vc_add(buf, line) == 1)
			eprintf(0, "couldn't add line");
	} while (read_line(line, fp) == 0);

	/* Initialize curses */
	initscr();	/* Doesn't return */
	atexit(end_curses);
	if (cbreak() == ERR ||
	    noecho() == ERR ||
	    keypad(stdscr, TRUE) == ERR)
		eprintf(0, "couldn't initialize curses");
}

void end_file(void)
{
	if (fclose(fp) == EOF)
		xprintf(1, "couldn't close %s", filename);
}

void end_vector(void)
{
	size_t i;

	for (i = 0; i < buf->cur; i++)
		vector_char_destroy(buf->dat[i]);
	vector_vc_destroy(buf);
}

void end_curses(void)
{
	if (endwin() == ERR)
		eprintf(0, "couldn't end curses");
}
