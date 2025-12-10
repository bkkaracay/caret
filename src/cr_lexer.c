#include <string.h>
#include <stdio.h>

#include "cr_debug.h"
#include "cr_err.h"
#include "cr_lexer.h"
#include "cr_utf8.h"

CrLexer cr_new_lexer(const char *source) {
	CrLexer l;
	l.token_start = source;
	l.current = source;
	l.line_start = source;
	l.line = 1;
	l.had_fatal = false;

	return l;
}

static void utf8_err(CrLexer *l) {
	if(!l->had_fatal) {
		l->had_fatal = true;
		cr_print_err(
			l->line,
			l->line_start, 
			l->token_start,
			(uint32_t)(l->current - l->token_start),
			"Malformed UTF-8." //TODO possible bad output
		);
	}
}

static uint32_t advance(CrLexer *l) {
	uint32_t val;

	const char *s = cr_utf8_decode(l->current, &val);
	if(s == NULL) {
		utf8_err(l);
		return '\0';
	}
	
	l->current = s;
	return val;
}

static uint32_t peek(CrLexer *l) {
	uint32_t val;
	const char *s = cr_utf8_decode(l->current, &val);
	if(s == NULL) {
		utf8_err(l);
		return '\0';
	}

	return val;
}

static bool is_at_end(CrLexer *l) {
	return peek(l) == '\0';
}

static uint32_t peek_next(CrLexer *l) {
	uint32_t val;

	const char *s = l->current;
	for(int i = 0; i < 2; i++) {
		s = cr_utf8_decode(s, &val);
		if(s == NULL) {
			utf8_err(l);
			return '\0';
		}

		if(is_at_end(l))
			return '\0';
	}

	return val;
}

static bool is_digit(uint32_t c) {
	return '0' <= c && c <= '9';
}

static bool is_alpha(uint32_t c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static bool match(CrLexer *l, uint32_t c) {
	if(peek(l) != c)
		return false;

	advance(l);
	return true;
}

static void skip_whitespace(CrLexer *l) {
	while(true) {
		uint32_t c = peek(l);
		if(c != ' ' && c != '\t')
			break;

		advance(l);
	}
}

static void increase_line(CrLexer *l) {
	l->line++;
	l->line_start = l->current;
}

static CrToken make_token(CrLexer *l, CrTokenType type) {
	CrToken t;
	
	if(l->had_fatal)
		t.type = CR_TT_FATAL;
	else
		t.type = type;

	t.start = l->token_start;
	t.length = (uint32_t)(l->current - l->token_start);
	t.line = l->line;
	t.line_start = l->line_start;

	cr_debug_print_token(t);
	return t;
}

static CrToken err_token(CrLexer *l, const char *message) {
	cr_print_err(
		l->line,
		l->line_start, 
		l->token_start,
		(uint32_t)(l->current - l->token_start),
		message
	);

	return make_token(l, CR_TT_ERR);
}

static CrToken numeric(CrLexer *l) {
	while(is_digit(peek(l)))
		advance(l);

	if(peek(l) == '.' && is_digit(peek_next(l))) {
		advance(l);

		while(is_digit(peek(l)))
			advance(l);

		return make_token(l, CR_TT_FLOAT_LIT);
	}

	return make_token(l, CR_TT_INT_LIT);
}

static CrTokenType check_keyword(CrLexer *l, int start, int len,
                                 const char *rest, CrTokenType type) {
	if((l->current - l->token_start) == (start + len) &&
			memcmp(l->token_start + start, rest, len) == 0) {
		return type;
	}

	return CR_TT_IDENTIFIER;
}

typedef struct {
	const char *rest;
	int rest_len;
	CrTokenType type;
} KeywordVariant;

static const KeywordVariant int_variants[] = {
	{"8",  1, CR_TT_INT8},
	{"16", 2, CR_TT_INT16},
	{"32", 2, CR_TT_INT32},
	{"64", 2, CR_TT_INT64}
};

static const KeywordVariant float_variants[] = {
	{"8",  1, CR_TT_FLOAT8},
	{"16", 2, CR_TT_FLOAT16},
	{"32", 2, CR_TT_FLOAT32},
	{"64", 2, CR_TT_FLOAT64}
};

static CrTokenType check_variants(CrLexer *l,
                                  int offset,
				  int len,
                                  const char *rest,
				  const KeywordVariant *variants,
				  int variant_count) {
	uint32_t token_len = l->current - l->token_start;
	int base_len = offset + len;

	if(token_len < base_len || 
			memcmp(l->token_start + offset, rest, len) != 0) {
		return CR_TT_IDENTIFIER;
	}

	for(int i = 0; i < variant_count; i++) {
		if(token_len == base_len + variants[i].rest_len) {
			if(memcmp(l->token_start + offset + len,
			          variants[i].rest,
		   	          variants[i].rest_len) == 0) {

				return variants[i].type;
			}
		}
	}

	return CR_TT_IDENTIFIER;
}

static bool check_len(CrLexer *l, uint32_t len) {
    return l->current - l->token_start == len;
}

static CrTokenType check_len_tt(CrLexer *l, uint32_t len, CrTokenType type) {
    return (l->current - l->token_start == len) ? type : CR_TT_IDENTIFIER;
}
     	
static CrTokenType identifier_type(CrLexer *l) {
	switch(l->token_start[0]) {
		case 't':
			return check_keyword(l, 1, 3, "rue", CR_TT_TRUE);
		case 'f':
			if(check_len(l, 1))
				break;

			switch(l->token_start[1]) {
				case 'a':
					return check_keyword(l, 2, 3, "lse", CR_TT_FALSE);
				case 'l':
					return check_variants(l, 2, 3, "oat", float_variants, 4);
			}
			break;
		case 'i':
			if(check_len(l, 1))
				break;

			switch(l->token_start[1]) {
				case 'f':
					return check_len_tt(l, 2, CR_TT_IF);
				case 'n':
					return check_variants(l, 2, 1, "t", int_variants, 4);
			}
			break;
	}

	return CR_TT_IDENTIFIER;
}

static CrToken identifier(CrLexer *l) {
	while(is_alpha(peek(l)) || is_digit(peek(l)) || peek(l) == '_')
		advance(l);

	return make_token(l, identifier_type(l));
}

static CrToken rune(CrLexer *l) {
	if(is_at_end(l))
		return err_token(l, "Unterminated rune.");
	
	uint32_t c = advance(l);

	if(c == '\\') {
		switch(c) {
			case '0':
			case 'n':
			case 't':
			case 'r':
			case '/':
				advance(l);
			default:
				return err_token(
					l, "Invalid rune escape sequance");
		}
	} else if(c == '\n') {
		return err_token(l, "Unexpected newline in rune.");
	}

	if(!match(l, '\'')) {
		int lines = 0;
		while(peek(l) != '\0' && !match(l, '\'')) {
			if(advance(l) == '\n')
				lines++;
		}

		CrToken err = err_token(l, "Multi-char rune literal.");
		l->line += lines;
		
		return err;
	}
	
	return make_token(l, CR_TT_RUNE_LIT);
}

CrToken cr_scan_token(CrLexer *l) {
	skip_whitespace(l);

	l->token_start = l->current;

	if(is_at_end(l))
		return make_token(l, CR_TT_EOF);

	uint32_t c = advance(l);

	if(is_digit(c))
		return numeric(l);
	if(is_alpha(c))
		return identifier(l);

	switch(c) {
		case '+':
			return make_token(l, CR_TT_PLUS);
		case '-':
			return make_token(l, CR_TT_MINUS);
		case '*':
			return make_token(l, CR_TT_STAR);
		case '\\':
			return make_token(l, CR_TT_SLASH);
		case '%':
			return make_token(l, CR_TT_PERCENT);
		case '{':
			return make_token(l, CR_TT_LBRACE);
		case '}':
			return make_token(l, CR_TT_RBRACE);
		case '=':
			return make_token(l, CR_TT_EQUAL);
		case '\'':
			return rune(l);
		case '\n':
			CrToken newline = make_token(l, CR_TT_NEWLINE);
			increase_line(l);
			return newline;
		case '\0':
			return make_token(l, CR_TT_EOF);
		default:
			return err_token(l, "Invalid char.");
	}
}
