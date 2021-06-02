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
static int next_page(WINDOW *w, const struct vector_vc *v, size_t ii, size_t ij,
		     size_t *ei, size_t *ej);
static void wait_page(WINDOW *w, const struct vector_vc *v,
		      size_t ii, size_t ij, size_t ei, size_t ej);
void print_results(WINDOW *w);

static char *filename;
static FILE *fp;
static struct vector_vc *buf;

static int consec_space = 1;		/* If we are within a group of spaces */
static int has_typed;			/* If the user has typed something */
static time_t start, end;		/* Starting and ending times */
static size_t errors, nerrors;		/* # of errors and non-errors */

int main(int argc, char **argv)
{
	WINDOW *w;		/* The window to print to */
	size_t ei, ej;		/* Marks the end of the page */

	set_prog_name("type9k");

	if (argc != 2)
		eprintf(0, "usage: %s FILE", prog_name());
	filename = argv[1];

	init();

	ei = ej = 0;
	w = stdscr;
	for (;;) {
		size_t ii, ij;
		int ret;

		ii = ei;
		ij = ej;
		ret = next_page(w, buf, ii, ij, &ei, &ej);

		if (wmove(w, 0, 0) == ERR)
			eprintf(0, "couldn't move cursor");

		wait_page(w, buf, ii, ij, ei, ej);

		if (wclear(w) == ERR)
			eprintf(0, "couldn't clear window");

		/* We've completed the page, and it's our last */
		if (ret == 0)
			break;
	}

	if (wmove(w, 0, 0) == ERR)
		eprintf(0, "couldn't move cursor");
	if ((end = time(NULL)) == -1)
		eprintf(0, "couldn't get time");

	print_results(w);
	wgetch(w);

	exit(0);
}

void init(void)
{
	extern void init_file(void);
	extern void init_vector(void);
	extern void init_curses(void);

	struct vector_char *line;

	init_file();
	init_vector();
	init_curses();

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
}

/*
 * Print the next "page" of output to w, starting at v->dat[ii]->dat[ij].
 *
 * Same return values as wvector_vc_dump.
 */
int next_page(WINDOW *w, const struct vector_vc *v, size_t ii, size_t ij,
              size_t *ei, size_t *ej)
{
	int ret;

	ret = wvector_vc_dump(w, v, ii, ij, ei, ej);
	if (ret == 2)
		eprintf(0, "couldn't print to window");
	if (wrefresh(w) == ERR)
		eprintf(0, "couldn't refresh window");
	return ret;
}

/*
 * Wait until a page of output has been typed by the user. The page starts from
 * v->dat[ii]->dat[ij] and ends right before v->dat[ei]->dat[ej]. User input is
 * echoed to w. The user does not ever have to type in more than one whitespace
 * character in a row, nor do they have to type whitespace at the beginnings of
 * lines.
 *
 * consec_space is an external variable because it is independent of paging.
 */
void wait_page(WINDOW *w, const struct vector_vc *v, size_t ii, size_t ij,
                     size_t ei, size_t ej)
{
	extern void get_until_match(WINDOW *w, char cur);

	while (ii < ei || ij < ej) {
		int cur;

		cur = v->dat[ii]->dat[ij];

		/* If we are in a run of spaces, skip typing another space */
		if (isspace(cur) && consec_space) {
			if (wpc(w, cur) == 2)
				eprintf(0, "couldn't print to window");
		} else {
			if (isspace(cur))
				consec_space = 1;
			else
				consec_space = 0;

			get_until_match(w, cur);

			/* c is the correct character */
			if (wattron(w, A_BOLD) == ERR)
				eprintf(0, "couldn't turn attribute on");
			if (wpc(w, cur) == 2)
				eprintf(0, "couldn't print to window");
			if (wattroff(w, A_BOLD) == ERR)
				eprintf(0, "couldn't turn attribute off");
		}
		if (wrefresh(w) == ERR)
			eprintf(0, "counldn't refresh window");

		/* Increment to the next character in v */
		if (ij >= v->dat[ii]->cur - 1) {
			/* Moving to the next line */
			ii++;
			ij = 0;
			consec_space = 1;
		} else {
			ij++;
		}
	}
}

/*
 * Get input from the user until it matches cur. Any whitespace matches any
 * other whitespace. The user must correct errors with backspace.
 *
 * External variables: has_typed, start, errors, nerrors.
 */
void get_until_match(WINDOW *w, char cur) {
	int c;
	size_t consec_error;

	consec_error = 0;
	for (;;) {
		if ((c = wgetch(w)) == ERR)
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

		if (consec_error == 0 &&
		    (c == cur || (isspace(cur) && isspace(c)))) {
			nerrors++;
			break;
		}

		if (curs_set(0) == ERR)
			eprintf(0, "couldn't set cursor state");
		consec_error++;
		errors++;
	}
}

/* External variables: start, end, errors, nerrors */
void print_results(WINDOW *w)
{
	double acc, cpm;

	/* Floating point overflow not checked */
	cpm = nerrors / (difftime(end, start) / 60);
	acc = nerrors / (double) (nerrors + errors) * 100;
	if (wprintw(w, "cpm: %g\nacc: %g%%", cpm, acc) == ERR)
		eprintf(0, "couldn't print to window");
	if (wrefresh(w) == ERR)
		eprintf(0, "couldn't refresh window");
}

void init_file(void)
{
	extern void end_file(void);

	if ((fp = fopen(filename, "r")) == NULL)
		eprintf(1, "couldn't open %s", filename);
	atexit(end_file);
}

void init_vector(void)
{
	extern void end_vector(void);

	if ((buf = vector_vc_init()) == NULL)
		eprintf(0, "couldn't initialize buffer");
	atexit(end_vector);
}

void init_curses(void)
{
	extern void end_curses(void);

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
