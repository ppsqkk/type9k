#ifndef VECTOR_H
#define VECTOR_H

#define TYPE char
#define TNAME char
#include "vtmpl.h"
#undef TYPE
#undef TNAME

#define TYPE struct vector_char *
#define TNAME vc
#include "vtmpl.h"
#undef TYPE
#undef TNAME

#endif
