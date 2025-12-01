#ifndef cr_debug_h
#define cr_debug_h

#include "cr_common.h"

#ifdef CR_DEBUG_PRINT_TOKEN
	#include "cr_lexer.h"

	void cr_debug_print_token(CrToken t);
#else
	#define cr_debug_print_token(t) do {} while(0)
#endif

#ifdef CR_DEBUG_PRINT_AST
	#include "cr_ast.h"

	void cr_debug_print_ast(CrNode *root);
#else
	#define cr_debug_print_ast(r) do {} while(0)
#endif

#endif
