#ifndef EFN_H
#define EFN_H

#include <stdarg.h>

void set_prog_name(const char *s);
const char *prog_name(void);
int vxprintf(int x, const char *fmt, va_list args);
int xprintf(int x, const char *fmt, ...);
void eprintf(int x, const char *fmt, ...);

#endif
