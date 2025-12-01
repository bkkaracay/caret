#ifndef cr_map_h
#define cr_map_h

#include "cr_mem.h"

typedef struct CrString CrString;

typedef struct {
	const CrString *key;
	const CrString *val;
	bool is_tomb;
} CrMapEntry;

typedef struct {
	int used;
	int capacity;
	CrMapEntry *entries;
} CrMap;

CrMap cr_new_map();
bool cr_map_get(CrMap *map, const CrString *key, const CrString **val);
bool cr_map_set(CrMap *map, const CrString *key, const CrString *val);
bool cr_map_del(CrMap *map, const CrString *key);
const CrString *cr_map_find_string(CrMap *map, const CrString *key);

#endif
