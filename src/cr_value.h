#ifndef cr_value_h
#define cr_value_h

#include "cr_int128.h"
#include "cr_map.h"

typedef enum {
	CR_LT_EMPTY,
	CR_LT_INT128,
	CR_LT_FLOAT,
	CR_LT_RUNE,
	CR_LT_BOOL
} CrLiteralType;

typedef struct {
	CrLiteralType type;

	union {
		cr_int128 i;
		double f;
		uint32_t r;
		uint8_t b;
	} as;
} CrLiteral;

void cr_print_lit(CrLiteral lit);

typedef struct CrString {
	CrHashable h;
	size_t length;
	const char *str;
} CrString;

#define CR_AS_STRING(x) ((const CrString *) x)

bool cr_string_eq(const CrHashable *h0, const CrHashable *h1);

#endif	
