#ifndef cr_value_h
#define cr_value_h

#include "cr_map.h"

typedef struct CrString {
	CrHashable h;
	size_t length;
	const char *str;
} CrString;

#endif	
