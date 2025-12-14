#ifndef cr_object_h
#define cr_object_h

#include "cr_map.h"

typedef struct CrString {
	CrHashable h;
	uint32_t length;
	const char *str;
} CrString;

#endif	
