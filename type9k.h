#ifndef TYPE9K_H
#define TYPE9K_H

#include <stddef.h>

int next_page(size_t *i, size_t *j, size_t *prev_i, size_t *prev_j);
void init(void);
void end_file(void);
void end_vector(void);
void end_curses(void);

#endif
