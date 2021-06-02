#include <stddef.h>

#include <ncurses.h>

#include "curse.h"
#include "vector.h"

/*
 * Put c onto w. If there is not enough space (for example, if c == '\n' and we
 * are on the last row of the window), abort. Otherwise, if ncurses line
 * wrapping would be triggered, print a newline, and then print c. In
 * practice, this means that if w has a width of win_width, lines will wrap on
 * the win_width - 1 column.
 *
 * Returns 0 normally, 1 if there was not enough space (see above), or 2 if
 * there was some other error.
 */
int wpc(WINDOW *w, char c)
{
	int win_width, win_height;
	int cur_col, cur_row;

	if (w == NULL)
		return 2;
	getmaxyx(w, win_height, win_width);
	getyx(w, cur_row, cur_col);

	/* We are on the last row and we need to move to the next */
	if (cur_row >= win_height - 1 &&
	    (c == '\n' || cur_col >= win_width - 1))
		return 1;

	/* We need to wrap */
	if (cur_col >= win_width - 1) {
		if (waddch(w, '\n') == ERR)
			return 2;
		return wpc(w, c);
	}

	/* Normal case: just print */
	if (waddch(w, c) == ERR)
		return 2;
	return 0;
}

/*
 * Dump as much as possible of v onto w. Starts from v->dat[ii]. If the
 * contents of v were not completely exhausted, stores the index of the next
 * character to print in *ei. If the contents of v were exhausted, *ei will hold
 * v->cur. This is the value that would be stored if there were another char in
 * v. No value will be stored in *ei if ei == NULL.
 *
 * Returns 0 if the contents of v were exhausted, 1 if there is still more to
 * print, or 2 if there was some other error.
 */
int wvector_char_dump(WINDOW *w, const struct vector_char *v, size_t ii,
		      size_t *ei)
{
	if (w == NULL || v == NULL)
		return 2;
	while (ii < v->cur) {
		int ret;

		ret = wpc(w, v->dat[ii]);
		if (ret == 2)
			return 2;
		if (ret == 1) {
			if (ei != NULL)
				*ei = ii;
			return 1;
		}
		ii++;
	}
	if (ei != NULL)
		*ei = ii;
	return 0;
}

/*
 * Dump as much as possible of v onto w. Starts from v->dat[ii]->dat[ij]. If
 * the contents of v were not completely exhausted, stores the indices of the
 * next character to print in *ei and *ej. If the contents of v were exhausted,
 * *ei will hold v->cur and *ej will hold 0. These are the values that would be
 * be stored if there were another vector_char in v. No value will be stored in
 * *ei if ei == NULL, and no value will be stored in *ej if ej == NULL.
 *
 * Returns 0 if the contents of v were exhausted, 1 if there is still more to
 * print, or 2 if there was some other error.
 */
int wvector_vc_dump(WINDOW *w, const struct vector_vc *v, size_t ii, size_t ij,
		    size_t *ei, size_t *ej)
{
	if (w == NULL || v == NULL)
		return 2;
	while (ii < v->cur) {
		int ret;
		size_t tmp;

		ret = wvector_char_dump(w, v->dat[ii], ij, &tmp);
		if (ret == 2)
			return 2;
		if (ret == 1) {
			if (ei != NULL)
				*ei = ii;
			if (ej != NULL)
				*ej = tmp;
			return 1;
		}
		ii++;
		ij = 0;
	}
	if (ei != NULL)
		*ei = ii;
	if (ej != NULL)
		*ej = ij;
	return 0;
}
