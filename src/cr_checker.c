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


static bool is_intx(const CrType *type) {
	return CR_TK_INT8 <= type->kind && type->kind <= CR_TK_INT64;  
}

static bool is_uintx(const CrType *type) {
	return CR_TK_UINT8 <= type->kind && type->kind <= CR_TK_UINT64;  
}

static bool is_int_lit(const CrType *type) {
	return type->kind == CR_TK_INT_LIT;
}

static bool is_int_family(const CrType *type) {
	return is_int_lit(type) || is_intx(type) || is_uintx(type);
}

static bool is_floatx(const CrType *type) {
	return type->kind == CR_TK_FLOAT32 || type->kind == CR_TK_FLOAT64;
}

static bool is_float_lit(const CrType *type) {
	return type->kind == CR_TK_FLOAT_LIT;
}

static bool is_float_family(const CrType *type) {
	return is_float_lit(type) || is_floatx(type);
}

static bool is_bool(const CrType *type) { return type->kind == CR_TK_BOOL; }

static bool is_rune(const CrType *type) { return type->kind == CR_TK_RUNE; }

static bool is_err(const CrType *type) { return type->kind == CR_TK_ERR; }

static bool is_compatible(const CrType *t0, const CrType *t1) {
	if(is_err(t0) || is_err(t1))
		return true;

	if(t0 == t1)
		return true;

	if(is_int_family(t0) && is_int_family(t1))
		return is_int_lit(t0) || is_int_lit(t1);
	
	if(is_float_family(t0) && is_float_family(t1))
		return is_float_lit(t0) || is_float_lit(t1);

	return false;
}

static const CrType *unify(CrChecker *c, const CrType *t0, const CrType *t1) {
	if(is_err(t0) || is_err(t1))
		return cr_basic_type(c->intr, CR_TK_ERR);

	if(!is_compatible(t0, t1))
		return cr_basic_type(c->intr, CR_TK_ERR);
	
	if(is_int_lit(t0) || is_float_lit(t0))
		return t1;

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

static const TypeGroup group_table[] = {
	[CR_TK_INT8]  = TG_INT,
	[CR_TK_INT16] = TG_INT,
	[CR_TK_INT32] = TG_INT,
	[CR_TK_INT64] = TG_INT,
	
	[CR_TK_UINT8]  = TG_UINT,
	[CR_TK_UINT16] = TG_UINT,
	[CR_TK_UINT32] = TG_UINT,
	[CR_TK_UINT64] = TG_UINT,
	
	[CR_TK_INT_LIT]   = TG_INT_LIT,

	[CR_TK_FLOAT_LIT] = TG_FLOAT, 
	[CR_TK_FLOAT32]   = TG_FLOAT,
	[CR_TK_FLOAT64]   = TG_FLOAT,
	
	[CR_TK_RUNE] = TG_RUNE,
	[CR_TK_BOOL] = TG_BOOL,
};

static TypeGroup type2group(const CrType *type) {
	if(is_err(type))
		return TG_ERR;

	return group_table[type->kind];
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

static void fold_binary(CrNode *node) {
	cr_int128 left = node->as.binary_op.left->as.int_lit.val;
	cr_int128 right = node->as.binary_op.right->as.int_lit.val;
	
	cr_int128 result;
	switch(node->as.binary_op.tt) {
		case CR_TT_PLUS:    result = cr_add128(left, right); break;
		case CR_TT_MINUS:   result = cr_sub128(left, right); break;
		case CR_TT_STAR:    result = cr_mul128(left, right); break;
		case CR_TT_SLASH:   result = cr_div128(left, right); break;
		case CR_TT_PERCENT: result = cr_rem128(left, right); break;
	}

	node->type = CR_NT_INT_LIT;
	node->as.int_lit.val = result;
}

static void fold_unary(CrNode *node) {
	cr_int128 right = node->as.unary_op.right->as.int_lit.val;
	
	cr_int128 result;
	switch(node->as.binary_op.tt) {
		case CR_TT_MINUS: result = cr_neg128(right); break;
	}

	node->type = CR_NT_INT_LIT;
	node->as.int_lit.val = result;
}
static void check(CrChecker *c, CrNode *node);

static void binary(CrChecker *c, CrNode *node) {
	check(c, node->as.binary_op.left);
	const CrType *t_left = node->as.binary_op.left->data_type;
	
	check(c, node->as.binary_op.right);
	const CrType *t_right = node->as.binary_op.right->data_type;


	if(is_err(t_left) || is_err(t_right)) {
		node->data_type = cr_basic_type(c->intr, CR_TK_ERR);
		return;
	}

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

		result_type = cr_basic_type(c->intr, CR_TK_ERR);
	}

	node->data_type = result_type;
	
	if(is_int_lit(result_type))
		fold_binary(node);
}

static void unary(CrChecker *c, CrNode *node) {
	check(c, node->as.unary_op.right);
	const CrType *t_right = node->as.unary_op.right->data_type;

	if(!can_apply_unary(node->as.unary_op.tt, t_right)) {
		err(c, node,
			"Operator '%s' cannot be applied to"
		    		" operand of type '%s'.",
			cr_tt_spelling(node->as.unary_op.tt),
			cr_type_spelling(t_right));
		
		t_right = cr_basic_type(c->intr, CR_TK_ERR);
	}

	node->data_type = t_right;
	
	if(is_int_lit(t_right))
		fold_unary(node);
}

static void var(CrChecker *c, CrNode *node) {
	const CrSym *sym = cr_resolve_var(c->symtab, node->as.var.name);
	if(sym == NULL) {
		err(c, node, "Undefined variable");
	
		node->data_type = cr_basic_type(c->intr, CR_TK_ERR);
		return;
	}
	
	node->data_type = sym->data_type;
	node->as.var.slot = sym->slot;
	node->as.var.is_global = cr_is_global(sym);
}

static void block(CrChecker *c, CrNode *node) {
	cr_begin_scope(c->symtab);

	cr_check_ast(c, node->as.block.stmts);

	cr_end_scope(c->symtab);
}

static void if_stmt(CrChecker *c, CrNode *node) {
	check(c, node->as.if_stmt.cond);
	const CrType *t_cond = node->as.if_stmt.cond->data_type;

	if(!is_bool(t_cond)) {
		err(c, node->as.if_stmt.cond,
			"Expected type 'bool' in condition, but found '%s'.",
			cr_type_spelling(t_cond));	
	}

	check(c, node->as.if_stmt.body);
}

static void var_decl(CrChecker *c, CrNode *node) {
	const CrType *t_var = node->as.var_decl.type;
	
	if(node->as.var_decl.init != NULL) {
		check(c, node->as.var_decl.init);
		const CrType *t_init = node->as.var_decl.init->data_type;
	
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
	
	CrSymCode code = cr_define_var(c->symtab, t_var, node->as.var_decl.var);
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
	
	check(c, node->as.assign.left);
	const CrType *t_left = node->as.assign.left->data_type;

	check(c, node->as.assign.right);
	const CrType *t_right = node->as.assign.right->data_type;

	if(!is_compatible(t_left, t_right)) {
		err(c, node,
			"Type mismatch: '%s' vs '%s'.",
			cr_type_spelling(t_left),
			cr_type_spelling(t_right));
		return;
	}

	node->as.assign.right->data_type = t_left;
}

typedef void (*CheckFunc) (CrChecker *, CrNode *);

typedef struct {
	CheckFunc func;
	CrTypeKind kind;
} CheckRule;

static CheckRule rules[] = {
	[CR_NT_BINARY]    = {binary,   0},
	[CR_NT_UNARY]     = {unary,    0},
	[CR_NT_INT_LIT]   = {NULL,     CR_TK_INT_LIT},
	[CR_NT_FLOAT_LIT] = {NULL,     CR_TK_FLOAT_LIT},
	[CR_NT_RUNE_LIT]  = {NULL,     CR_TK_RUNE},
	[CR_NT_BOOL_LIT]  = {NULL,     CR_TK_BOOL},
	[CR_NT_VAR]       = {var,      0},
	[CR_NT_BLOCK]     = {block,    0},
	[CR_NT_IF]        = {if_stmt,  0}, 
	[CR_NT_VAR_DECL]  = {var_decl, 0}, 
	[CR_NT_ASSIGN]    = {assign,   0}, 
};

static void check(CrChecker *c, CrNode *node) {
	CheckRule rule = rules[node->type];
	if(rule.func == NULL)
		node->data_type = cr_basic_type(c->intr, rule.kind);
	else
		rule.func(c, node);
}

bool cr_check_ast(CrChecker *c, CrNode *root) {
	CrNode *iter = root;
	while(iter != NULL) {
		check(c, iter);
		iter = iter->next;
	}

	return !c->had_err;
}
