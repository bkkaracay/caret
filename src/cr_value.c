#include <stdio.h>
#include <string.h>

#include "cr_value.h"

void cr_print_lit(CrLiteral lit) {
	switch(lit.type) {
		case CR_LT_EMPTY:  break;
		case CR_LT_INT128: cr_print128(lit.as.i); break;
		case CR_LT_FLOAT:  printf("%lf", lit.as.f); break;
		case CR_LT_RUNE:   printf("%"PRIu32"r", lit.as.r); break;
		case CR_LT_BOOL:
			const char *s = (lit.as.b != 0) ? "true" : "false";
			printf("%s", s);
			break;
	}
}

bool cr_string_eq(const CrHashable *h0, const CrHashable *h1) {
	const CrString *s0 = CR_AS_STRING(h0);
	const CrString *s1 = CR_AS_STRING(h1);

	if(s0->length == s1->length && h0->hash == h1->hash &&
			memcmp(s0->str, s1->str, s0->length) == 0) {
		return true;
	}

	return false;
}
