#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ncurses.h>

#include "curse.h"
#include "efn.h"
#include "vector.h"
#include "vhelp.h"

static void init(void);
static void end_file(void);
static void end_vector(void);
static void end_curses(void);

static char *filename;
static FILE *fp;
static struct vector_vc *buf;

int main(int argc, char **argv)
{
	size_t ei, ej;		/* Marks the end of the page */
	int consec_space;	/* Boolean: if we are in a run of spaces */

	size_t errors, nerrors;
	double acc;

	int has_typed;
	time_t start, end;
	double cpm;

	set_prog_name("type9k");

	/* TODO: Options processing */
	if (argc != 2)
		eprintf(0, "usage: %s FILE", prog_name());
	filename = argv[1];

	init();

	ei = ej = 0;
	consec_space = 1;	/* Skip leading whitespace by pretending to be in a run of spaces */
	errors = nerrors = 0;
	has_typed = 0;
	start = -1;
	for (;;) {
		size_t ii, ij;
		int ret;

		/* Display the next page */
		ii = ei;
		ij = ej;
		ret = wvector_vc_dump(stdscr, buf, ii, ij, &ei, &ej);
		if (ret == 2)
			eprintf(0, "couldn't print to window");
		if (refresh() == ERR)
			eprintf(0, "couldn't refresh window");

		if (move(0, 0) == ERR)
			eprintf(0, "couldn't move cursor");

		/* While the page hasn't been completed */
		while (ii < ei || ij < ej) {
			int cur;

			cur = buf->dat[ii]->dat[ij];

			/* If we are in a run of spaces, skip typing another space */
			if (isspace(cur) && consec_space) {
				if (wpc(stdscr, cur) == 2)
					eprintf(0, "couldn't print to window");
			} else {
				int c;
				size_t consec_error;

				if (isspace(cur))
					consec_space = 1;
				else
					consec_space = 0;

				consec_error = 0;
				for (;;) {
					if ((c = getch()) == ERR)
						eprintf(0, "couldn't read input");
					if (!has_typed) {
						has_typed = 1;
						if ((start = time(NULL)) == -1)
							eprintf(0, "couldn't get time");
					}

					if (c == KEY_BACKSPACE || c == 127) {
						if (consec_error > 0)
							consec_error--;
						if (consec_error == 0)
							if (curs_set(1) == ERR)
								eprintf(0, "couldn't set cursor state");
						continue;
					}

					if (consec_error == 0 && (c == cur || (isspace(cur) && isspace(c)))) {
						nerrors++;
						break;
					}

					if (curs_set(0) == ERR)
						eprintf(0, "couldn't set cursor state");
					consec_error++;
					errors++;
				}

				/* c is the correct character */
				if (attron(A_BOLD) == ERR)
					eprintf(0, "couldn't turn attribute on");
				if (wpc(stdscr, cur) == 2)
					eprintf(0, "couldn't print to window");
				if (attroff(A_BOLD) == ERR)
					eprintf(0, "couldn't turn attribute off");
			}
			if (refresh() == ERR)
				eprintf(0, "counldn't refresh window");

			/* Increment to the next character in buf */
			if (ij >= buf->dat[ii]->cur - 1) {
				/* Moving to the next line */
				ii++;
				ij = 0;
				consec_space = 1;
			} else {
				ij++;
			}
		}

		/* Cleanup */
		if (clear() == ERR)
			eprintf(0, "couldn't clear window");

		/* We've completed the page, and it's our last */
		if (ret == 0)
			break;
	}

	if (move(0, 0) == ERR)
		eprintf(0, "couldn't move cursor");
	if ((end = time(NULL)) == -1)
		eprintf(0, "couldn't get time");

	fprintf(stderr, "%zu %zu\n", nerrors, errors);
	/* Floating point overflow not checked */
	cpm = nerrors / (difftime(end, start) / 60);
	acc = nerrors / (double) (nerrors + errors) * 100;
	if (printw("cpm: %g\nacc: %g%%", cpm, acc) == ERR)
		eprintf(0, "couldn't print to window");
	if (refresh() == ERR)
		eprintf(0, "couldn't refresh window");
	getch();

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
