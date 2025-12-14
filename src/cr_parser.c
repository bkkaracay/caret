#include "cr_ast.h"
#include "cr_debug.h"
#include "cr_err.h"
#include "cr_lexer.h"
#include "cr_parser.h"
#include "cr_utf8.h"
#include "cr_strops.h"

CrParser cr_new_parser(CrLexer *lexer, CrArena *arena, CrInterner *intr) {
	CrParser p;
	p.lexer = lexer;
	p.arena = arena;
	p.intr = intr;
	p.had_fatal = false;
	p.had_err = false;
	p.sync_mode = false;

	return p;
}

typedef enum {
	PREC_NONE,
	PREC_LOGIC_OR,
	PREC_LOGIC_AND,
	PREC_BITWISE_OR,
	PREC_BITWISE_XOR,
	PREC_BITWISE_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_SHIFT,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_POST,
	PREC_PRIMARY,
} Precedence;

typedef CrNode* (*ParseFunc)(CrParser*, CrNode *prev); 

typedef struct {
	ParseFunc prefix;
	ParseFunc infix;
	Precedence prec;
} ParseRule;

static ParseRule get_rule(CrToken t);
static CrNode *parse_precedence(CrParser *p, Precedence prec);

static void err_at(CrParser *p, CrToken token, const char *msg) {
	if(p->sync_mode)
		return;
	
	p->sync_mode = 1;
	p->had_err = 1;
	cr_print_err(
		token.line,
		token.line_start,
		token.start,
		token.length,
		msg
	);
}

static void err_at_prev(CrParser *p, const char *msg) {
	err_at(p, p->previous, msg);
}

static void err_at_curr(CrParser *p, const char *msg) {
	err_at(p, p->current, msg);
}

static void advance(CrParser *p) {
	p->previous = p->current;
	p->current = p->next;
	
	while(true) {
		p->next = cr_scan_token(p->lexer);
		if(p->next.type == CR_TT_FATAL) {
			p->had_fatal = true;
			break;
		} else if(p->next.type != CR_TT_ERR) {
			break;
		}
		
		p->had_err = 1;
		p->sync_mode = 1;
	}
}

static bool match(CrParser *p, CrTokenType type) {
	if(p->current.type != type)
		return false;

	advance(p);
	return true;
}

static void consume(CrParser *p, CrTokenType type, const char *msg) {
	if(!match(p, type)) {
		err_at_curr(p, msg);
	}
}

static void skip_newlines(CrParser *p) {
	while(p->current.type == CR_TT_NEWLINE)
		advance(p);
}

static bool is_at_end(CrParser *p) {
	if(p->current.type == CR_TT_EOF || p->current.type == CR_TT_FATAL)
		return true;

	return false;
}

static void sync(CrParser *p) {
	p->sync_mode = 0;

	while(!is_at_end(p)) {
		if(p->previous.type == CR_TT_NEWLINE) {
			switch(p->current.type) {
				case CR_TT_IF:
					return;
				case CR_TT_IDENTIFIER:
					if(p->next.type == CR_TT_IDENTIFIER)
						return;
			}
		}

		if(p->current.type == CR_TT_RBRACE) {
			return;
		}

		advance(p);
	}
}

static CrNode *new_node(CrArena *a, CrNodeType type) {
	CrNode *node = CR_ARENA_NEW(a, CrNode);
	node->type = type;
	node->next = NULL;

	return node;
}

static void set_span_t(CrNode *n, CrToken first) {
	n->start = first.start;
	n->length = first.length;
	n->line = first.line;
	n->line_start = first.line_start;
}

static void set_span_tt(CrNode *n, CrToken first, CrToken last) {
	n->start = first.start;
	n->length = (uint32_t) (last.start + last.length - first.start);
	n->line = first.line;
	n->line_start = first.line_start;
}

static void set_span_tn(CrNode *n, CrToken first, CrNode *last) {
	n->start = first.start;
	n->length = (uint32_t) (last->start + last->length - first.start);
	n->line = first.line;
	n->line_start = first.line_start;
}

static void set_span_nn(CrNode *n, CrNode *first, CrNode *last) {
	n->start = first->start;
	n->length = (uint32_t) (last->start + last->length - first->start);
	n->line = first->line;
	n->line_start = first->line_start;
}


static CrNode *unary(CrParser *p, CrNode *null_node) {
	CrNode *op = new_node(p->arena, CR_NT_UNARY);
	CrToken op_tok = p->previous;
	op->as.unary_op.tt = op_tok.type;

	skip_newlines(p);

	op->as.unary_op.right = parse_precedence(p, PREC_UNARY);

	set_span_tn(op, op_tok, op->as.unary_op.right);
	return op;
}

static CrNode *binary(CrParser *p, CrNode *prev) {
	CrNode *op = new_node(p->arena, CR_NT_BINARY);	
	op->as.binary_op.left = prev;

	op->as.binary_op.tt = p->previous.type;
	Precedence prec = get_rule(p->previous).prec + 1;

	skip_newlines(p);
	op->as.binary_op.right = parse_precedence(p, prec);

	set_span_nn(op, op->as.binary_op.left, op->as.binary_op.right);	
	return op;
}

static CrNode *int_lit(CrParser *p, CrNode *prev) {
	uint64_t val;	
	if(!cr_str2uint(p->previous.start, p->previous.length, &val)) {
		err_at_prev(p,
			"Integer literal exceeds maximum possible value.");
		return NULL;
	}

	
	CrNode *node = new_node(p->arena, CR_NT_INT_LIT);
	node->as.int_lit.val = val;

	set_span_t(node, p->previous);
	return node;
}

static CrNode *float_lit(CrParser *p, CrNode *prev) {
	double val;
	if(!cr_str2float(p->previous.start, p->previous.length, &val)) {
		err_at_prev(p, "Invalid float literal.");
	}

	CrNode *node = new_node(p->arena, CR_NT_FLOAT_LIT);
	node->as.float_lit.val = val;

	set_span_t(node, p->previous);
	return node;
}

static CrNode *rune_lit(CrParser *p, CrNode *prev) {
	uint32_t rune;
	cr_utf8_decode(p->previous.start + 1, &rune);

	CrNode *node = new_node(p->arena, CR_NT_RUNE_LIT);
	node->as.rune_lit.val = rune;

	set_span_t(node, p->previous);
	return node;
}

static CrNode *true_lit(CrParser *p, CrNode *prev) {
	CrNode *node = new_node(p->arena, CR_NT_BOOL_LIT);
	node->as.bool_lit.val = 1;

	set_span_t(node, p->previous);
	return node;
}

static CrNode *false_lit(CrParser *p, CrNode *prev) {
	CrNode *node = new_node(p->arena, CR_NT_BOOL_LIT);
	node->as.bool_lit.val = 0;

	set_span_t(node, p->previous);
	return node;
}

static CrNode *variable(CrParser *p, CrNode *prev) {
	CrNode *node = new_node(p->arena, CR_NT_VAR);
	node->as.var.name = cr_intern_string(p->intr, p->previous.start,
	                                     p->previous.length);
	
	set_span_t(node, p->previous);
	return node;
}

static ParseRule rules[] = {
	[CR_TT_PLUS]        = {NULL,       binary, PREC_TERM},
	[CR_TT_MINUS]       = {unary,      binary, PREC_TERM},
	[CR_TT_STAR]        = {NULL,       binary, PREC_FACTOR},
	[CR_TT_SLASH]       = {NULL,       binary, PREC_FACTOR},
	[CR_TT_PERCENT]     = {NULL,       binary, PREC_FACTOR},
	[CR_TT_LBRACE]      = {NULL,       NULL,   PREC_NONE},
	[CR_TT_RBRACE]      = {NULL,       NULL,   PREC_NONE},
	[CR_TT_EQUAL]       = {NULL,       NULL,   PREC_NONE},
	[CR_TT_INT_LIT]     = {int_lit,    NULL,   PREC_NONE},
	[CR_TT_FLOAT_LIT]   = {float_lit,  NULL,   PREC_NONE},
	[CR_TT_RUNE_LIT]    = {rune_lit,   NULL,   PREC_NONE},
	[CR_TT_TRUE]        = {true_lit,   NULL,   PREC_NONE},
	[CR_TT_FALSE]       = {false_lit,  NULL,   PREC_NONE},
	[CR_TT_IDENTIFIER]  = {variable,   NULL,   PREC_NONE},
	[CR_TT_INT8]        = {NULL,       NULL,   PREC_NONE},
	[CR_TT_INT16]       = {NULL,       NULL,   PREC_NONE},
	[CR_TT_INT32]       = {NULL,       NULL,   PREC_NONE},
	[CR_TT_INT64]       = {NULL,       NULL,   PREC_NONE},
	[CR_TT_FLOAT8]      = {NULL,       NULL,   PREC_NONE},
	[CR_TT_FLOAT16]     = {NULL,       NULL,   PREC_NONE},
	[CR_TT_FLOAT32]     = {NULL,       NULL,   PREC_NONE},
	[CR_TT_FLOAT64]     = {NULL,       NULL,   PREC_NONE},
	[CR_TT_NEWLINE]     = {NULL,       NULL,   PREC_NONE},
	[CR_TT_ERR]         = {NULL,       NULL,   PREC_NONE},
	[CR_TT_FATAL]       = {NULL,       NULL,   PREC_NONE},
	[CR_TT_EOF]         = {NULL,       NULL,   PREC_NONE},
};

static ParseRule get_rule(CrToken t) {
	return rules[t.type];
}

static CrNode *parse_precedence(CrParser *p, Precedence prec) {
	advance(p);

	ParseFunc prefix = get_rule(p->previous).prefix;
	if(prefix == NULL) {
		err_at_prev(p, "Expect expression.");
		return NULL;
	}
	CrNode *node = prefix(p, NULL);

	while(prec <= get_rule(p->current).prec) {
		advance(p);

		ParseFunc infix = get_rule(p->previous).infix;
		node = infix(p, node);
	}

	return node;
}

static CrNode *expression(CrParser *p) {
	return parse_precedence(p, PREC_LOGIC_OR);
}

static CrNode *statement(CrParser*);
static CrNode *block(CrParser*);

static CrNode *block(CrParser *p) {
	consume(p, CR_TT_LBRACE, "Expect '{' before block.");
	CrToken lbrace = p->previous;

	CrNode *block_node = new_node(p->arena, CR_NT_BLOCK);

	skip_newlines(p);
	
	CrNode *head = NULL;
	CrNode **iter = &head;
	while(!is_at_end(p) && p->current.type != CR_TT_RBRACE) {
		CrNode *stmt = statement(p);

		*iter = stmt;
		iter = &stmt->next;
	}
	
	block_node->as.block.stmts = head;

	consume(p, CR_TT_RBRACE, "Expect '}' after block");
	CrToken rbrace = p->previous;

	set_span_tt(block_node, lbrace, rbrace);
	return block_node;
}

static CrNode *if_stmt(CrParser *p) {
	CrNode *node = new_node(p->arena, CR_NT_IF);
	CrToken if_tok = p->previous;

	node->as.if_stmt.cond = expression(p);
	node->as.if_stmt.body = block(p);

	set_span_tn(node, if_tok, node->as.if_stmt.body);
	return node;
}

static CrType *type(CrParser *p) {
	CrTypeKind kind;
	switch(p->previous.type) {
		case CR_TT_INT8:       kind = CR_TK_INT8; break;
		case CR_TT_INT16:      kind = CR_TK_INT16; break;
		case CR_TT_INT32:      kind = CR_TK_INT32; break;
		case CR_TT_INT64:      kind = CR_TK_INT64; break;
		case CR_TT_FLOAT8:     kind = CR_TK_FLOAT8; break;
		case CR_TT_FLOAT16:    kind = CR_TK_FLOAT16; break;
		case CR_TT_FLOAT32:    kind = CR_TK_FLOAT32; break;
		case CR_TT_FLOAT64:    kind = CR_TK_FLOAT64; break;
		case CR_TT_IDENTIFIER: kind = CR_TK_USER_DEF; break;
	}

	CrType *type = CR_ARENA_NEW(p->arena, CrType); 
	type->kind = kind;

	return type;
}

static CrNode *declaration(CrParser *p) {
	CrNode *node = new_node(p->arena, CR_NT_VAR_DECL);
	CrToken first = p->previous;

	node->as.var_decl.type = type(p);
	
	advance(p);
	node->as.var_decl.var = cr_intern_string(p->intr, p->previous.start,
	                                         p->previous.length);
	CrToken var_tok = p->previous;

	node->as.var_decl.init = NULL;

	if(match(p, CR_TT_EQUAL)) {
		node->as.var_decl.init = expression(p);
		
		set_span_tn(node, first, node->as.var_decl.init);
		return node;
	}

	set_span_tt(node, first, var_tok); 
	return node;
}

static CrNode *exp_assign_stmt(CrParser *p) {
	CrNode *left = expression(p);

	if(match(p, CR_TT_EQUAL)) {
		CrNode *assign = new_node(p->arena, CR_NT_ASSIGN);
		
		assign->as.assign.left = left;
		assign->as.assign.right = expression(p);

		set_span_nn(assign, left, assign->as.assign.right); 
		return assign;
	}

	return left;
}

static CrNode *statement(CrParser *p) {
	CrNode *stmt;
	switch(p->current.type) {
		case CR_TT_IF:
			advance(p); stmt = if_stmt(p);
			break;
		case CR_TT_INT8:
		case CR_TT_INT16:
		case CR_TT_INT32:
		case CR_TT_INT64:
		case CR_TT_FLOAT8:
		case CR_TT_FLOAT16:
		case CR_TT_FLOAT32:
		case CR_TT_FLOAT64:
		case CR_TT_IDENTIFIER:
			if(p->next.type == CR_TT_IDENTIFIER) {
				advance(p);
				stmt = declaration(p);
				break;
			}

			stmt = exp_assign_stmt(p);
			break;
		case CR_TT_LBRACE:
			stmt = block(p);
			break;
		default:
			stmt = exp_assign_stmt(p);
			break;
	}

	consume(p, CR_TT_NEWLINE, "Expected newline.");

	skip_newlines(p);
	
	if(p->had_err)
		sync(p);
	
	return stmt;
}

CrNode *cr_produce_ast(CrParser *p) {
	advance(p);
	advance(p);

	CrNode *ast = NULL;
	CrNode **iter = &ast;
	while(!is_at_end(p)) {
		CrNode *stmt = statement(p);

		*iter = stmt;
		iter = &stmt->next;
	}
	
	cr_debug_print_ast(ast);

	return ast;
}
