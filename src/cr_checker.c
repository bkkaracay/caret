#include "cr_checker.h"
#include "cr_debug.h"
#include "cr_err.h"

CrChecker cr_new_checker(CrSymTab *symtab, CrInterner *intr) {
	CrChecker c;
	c.symtab = symtab;
	c.intr = intr;
	c.had_err = false;

	return c;
}

static void err(CrChecker *c, CrNode *node, const char *msg_fmt, ...) {
	c->had_err = true;
	
	va_list args;
	va_start(args, msg_fmt);

	cr_print_err_valist(
		node->line,
		node->line_start,
		node->start,
		node->length,
		msg_fmt,
		args
	);

	va_end(args);
}


static bool is_int(const CrType *type) {
	switch(type->kind) {
		case CR_TK_INT8:
		case CR_TK_INT16:
		case CR_TK_INT32:
		case CR_TK_INT64:
		case CR_TK_INT_LIT:
			return true;
	}

	return false;
}

static bool is_uint(const CrType *type) {
	switch(type->kind) {
		case CR_TK_UINT8:
		case CR_TK_UINT16:
		case CR_TK_UINT32:
		case CR_TK_UINT64:
		case CR_TK_INT_LIT:
			return true;
	}

	return false;
}

static bool is_float(const CrType *type) {
	switch(type->kind) {
		case CR_TK_FLOAT32:
		case CR_TK_FLOAT64:
			return true;
	}

	return false;
}

static bool is_numeric(const CrType *type) {
	return is_int(type) || is_float(type);
}

static bool is_bool(const CrType *type) {
	return type->kind == CR_TK_BOOL;
}

static bool is_rune(const CrType *type) {
	return type->kind == CR_TK_BOOL;
}


static bool is_err(const CrType *type) {
	return type->kind == CR_TK_ERR;
}

static bool is_compatible(const CrType *t0, const CrType *t1) {
	if(is_err(t0) || is_err(t1))
		return true;

	switch(t0->kind) {
		case CR_TK_INT8:
		case CR_TK_INT16:
		case CR_TK_INT32:
		case CR_TK_INT64:
		case CR_TK_UINT8:
		case CR_TK_UINT16:
		case CR_TK_UINT32:
		case CR_TK_UINT64:
			if(t1->kind == CR_TK_INT_LIT)
				return true;
			break;
		case CR_TK_FLOAT32:
		case CR_TK_FLOAT64:
			if(t1->kind == CR_TK_FLOAT_LIT)
				return true;
			break;
		case CR_TK_INT_LIT:
			if(is_int(t1) || is_uint(t1))
				return true;
			break;
		case CR_TK_FLOAT_LIT:
			if(is_float(t1))
				return true;
			break;
		case CR_TK_BOOL:
			if(is_bool(t1))
				return true;
			break;
		case CR_TK_RUNE:
			if(is_rune(t1))
				return true;
			break;

	}

	return t0->kind == t1->kind;
}

static const CrType *unify(CrChecker *c, const CrType *t0, const CrType *t1) {
	if(is_err(t0) || is_err(t1))
		return cr_basic_type(c->intr, CR_TK_ERR);

	if(!is_compatible(t0, t1))
		return cr_basic_type(c->intr, CR_TK_ERR);

	switch(t0->kind) {
		case CR_TK_INT_LIT:
		case CR_TK_FLOAT_LIT:
			return t1;
	}

	return t0;
}

typedef enum {
	TG_UNSUPPORTED = 0,
	TG_INT         = 1 << 0,
	TG_UINT        = 1 << 1, 
	TG_INT_LIT     = 1 << 2, 
	TG_FLOAT       = 1 << 3,
	TG_BOOL        = 1 << 4,
	TG_RUNE        = 1 << 5,
	TG_ERR         = ~0,
} TypeGroup;

static TypeGroup type2group(const CrType *type) {
	switch(type->kind) {
		case CR_TK_INT8:
		case CR_TK_INT16:
		case CR_TK_INT32:
		case CR_TK_INT64:
			return TG_INT;
		case CR_TK_UINT8:
		case CR_TK_UINT16:
		case CR_TK_UINT32:
		case CR_TK_UINT64:
			return TG_UINT;
		case CR_TK_INT_LIT:
			return TG_INT_LIT;
		case CR_TK_FLOAT32:
		case CR_TK_FLOAT64:
		case CR_TK_FLOAT_LIT:
			return TG_FLOAT;
		case CR_TK_RUNE:
			return TG_RUNE;
		case CR_TK_BOOL:
			return TG_BOOL;
		case CR_TK_ERR:
			return TG_ERR;
	}

	return TG_UNSUPPORTED;
}

static bool can_apply(TypeGroup rules[], size_t len, CrTokenType tt,
                      const CrType *type) {
	if(tt >= len)
		return false;
	
	TypeGroup group = type2group(type);

	if(rules[tt] & group)
		return true;

	return false;
}

static TypeGroup binary_rules[] = {
	[CR_TT_PLUS]    = TG_INT | TG_INT_LIT | TG_FLOAT,
	[CR_TT_MINUS]   = TG_INT | TG_INT_LIT | TG_FLOAT,
	[CR_TT_STAR]    = TG_INT | TG_INT_LIT | TG_FLOAT,
	[CR_TT_SLASH]   = TG_INT | TG_INT_LIT | TG_FLOAT,
	[CR_TT_PERCENT] = TG_UINT | TG_INT_LIT,
};

static TypeGroup unary_rules[] = {
	[CR_TT_MINUS]   = TG_INT | TG_INT_LIT | TG_FLOAT,
};

static bool can_apply_binary(CrTokenType tt, const CrType *type) {
	return can_apply(
		binary_rules, 
		sizeof(binary_rules) / sizeof(binary_rules[0]),
		tt,
		type
	);
}

static bool can_apply_unary(CrTokenType tt, const CrType *type) {
	return can_apply(
		unary_rules, 
		sizeof(unary_rules) / sizeof(unary_rules[0]),
		tt,
		type
	);
}

static void report_symcode(CrChecker *c, CrNode *node, CrSymCode code) {
	const char *msg;

	switch(code) {
		case CR_SYMCODE_OK: 
			break;
		case CR_SYMCODE_TOO_MANY_LOCALS:
			msg = "Too many local variables";
			break;
		case CR_SYMCODE_REDECLERATION:
			msg = "Variable already declared in this scope";
			break;
	}

	err(c, node, msg);
}

static void scan_var_decl(CrChecker *c, CrNode *node) {
	CrSymCode code = cr_define_var(c->symtab, node->as.var_decl.type,
	                               node->as.var_decl.var);
	if(code != CR_SYMCODE_OK) 
		report_symcode(c, node, code);
}

static void scan(CrChecker *c, CrNode *node) {
	switch(node->type) {
		case CR_NT_VAR_DECL:
			scan_var_decl(c, node);
	}
}

void cr_scan_decl(CrChecker *c, CrNode *root) {
	CrNode *iter = root;
	while(iter != NULL) {
		scan(c, iter);
		iter = iter->next;
	}
}

static const CrType *check(CrChecker *c, CrNode *node);

static const CrType *binary(CrChecker *c, CrNode *node) {
	const CrType *t_left = check(c, node->as.binary_op.left);
	const CrType *t_right = check(c, node->as.binary_op.right);

	if(is_err(t_left) || is_err(t_right))
		return cr_basic_type(c->intr, CR_TK_ERR);

	const CrType *result_type = unify(c, t_left, t_right);
	if(is_err(result_type)) {
		err(c, node, 
			"Type mismatch: '%s' vs '%s'", 
			cr_type_spelling(t_left),
			cr_type_spelling(t_right));
	}

	if(!can_apply_binary(node->as.binary_op.tt, result_type)) {
		err(c, node,
			"Operator '%s' cannot be applied to"
		    		" operands of type '%s' and '%s'.",
			cr_tt_spelling(node->as.binary_op.tt),
			cr_type_spelling(t_left),
			cr_type_spelling(t_right));

		return cr_basic_type(c->intr, CR_TK_ERR);
	}

	return result_type;
}

static const CrType *unary(CrChecker *c, CrNode *node) {
	const CrType *t_right = check(c, node->as.unary_op.right);

	if(!can_apply_unary(node->as.unary_op.tt, t_right)) {
		err(c, node,
			"Operator '%s' cannot be applied to"
		    		" operand of type '%s'.",
			cr_tt_spelling(node->as.unary_op.tt),
			cr_type_spelling(t_right));
	}

	return t_right;
}

static const CrType *var(CrChecker *c, CrNode *node) {
	const CrSym *sym = cr_resolve_var(c->symtab, node->as.var.name);
	if(sym == NULL) {
		err(c, node, "Undefined variable");
	
		return cr_basic_type(c->intr, CR_TK_ERR);
	}
	
	node->as.var.type = sym->data_type;
	node->as.var.slot = sym->slot;
	node->as.var.is_global = cr_is_global(sym);
	
	return sym->data_type;
}

static const CrType *check(CrChecker *c, CrNode *node) {
	switch(node->type) {
		case CR_NT_BINARY:
			return binary(c, node);
		case CR_NT_UNARY:
			return unary(c, node);
		case CR_NT_INT_LIT:
			return cr_basic_type(c->intr, CR_TK_INT_LIT);
		case CR_NT_FLOAT_LIT:
			return cr_basic_type(c->intr, CR_TK_FLOAT_LIT);
		case CR_NT_RUNE_LIT:
			return cr_basic_type(c->intr, CR_TK_RUNE);
		case CR_NT_BOOL_LIT:
			return cr_basic_type(c->intr, CR_TK_BOOL);
		case CR_NT_VAR:
			return var(c, node);
	}

	return cr_basic_type(c->intr, CR_TK_ERR);
}


static void analyze(CrChecker *c, CrNode *node);

static void block(CrChecker *c, CrNode *node) {
	cr_begin_scope(c->symtab);

	cr_check_ast(c, node->as.block.stmts);

	cr_end_scope(c->symtab);
}

static void if_stmt(CrChecker *c, CrNode *node) {
	const CrType *t_cond = check(c, node->as.if_stmt.cond);

	if(!is_bool(t_cond)) {
		err(c, node->as.if_stmt.cond,
			"Expected type 'bool' in condition, but found '%s'.",
			cr_type_spelling(t_cond));	
	}

	analyze(c, node->as.if_stmt.body);
}

static void var_decl(CrChecker *c, CrNode *node) {
	const CrType *t_var = node->as.var_decl.type;
	
	if(node->as.var_decl.init != NULL) {
		const CrType *t_init = check(c, node->as.var_decl.init);
	
		if(!is_compatible(t_var, t_init)) {
			err(c, node, 
				"Cannot initialize variable:"
					" expected type '%s', found '%s'.",
				cr_type_spelling(t_var),
				cr_type_spelling(t_init));
	
			t_var = cr_basic_type(c->intr, CR_TK_ERR);
		}
	}

	if(cr_in_global_scope(c->symtab))
		return;
	
	CrSymCode code = cr_define_var(c->symtab, node->as.var_decl.type,
	                               node->as.var_decl.var);
	if(code != CR_SYMCODE_OK) 
		report_symcode(c, node, code);
}

static bool is_assignable(CrNode *node) {
	switch(node->type) {
		case CR_NT_VAR:
			return true;
	}

	return false;
}

static void assign(CrChecker *c, CrNode *node) {
	if(!is_assignable(node->as.assign.left))
		err(c, node->as.assign.left, "Invalid assignment target.");

	const CrType *t_left = check(c, node->as.assign.left);
	const CrType *t_right = check(c, node->as.assign.right);

	if(!is_compatible(t_left, t_right)) {
		err(c, node,
			"Type mismatch: '%s' vs '%s'.",
			cr_type_spelling(t_left),
			cr_type_spelling(t_right));
	}
}

static void analyze(CrChecker *c, CrNode *node) {
	switch(node->type) {
		case CR_NT_BLOCK:    block(c, node); break;
		case CR_NT_IF:       if_stmt(c, node); break;
		case CR_NT_VAR_DECL: var_decl(c, node); break;
		case CR_NT_ASSIGN:   assign(c, node); break;
		default:             check(c, node);
	}
}

bool cr_check_ast(CrChecker *c, CrNode *root) {
	CrNode *iter = root;
	while(iter != NULL) {
		analyze(c, iter);
		iter = iter->next;
	}

	return !c->had_err;
}
