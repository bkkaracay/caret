#include "cr_mem.h"
#include "cr_symtab.h"

void cr_new_sym_tab(CrSymTab *st, CrArena *global_arena) {
	st->current_depth = 0;
	st->local_count = 0;

	st->globals = cr_new_map(cr_string_eq);
	st->global_count = 0;
	st->global_arena = global_arena;
}

void cr_free_sym_tab(CrSymTab *st) {
	if(st == NULL)
		return;

	cr_free_map(&st->globals);
}

void cr_end_scope(CrSymTab *st) {
	st->current_depth--;
	
	uint32_t i = st->local_count;
	while(i-- > 0) {
		if(st->locals[i].depth <= st->current_depth)
			break;

		st->local_count--;
	}
}

static CrSym *new_global(CrSymTab *st, const CrType *data_type,
                         const CrString *name) {
	CrSym *sym = CR_ARENA_NEW(st->global_arena, CrSym);

	sym->name = name;
	sym->data_type = data_type;
	sym->slot = st->global_count++;

	return sym;
}

static CrSymCode define_global(CrSymTab *st, const CrType *data_type,
                               const CrString *name) {
	if(cr_map_get(&st->globals, CR_AS_HASHABLE(name), NULL))
		return CR_SYMCODE_REDECLERATION;
	
	CrSym *global = new_global(st, data_type, name);
	cr_map_set(&st->globals, CR_AS_HASHABLE(name), global);
	
	return CR_SYMCODE_OK;
}

static CrSym new_local(CrSymTab *st, const CrType *data_type,
                         const CrString *name) {
	CrSym sym;
	
	sym.name = name;
	sym.data_type = data_type;
	sym.depth = st->current_depth;
	sym.slot = st->local_count;

	return sym;
}

static CrSymCode define_local(CrSymTab *st, const CrType *data_type,
                              const CrString *name) {
	uint32_t i = st->local_count;
	while(i-- > 0) {
		if(st->locals[i].depth == st->current_depth &&
				st->locals[i].name == name) {
			return CR_SYMCODE_REDECLERATION;
		}
	}

	if(st->local_count + 1 > CR_MAX_LOCAL_COUNT)
		return CR_SYMCODE_TOO_MANY_LOCALS;

	st->locals[st->local_count++] = new_local(st, data_type, name);

	return CR_SYMCODE_OK;
}

CrSymCode cr_define_var(CrSymTab *st, const CrType *data_type,
                        const CrString *name) {
	if(st->current_depth == 0)
		return define_global(st, data_type, name);

	return define_local(st, data_type, name);
}

const CrSym *cr_resolve_var(CrSymTab *st, const CrString *name) {
	uint32_t i = st->local_count;
	while(i-- > 0) {
		if(st->locals[i].name == name)
			return &st->locals[i];
	}

	const CrSym *global;
	if(!cr_map_get(&st->globals, CR_AS_HASHABLE(name), 
			CR_AS_VOID_PP(&global))) {
		return NULL;
	}

	return global;
}

