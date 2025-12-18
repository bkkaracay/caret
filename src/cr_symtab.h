#ifndef cr_symtab_h
#define cr_symtab_h

#include "cr_ast.h"
#include "cr_map.h"

#define CR_MAX_LOCAL_COUNT 256

typedef enum {
	CR_SYMCODE_OK,
	CR_SYMCODE_TOO_MANY_LOCALS,
	CR_SYMCODE_REDECLERATION,
} CrSymCode;

typedef struct {
	const CrString *name;
	const CrType *data_type;
	uint32_t slot;
	int depth;
} CrSym;

typedef struct {
	CrSym locals[CR_MAX_LOCAL_COUNT];
	uint32_t local_count;
	uint32_t current_depth;

	CrMap globals;
	CrArena *global_arena;
	uint32_t global_count;
} CrSymTab;

void cr_new_sym_tab(CrSymTab *st, CrArena *global_arena);
void cr_free_sym_tab(CrSymTab*);

static inline void cr_begin_scope(CrSymTab *st) { st->current_depth++; }
void cr_end_scope(CrSymTab*);
CrSymCode cr_define_var(CrSymTab *table, const CrType *type,
                        const CrString *name);

const CrSym *cr_resolve_var(CrSymTab *table, const CrString *name);


static inline bool cr_is_global(const CrSym *sym) { return sym->depth == 0; }
static inline bool cr_in_global_scope(CrSymTab *st) {
	return st->current_depth == 0;
}

#endif
