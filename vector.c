#include "vector.h"

enum {
	VINIT = 1,
	VGROW = 2
};

#define TYPE char
#define TNAME char
#include "vtmpl.c"
#undef TYPE
#undef TNAME

#define TYPE struct vector_char *
#define TNAME vc
#include "vtmpl.c"
#undef TYPE
#undef TNAME
