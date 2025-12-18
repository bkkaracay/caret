#ifndef cr_value_h
#define cr_value_h

#include "cr_map.h"

typedef struct CrString {
	CrHashable h;
	size_t length;
	const char *str;
} CrString;

#define CR_AS_STRING(x) ((const CrString *) x)

bool cr_string_eq(const CrHashable *h0, const CrHashable *h1);

#endif	
