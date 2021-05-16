#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efn.h"

enum {
	MAX_NAME_LEN = 50
};

static char name[MAX_NAME_LEN + 1];

void set_prog_name(const char *s)
{
	if (s == NULL)
		return;
	strncpy(name, s, MAX_NAME_LEN + 1);
	name[MAX_NAME_LEN] = '\0';
}

/* Never manually modify the program name. Always use set_prog_name. */
const char *prog_name(void)
{
	return name;
}

/*
 * Just like vprintf, but also prints prog_name(). Error string printing with
 * strerror(errno) is enabled if x != 0.
 *
 * Returns 0 on success, 1 on stream error.
 */
int vxprintf(int x, const char *fmt, va_list args)
{
	fprintf(stderr, "%s: ", prog_name());
	vfprintf(stderr, fmt, args);
	if (x)
		fprintf(stderr, " %s", strerror(errno));
	fprintf(stderr, "\n");
	if (fflush(NULL) == EOF || ferror(stderr))
		return 1;
	return 0;
}

/* Warning message printing with vxprintf. Same return values as vxprintf. */
int xprintf(int x, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vxprintf(x, fmt, args);
	va_end(args);
	return ret;
}

/*
 * Error message printing with vxprintf. Exits the entire program with 1 on
 * success, 2 on stream error.
 */
void eprintf(int x, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vxprintf(x, fmt, args);
	va_end(args);
	if (ret == 1) exit(2);
	else exit(1);
}
