#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
	size_t prev_i, prev_j;

	size_t total, errors;
	double acc;

	int has_typed;
	time_t start, end;
	double cpm;

	int c;
	int ret;

	set_prog_name("type9k");

	/* TODO: Options processing */
	if (argc != 2)
		eprintf(0, "usage: %s FILE", prog_name());
	filename = argv[1];

	init();

	i = j = 0;
	total = errors = 0;
	has_typed = 0;
	start = -1;
	next_page(&i, &j, &prev_i, &prev_j);

	while ((c = getch()) != ERR) {
		if (!has_typed) {
			has_typed = 1;
			if ((start = time(NULL)) == -1)
				eprintf(0, "couldn't get time");
		}
		total++;
		/* If the right key was pressed */
		if (c == buf->dat[prev_i]->dat[prev_j]) {
			if (attron(A_BOLD) == ERR)
				eprintf(0, "couldn't turn attribute on");
			if (wpc(stdscr, c) == 2)
				eprintf(0, "couldn't print to window");
			if (attroff(A_BOLD) == ERR)
				eprintf(0, "couldn't turn attribute off");
			/* Increment to the next character in buf */
			if (prev_j >= buf->dat[prev_i]->cur - 1) {
				prev_i++;
				prev_j = 0;
			} else {
				prev_j++;
			}
			if (refresh() == ERR)
				eprintf(0, "couldn't refresh window");
		} else {
			errors++;
		}
		/* If we need to print the next page */
		if (prev_i > i || (prev_i == i && prev_j >= j)) {
			ret = next_page(&i, &j, &prev_i, &prev_j);
			/* If there is no page left to print */
			if (ret == 0)
				break;
		}
	}
	if (c == ERR)
		eprintf(0, "couldn't read input");
	if ((end = time(NULL)) == -1)
		eprintf(0, "couldn't get time");

	/* Floating point overflow not checked */
	cpm = total / (difftime(end, start) / 60);
	acc = (total - errors) / (double) total * 100;
	if (printw("cpm: %g\nacc: %g%%", cpm, acc) == ERR)
		eprintf(0, "couldn't print to window");
	getch();

	exit(0);
}

int next_page(size_t *i, size_t *j, size_t *prev_i, size_t *prev_j)
{
	int ret;

	*prev_i = *i;
	*prev_j = *j;
	if (clear() == ERR)
		eprintf(0, "couldn't clear window");
	ret = wvector_vc_dump(stdscr, buf, *i, *j, i, j);
	if (ret == 2)
		eprintf(0, "couldn't print to window");
	if (refresh() == ERR)
		eprintf(0, "couldn't refresh window");
	if (move(0, 0) == ERR)
		eprintf(0, "couldn't move cursor");
	return ret;
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
	/* Last one was a fluke; destroy it */
	vector_char_destroy(line);
	buf->cur--;

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
