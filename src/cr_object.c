#include <string.h>

#include "cr_object.h"

static uint32_t hash_str(const char *s, uint32_t len) {
	uint32_t hash = 2166136261;
	for(uint32_t i = 0; i < len; i++) {
		hash ^= (unsigned char) s[i];
		hash *= 16777619;
	}

	return hash;
}

CrStringPool cr_new_string_pool(CrArena *arena) {
	CrStringPool pool;
	pool.map = cr_new_map();
	pool.arena = arena;

	return pool;
}

static char *copystr2arena(CrArena *arena, const char *s, uint32_t len) {
	char *new = cr_arena_alloc(arena, len);
	memcpy(new, s, len);

	return new;
}

const CrString *cr_intern_string(CrStringPool *pool, const char *str,
                                 uint32_t length) {
	CrString temp;
	temp.str = str;
	temp.length = length;
	temp.hash = hash_str(str, length);

	const CrString *result = cr_map_find_string(&pool->map, &temp);
	if(result != NULL)
		return result;
	
	CrString *new = cr_arena_alloc(pool->arena, sizeof(CrString));
	new->str = copystr2arena(pool->arena, str, length);
	new->length = length;
	new->hash = temp.hash;

	cr_map_set(&pool->map, new, NULL);
	return new;
}

