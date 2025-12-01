#include <stdio.h>

#include "cr_debug.h"

#ifdef CR_DEBUG_PRINT_TOKEN

static void print_tt(CrTokenType tt) {
	char *str;

	switch(tt) {
		case CR_TT_PLUS:       str = "CR_TT_PLUS"; break;
		case CR_TT_MINUS:      str = "CR_TT_MINUS"; break;
		case CR_TT_STAR:       str = "CR_TT_STAR"; break;
		case CR_TT_SLASH:      str = "CR_TT_SLASH"; break;
		case CR_TT_PERCENT:    str = "CR_TT_PERCENT"; break;
		case CR_TT_LBRACE:     str = "CR_TT_LBRACE"; break;
		case CR_TT_RBRACE:     str = "CR_TT_RBRACE"; break;
		case CR_TT_EQUAL:     str = "CR_TT_EQUAL"; break;
		case CR_TT_INT_LIT:    str = "CR_TT_INT_LIT"; break;
		case CR_TT_FLOAT_LIT:  str = "CR_TT_FLOAT_LIT"; break;
		case CR_TT_RUNE_LIT:   str = "CR_TT_RUNE_LIT"; break;
		case CR_TT_TRUE:       str = "CR_TT_TRUE"; break;
		case CR_TT_FALSE:      str = "CR_TT_FALSE"; break;
		case CR_TT_IDENTIFIER: str = "CR_TT_IDENTIFIER"; break;
		case CR_TT_IF:         str = "CR_TT_IF"; break;
		case CR_TT_INT8:       str = "CR_TT_INT8"; break;
		case CR_TT_INT16:      str = "CR_TT_INT16"; break;
		case CR_TT_INT32:      str = "CR_TT_INT32"; break;
		case CR_TT_INT64:      str = "CR_TT_INT64"; break;
		case CR_TT_FLOAT8:     str = "CR_TT_FLOAT8"; break;
		case CR_TT_FLOAT16:    str = "CR_TT_FLOAT16"; break;
		case CR_TT_FLOAT32:    str = "CR_TT_FLOAT32"; break;
		case CR_TT_FLOAT64:    str = "CR_TT_FLOAT64"; break;
		case CR_TT_NEWLINE:    str = "CR_TT_NEWLINE"; break;
		case CR_TT_ERR:        str = "CR_TT_ERR"; break;
		case CR_TT_FATAL:      str = "CR_TT_FATAL"; break;
		case CR_TT_EOF:        str = "CR_TT_EOF"; break;
		default:               str = "Invalid"; break;
	}

	printf(str);
}

void cr_debug_print_token(CrToken t) {
	printf("\n-------------");
	
	printf("\nType: ");
	print_tt(t.type);
	
	printf("\nText: %.*s", t.length, t.start);
	printf("\nLine: %"PRIu32, t.line);
	printf("\nLine Start: %p", t.line_start);

	printf("\n-------------\n");
}

#endif

#ifdef CR_DEBUG_PRINT_AST

static void print_node(CrNode *node, int in);

static void print_indent(int intent) {
	for(int i = 0; i < intent; i++)
		printf("    ");
}

static void print_node_binary(CrNode *node) {
	printf("( ");	
	print_node(node->as.binary_op.left, 0);

	printf(" %.*s ", node->token.length, node->token.start);
	
	print_node(node->as.binary_op.right, 0);
	printf(" )");	
}

static void print_node_unary(CrNode *node) {
	printf("( ");	
	printf("%.*s ", node->token.length, node->token.start);

	print_node(node->as.unary_op.right, 0);
	printf(" )");
}

static void print_type(CrType *type) {
	char *str;
	switch(type->kind) {
		case CR_TK_INT8:     str = "int8"; break;
		case CR_TK_INT16:    str = "int16"; break;
		case CR_TK_INT32:    str = "int32"; break;
		case CR_TK_INT64:    str = "int64"; break; 
		case CR_TK_FLOAT8:   str = "float8"; break;
		case CR_TK_FLOAT16:  str = "float16"; break;
		case CR_TK_FLOAT32:  str = "float32"; break;
		case CR_TK_FLOAT64:  str = "float64"; break;
		case CR_TK_USER_DEF: str = "<TODO>"; break;
		default: str = "Invalid";
	}
	
	printf(str);
}

static void print_node_var_decl(CrNode *node) {
	printf("(");
	print_type(node->as.var_decl.type);
	
	const CrString *var_name = node->as.var_decl.var;
	printf(" %.*s ", var_name->length, var_name->str);

	print_node(node->as.var_decl.init, 0);
	printf(")");
}


static void print_node_block(CrNode *node, int in) {
	printf("(block");

	CrNode *iter = node->as.block.stmts;
	while(iter != NULL) {
		printf("\n");
		print_node(iter, in + 1);

		iter = iter->next;
	}

	printf(")");
}

static void print_node_if(CrNode *node, int in) {
	printf("(if\n");
	
	print_node(node->as.if_stmt.cond, in + 1);
	printf("\n");
	print_node(node->as.if_stmt.body, in + 1);
	
	printf(")");
}

static void print_node(CrNode *node, int in) {
	print_indent(in);

	if(node == NULL) {
		printf("NULL");
		return;
	}

	switch(node->type) {
		case CR_NT_BINARY:
			print_node_binary(node); 
			break;
		case CR_NT_UNARY:
			print_node_unary(node);
			break;
		case CR_NT_INT_LIT:
			printf("%"PRId64, node->as.int_lit.val);
			break;
		case CR_NT_FLOAT_LIT:
			printf("%lf", node->as.float_lit.val);
			break;
		case CR_NT_RUNE_LIT:
			printf("%"PRIu32"r", node->as.rune_lit.val);
			break;
		case CR_NT_BOOL_LIT:
			char *s = (node->as.bool_lit.val != 0) ? 
				"true" : "false";
			printf(s);
			break;
		case CR_NT_VAR:
			const CrString *str = node->as.var.name;
			printf("%.*s", str->length, str->str);
			break;
		case CR_NT_BLOCK:
			print_node_block(node, in);
			break;
		case CR_NT_IF:
			print_node_if(node, in);
			break;
		case CR_NT_VAR_DECL:
			print_node_var_decl(node);
			break;
		case CR_NT_ASSIGN:
			print_node_binary(node);
			break;
		default:
			printf("Invalid");
	}
}

void cr_debug_print_ast(CrNode *node) {
	CrNode *iter = node;
	while(iter != NULL) {
		printf("\n");
		print_node(iter, 0);
		
		iter = iter->next;
		printf("\n");
	}
}

#endif
