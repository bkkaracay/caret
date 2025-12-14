#ifndef cr_parser_h
#define cr_parser_h

#include "cr_ast.h"
#include "cr_intern.h"
#include "cr_lexer.h"
#include "cr_mem.h"

typedef struct {
	CrToken current;
	CrToken previous;
	CrToken next;
	CrLexer *lexer;
	CrArena *arena;
	CrInterner *intr;
	bool had_fatal;
	bool had_err;
	bool sync_mode;
} CrParser;

CrParser cr_new_parser(CrLexer *lexer, CrArena *arena, CrInterner *intr);
CrNode *cr_produce_ast(CrParser *parser);

#endif
