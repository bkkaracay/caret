#ifndef cr_object_h
#define cr_object_h

#include "cr_map.h"

typedef struct CrString {
	const char *str;
	uint32_t length;
	uint32_t hash;
} CrString;

typedef struct {
	CrMap map;
	CrArena *arena;
} CrStringPool;

CrStringPool cr_new_string_pool(CrArena *arena);
const CrString *cr_intern_string(CrStringPool *pool, const char *s,
                                 uint32_t len);

#endif	
