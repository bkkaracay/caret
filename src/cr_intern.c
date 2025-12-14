#include <string.h>

#include "cr_intern.h"

#define AS_STRING(x) ((const CrString *) x)
#define AS_TYPE(x)   ((const CrType *) x)

bool compare_string(const CrHashable *h0, const CrHashable *h1) {
	const CrString *s0 = AS_STRING(h0);
	const CrString *s1 = AS_STRING(h1);

	if(s0->length == s1->length && h0->hash == h1->hash &&
			memcmp(s0->str, s1->str, s0->length) == 0) {
		return true;
	}

	return false;
}

CrInterner cr_new_interner(CrArena *arena) {
	CrInterner intr;
	intr.string_map = cr_new_map(compare_string);
	intr.arena = arena;

	return intr;
}

void cr_free_interner(CrInterner *intr) {
	if(intr == NULL)
		return;

	cr_free_map(&intr->string_map);
}

static uint32_t hash_str(const char *s, uint32_t len) {
	uint32_t hash = 2166136261;
	for(uint32_t i = 0; i < len; i++) {
		hash ^= (unsigned char) s[i];
		hash *= 16777619;
	}

	return hash;
}

static CrString *push_string(CrArena *arena, CrString s) {
	CrString *new = cr_arena_alloc(arena, sizeof(CrString) + s.length);

	new->h.hash = s.h.hash;
	new->length = s.length;
	new->str = (const char *) (new + 1);
	
	memcpy((char *) new->str, s.str, s.length);

	return new;
}

const CrString *cr_intern_string(CrInterner *in, const char *str,
                                 uint32_t length) {
	CrString temp;
	temp.str = str;
	temp.length = length;
	temp.h.hash = hash_str(str, length);

	const CrString *result = AS_STRING(cr_map_find(&in->string_map,
	                                               CR_AS_HASHABLE(&temp)));
	if(result != NULL)
		return result;

	CrString *new = push_string(in->arena, temp);

	cr_map_set(&in->string_map, CR_AS_HASHABLE(new), NULL);
	return new;
}
