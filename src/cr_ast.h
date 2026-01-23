#ifndef cr_ast_h
#define cr_ast_h

#include "cr_lexer.h"
#include "cr_value.h"
#include "cr_type.h"

typedef enum {
	CR_NT_BINARY, CR_NT_UNARY,
	
	//TODO Merge literal types into single type
	CR_NT_INT_LIT, CR_NT_FLOAT_LIT, CR_NT_RUNE_LIT, CR_NT_BOOL_LIT,
	CR_NT_VAR,
	CR_NT_BLOCK,
	CR_NT_IF, CR_NT_VAR_DECL,
	CR_NT_ASSIGN,
} CrNodeType;

typedef struct CrNode {
	CrNodeType type;
	const CrType *data_type; //Filled by CrChecker 
	
	struct CrNode *next;
	
	const char *start;
	uint32_t length;
	uint32_t line;
	const char *line_start;

	union {
		struct { uint64_t val; } int_lit;
		struct { double val; } float_lit;
		struct { uint32_t val; } rune_lit;
		struct { uint8_t val; } bool_lit;

		struct {
			CrTokenType tt;
			struct CrNode *right; 
		} unary_op;

		struct {
			CrTokenType tt;
			struct CrNode *left;
			struct CrNode *right;
		} binary_op;

		struct {
			const CrString *name;

			//Filled by CrChecker
			uint32_t slot;
			bool is_global;
		} var;

		struct {
			struct CrNode *stmts;
		} block;

		struct {
			struct CrNode *cond;
			struct CrNode *body;
		} if_stmt;

		struct {
			const CrType *type;
			const CrString *var;
			struct CrNode *init;
		} var_decl;

		struct {
			struct CrNode *left;
			struct CrNode *right;
		} assign;
	} as;
} CrNode;

#endif
