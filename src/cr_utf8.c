#include <stddef.h>

#include "cr_utf8.h"


const char *cr_utf8_decode(const char *s, uint32_t *val) {
	if(s == NULL)
		return NULL;

	uint32_t out = 0;
	unsigned char c = s[0];
	int count = 1;

	if(c < 0x80) {
		out = c;
	} else {
		while(c & 0x40) {
			if((s[count] & 0xC0) != 0x80)
				return NULL;

			out = (out << 6) | (s[count] & 0x3F);

			count++;
			c <<= 1;
		}

		if(count > 4)
			return NULL;

		out = out | ((c & 0x7F) << (count - 1) * 5);
	}
	
	if(val != NULL)
		*val = out;
	return s + count;
}
