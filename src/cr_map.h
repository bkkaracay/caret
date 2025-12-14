#ifndef cr_map_h
#define cr_map_h

#include "cr_common.h"

typedef struct {
	uint32_t hash;
} CrHashable;

typedef struct {
	const CrHashable *key;
	const CrHashable *val;
	bool is_tomb;
} CrMapEntry;

typedef bool (*cr_map_keycmp)(const CrHashable*, const CrHashable*);

typedef struct {
	int used;
	int capacity;
	CrMapEntry *entries;
	cr_map_keycmp cmp;
} CrMap;

#define CR_AS_HASHABLE(x) ((const CrHashable *) x)

CrMap cr_new_map(cr_map_keycmp comparer_fn);
void cr_free_map(CrMap *map);

bool cr_map_get(CrMap *map, const CrHashable *key, const void **val);
void cr_map_set(CrMap *map, const CrHashable *key, const void *val);
bool cr_map_del(CrMap *map, const CrHashable *key);
const CrHashable *cr_map_find(CrMap *map, const CrHashable *key);

#endif
