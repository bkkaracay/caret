#ifndef cr_mem_h
#define cr_mem_h

#include "cr_common.h"

typedef struct CrBlock {
	struct CrBlock *next;
	uint32_t size;
	uint32_t used;
	char data[];
} CrBlock;

typedef struct {
	CrBlock *head;
	CrBlock *tail;
} CrArena;

#define CR_ARENA_NEW(arena, type) \
	((type*) cr_arena_alloc(arena, sizeof(type)))

CrArena cr_new_arena();
void *cr_arena_alloc(CrArena *arena, uint32_t size);
void cr_free_arena(CrArena*);

void *cr_calloc(size_t num, size_t size);
void cr_free(void *p);

#endif
