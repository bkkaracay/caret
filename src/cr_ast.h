#ifndef cr_ast_h
#define cr_ast_h

#include "cr_lexer.h"
#include "cr_object.h"

typedef enum {
	CR_TK_INT8, CR_TK_INT16, CR_TK_INT32, CR_TK_INT64, 
	CR_TK_FLOAT8, CR_TK_FLOAT16, CR_TK_FLOAT32, CR_TK_FLOAT64,
	CR_TK_USER_DEF,
} CrTypeKind;

typedef struct {
	CrTypeKind kind;
} CrType;

typedef enum {
	CR_NT_BINARY, CR_NT_UNARY,

	CR_NT_INT_LIT, CR_NT_FLOAT_LIT, CR_NT_RUNE_LIT, CR_NT_BOOL_LIT,
	CR_NT_VAR,
	CR_NT_BLOCK,
	CR_NT_IF, CR_NT_VAR_DECL,
	CR_NT_ASSIGN,
} CrNodeType;

typedef struct CrNode {
	CrNodeType type;
	struct CrNode *next;
	CrToken token; //TODO remove

	union {
		struct { uint64_t val; } int_lit;
		struct { double val; } float_lit;
		struct { uint32_t val; } rune_lit;
		struct { uint8_t val; } bool_lit;

		struct { 
			struct CrNode *right; 
		} unary_op;

		struct {
			struct CrNode *left;
			struct CrNode *right;
		} binary_op;

		struct {
			const CrString *name;
		} var;

		struct {
			struct CrNode *stmts;
		} block;

		struct {
			struct CrNode *cond;
			struct CrNode *body;
		} if_stmt;

		struct {
			CrType *type;
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
