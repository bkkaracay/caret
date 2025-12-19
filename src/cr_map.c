#include <string.h>

#include "cr_map.h"
#include "cr_mem.h"

#define MAX_LOAD 0.60
#define GROW_FACTOR 1.50
#define INITIAL_SIZE 8

CrMap cr_new_map(cr_map_keycmp comparer) {
	CrMap map;
	map.used = 0;
	map.capacity = 0;
	map.entries = NULL;
	map.cmp = comparer;

	return map;
}

void cr_free_map(CrMap *map) {
	if(map == NULL)
		return;

	cr_free(map->entries);
}

static CrMapEntry *find_entry(CrMap *map, const CrHashable *key) {
	uint32_t index = key->hash % map->capacity;

	CrMapEntry *tomb = NULL;
	while(true) {
		CrMapEntry *entry = map->entries + index;
		
		if(entry->key == NULL) { 
			if(entry->is_tomb) {
				tomb = entry;
			} else {
				if(tomb != NULL)
					return tomb;
				
				return entry;
			}
		} else if(entry->key == key) {
			return entry;
		}
			
		index = (index + 1) % map->capacity;
	}

	return NULL;
}

bool cr_map_get(CrMap *map, const CrHashable *key, const void **val) {
	if(map->capacity == 0)
		return false;

	CrMapEntry *entry = find_entry(map, key);
	if(entry->key == NULL)
		return false;
	
	if(val != NULL)
		*val = entry->val;

	return true;
}

static void grow_map(CrMap *map) {
	uint32_t old_cap = map->capacity;
	if(old_cap == 0)
		map->capacity = INITIAL_SIZE;
	else
		map->capacity = old_cap * GROW_FACTOR;

	CrMapEntry *old_entries = map->entries;
	map->entries = cr_calloc(map->capacity, sizeof(CrMapEntry));
	
	map->used = 0;

	for(uint32_t i = 0; i < old_cap; i++) {
		CrMapEntry old = old_entries[i];
		if(old.key == NULL)
			continue;
		
		cr_map_set(map, old.key, old.val);
	}

	cr_free(old_entries);
}


void cr_map_set(CrMap *map, const CrHashable *key, const void *val) {
	if(MAX_LOAD * map->capacity < map->used + 1)
		grow_map(map);

	CrMapEntry *entry = find_entry(map, key);
	if(entry->key == NULL) {
		entry->key = key;

		if(!entry->is_tomb)
			map->used++;
	}

	entry->val = val;
	entry->is_tomb = false;
}

bool cr_map_del(CrMap *map, const CrHashable *key) {
	if(map->capacity == 0)
		return false;

	CrMapEntry *entry = find_entry(map, key);
	if(entry->key == NULL)
		return false;

	entry->key = NULL;
	entry->is_tomb = true;

	return true;
}

const CrHashable *cr_map_find(CrMap *map, const CrHashable *key) {
	if(map->capacity == 0)
		return NULL;

	uint32_t index = key->hash % map->capacity;

	CrMapEntry *tomb = NULL;
	while(true) {
		CrMapEntry *entry = map->entries + index;
		
		if(entry->key == NULL) {
			if(!entry->is_tomb)
				return NULL;
		} else if(map->cmp(entry->key, key)) {
			return entry->key;
		}
			
		index = (index + 1) % map->capacity;
	}
}

