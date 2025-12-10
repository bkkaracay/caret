#include <stdio.h>

#include "cr_debug.h"

#ifdef CR_DEBUG_PRINT_TOKEN

void cr_debug_print_token(CrToken t) {
	printf("\n-------------");
	
	printf("\nType: ");
	printf(cr_tt_spelling(t.type));
	
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
	
	printf(" %s ", cr_tt_spelling(node->as.binary_op.tt)); 
	
	print_node(node->as.binary_op.right, 0);
	printf(" )");	
}

static void print_node_unary(CrNode *node) {
	printf(" %s ", cr_tt_spelling(node->as.unary_op.tt)); 

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

static void print_node_assign(CrNode *node) {
	printf("( ");	
	print_node(node->as.assign.left, 0);
	
	printf(" = "); 
	
	print_node(node->as.assign.right, 0);
	printf(" )");	
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
			print_node_assign(node);
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
