#include <string.h>

#include "cr_value.h"

bool cr_string_eq(const CrHashable *h0, const CrHashable *h1) {
	const CrString *s0 = CR_AS_STRING(h0);
	const CrString *s1 = CR_AS_STRING(h1);

	if(s0->length == s1->length && h0->hash == h1->hash &&
			memcmp(s0->str, s1->str, s0->length) == 0) {
		return true;
	}

	return false;
}
