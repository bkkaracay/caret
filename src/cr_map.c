#include <string.h>

#include "cr_map.h"
#include "cr_object.h"

#define MAX_LOAD 0.60
#define GROW_FACTOR 1.50
#define INITIAL_SIZE 8

CrMap cr_new_map() {
	CrMap map;
	map.used = 0;
	map.capacity = 0;
	map.entries = NULL;

	return map;
}

static CrMapEntry *find_entry(CrMap *map, const CrString *key) {
	int index = key->hash % map->capacity;

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

bool cr_map_get(CrMap *map, const CrString *key, const CrString **val) {
	if(map->capacity == 0)
		return false;

	CrMapEntry *entry = find_entry(map, key);
	if(entry->key == NULL)
		return false;

	*val = entry->val;
	return true;
}

static void grow_map(CrMap *map) {
	int old_cap = map->capacity;
	if(old_cap == 0)
		map->capacity = INITIAL_SIZE;
	else
		map->capacity = old_cap * GROW_FACTOR;

	CrMapEntry *old_entries = map->entries;
	map->entries = cr_calloc(map->capacity, sizeof(CrMapEntry));
	
	map->used = 0;

	for(int i = 0; i < old_cap; i++) {
		CrMapEntry old = old_entries[i];
		if(old.key == NULL)
			continue;
		
		cr_map_set(map, old.key, old.val);
	}

	cr_free(old_entries);
}


bool cr_map_set(CrMap *map, const CrString *key, const CrString *val) {
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
	return true;
}

bool cr_map_del(CrMap *map, const CrString *key) {
	if(map->capacity == 0)
		return false;

	CrMapEntry *entry = find_entry(map, key);
	if(entry->key == NULL)
		return false;

	entry->key = NULL;
	entry->is_tomb = true;

	return true;
}

static bool strings_equal(const CrString *s0, const CrString *s1) {
	return s0->length == s1->length &&
	       s0->hash == s1->hash &&
	       memcmp(s0->str, s1->str, s0->length) == 0;
}

const CrString *cr_map_find_string(CrMap *map, const CrString *key) {
	if(map->capacity == 0)
		return NULL;

	int index = key->hash % map->capacity;

	CrMapEntry *tomb = NULL;
	while(true) {
		CrMapEntry *entry = map->entries + index;
		
		if(entry->key == NULL) {
			if(!entry->is_tomb)
				return NULL;
		} else if(strings_equal(entry->key, key)) {
			return entry->key;
		}
			
		index = (index + 1) % map->capacity;
	}
}

