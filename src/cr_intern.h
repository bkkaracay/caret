#ifndef cr_intern_h
#define cr_intern_h

#include "cr_map.h"
#include "cr_mem.h"
#include "cr_object.h"
#include "cr_type.h"

typedef struct CrInterner {
	CrMap string_map;
	CrArena *arena;
} CrInterner;

CrInterner cr_new_interner(CrArena *arena);
void cr_free_interner(CrInterner *intr);

const CrString *cr_intern_string(CrInterner *intr, const char *s,
                                 uint32_t len);

#endif
