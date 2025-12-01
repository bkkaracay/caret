#ifndef cr_lexer_h
#define cr_lexer_h

#include "cr_common.h"

typedef enum {
	CR_TT_PLUS, CR_TT_MINUS, CR_TT_STAR, CR_TT_SLASH, CR_TT_PERCENT,
	CR_TT_LBRACE, CR_TT_RBRACE,

	CR_TT_EQUAL,
	
	CR_TT_INT_LIT, CR_TT_FLOAT_LIT, CR_TT_RUNE_LIT,

	CR_TT_TRUE, CR_TT_FALSE,
	
	CR_TT_IDENTIFIER, CR_TT_IF,
	CR_TT_INT8, CR_TT_INT16, CR_TT_INT32, CR_TT_INT64,
	CR_TT_FLOAT8, CR_TT_FLOAT16, CR_TT_FLOAT32, CR_TT_FLOAT64,

	CR_TT_NEWLINE, CR_TT_ERR, CR_TT_FATAL, CR_TT_EOF
} CrTokenType;

typedef struct {
	CrTokenType type;
	const char *start;
	uint32_t length;
	uint32_t line;
	const char *line_start;
} CrToken;

typedef struct {
	const char *token_start;
	const char *current;
	const char *line_start;
	uint32_t line;
	bool had_fatal;
} CrLexer;
	
CrLexer cr_new_lexer(const char *source);
CrToken cr_scan_token(CrLexer*);

#endif
