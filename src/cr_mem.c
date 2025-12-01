#include <stdlib.h>

#include "cr_mem.h"

#define BLOCK_SIZE 1048510 //1MiB

union max_align {
	long long i;
	long double d;
	void *p;
	void (*f)(void);
};

static uint32_t calc_aligned(uint32_t size) {
	uint32_t aligner = sizeof(union max_align) - 1;
	return (size + aligner) & ~aligner;
}

CrArena cr_new_arena() {
	CrArena a;
	a.head = NULL;
	a.tail = NULL;

	return a;
}

static CrBlock *new_block(CrArena *ar, uint32_t aligned_size) {
	if(aligned_size < BLOCK_SIZE)
		aligned_size = BLOCK_SIZE;

	CrBlock *block = malloc(sizeof(CrBlock) + aligned_size);
	block->next = NULL;
	block->size = aligned_size;
	block->used = 0;
	
	return block;
}

static void link_block(CrArena *ar, CrBlock *block) {
	if(ar->head == NULL) {
		ar->tail = ar->head = block;
		return;
	}

	ar->head->next = block;
	ar->head = block;
}

void *cr_arena_alloc(CrArena *ar, uint32_t size) {
	uint32_t needed = calc_aligned(size);

	if(ar->head == NULL || needed > (ar->head->size - ar->head->used)) {
		link_block(ar, new_block(ar, needed));
	}
	
	void *p = ar->head->data + ar->head->used;
	ar->head->used += needed;
	return p;
}

void cr_free_arena(CrArena *ar) {
	CrBlock *iter = ar->tail;
	CrBlock *temp;

	while(iter != NULL) {
		temp = iter;
		iter = iter->next;

		free(temp);
	}

	ar->head = NULL;
	ar->tail = NULL;
}

void *cr_malloc(size_t size) {
	return malloc(size);
}

void *cr_calloc(size_t num, size_t size) {
	return calloc(num, size);
}

void cr_free(void *p) {
	return free(p);
}
