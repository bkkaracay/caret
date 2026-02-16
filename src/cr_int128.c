#include <stdio.h>

#include "cr_int128.h"

static cr_int128 calc_div_rem(cr_int128 a, cr_int128 b, cr_int128 *rem_return);

static void reverse_string(char *str, size_t len) {
	for(int i = 0; i < len / 2; i++) {
		char tmp = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = tmp;
	}
}

void cr_print128(cr_int128 a) {
	if(cr_eq128(a, CR_ZERO128)) {
		printf("0");
		return;
	}

	if(cr_eq128(a, CR_INT128_MIN)) {
		printf("-170141183460469231731687303715884105728");
		return;
	}
	
	char buff[41];
	cr_int128 ten = cr_u64_to_i128(10);
	
	bool is_neg = cr_lt128(a, CR_ZERO128);
	if(is_neg)
		a = cr_neg128(a);
	
	int i = 0;
	for(; !cr_eq128(a, CR_ZERO128); i++) {
		cr_int128 rem;
		a = calc_div_rem(a, ten, &rem);

		buff[i] = cr_i128_to_u64(rem) + '0';
	}

	if(is_neg) {
		buff[i] = '-';
		i++;
	}

	buff[i] = '\0';

	reverse_string(buff, i);

	printf("%s", buff);
}

static cr_int128 long_multiply(uint64_t x, uint64_t y) {
	cr_int128 res;

	uint64_t x_lo = (uint32_t) x;
	uint64_t x_hi = x >> 32;

	uint64_t y_lo = (uint32_t) y;
	uint64_t y_hi = y >> 32;
	
	uint64_t part0 = x_lo * y_lo;
	uint64_t part1 = x_lo * y_hi;
	uint64_t part2 = x_hi * y_lo;
	uint64_t part3 = x_hi * y_hi;
	
	uint64_t mid_sum = (part0 >> 32) + (uint32_t) part1 + (uint32_t) part2;
	uint64_t sum_carry = mid_sum >> 32; 

	res.low = (uint32_t) part0 + (mid_sum << 32);
	res.high = part3 + (part1 >> 32) + (part2 >> 32) + sum_carry;

	return res;
}

cr_int128 cr_mul128(cr_int128 a, cr_int128 b) {
	cr_int128 res = long_multiply(a.low, b.low);

	res.high += a.low * b.high + a.high * b.low;

	return res;
}

static cr_int128 calc_div_rem(cr_int128 a, cr_int128 b, cr_int128 *rem_return) {
	bool is_neg_a = cr_lt128(a, CR_ZERO128);
	bool is_neg_b = cr_lt128(b, CR_ZERO128);

	bool negate_quot = is_neg_a ^ is_neg_b;

	if(is_neg_a)
		a = cr_neg128(a);

	if(is_neg_b)
		b = cr_neg128(b);

	cr_int128 quot = CR_ZERO128, rem = CR_ZERO128;

	for(int i = 0; i < 128; i++) {
		int bit = cr_rshift128(a, (127 - i)).low & 1;
		
		rem = cr_lshift128(rem, 1);
		rem.low |= bit;

		quot = cr_lshift128(quot, 1);
		
		if(cr_gte128(rem, b)) {
			rem = cr_sub128(rem, b);
			quot.low |= 1;
		}
	}
	
	if(is_neg_a)
		rem = cr_neg128(rem);

	if(rem_return != NULL)
		*rem_return = rem;


	if(negate_quot)
		return cr_neg128(quot);

	return quot;
}

cr_int128 cr_div128(cr_int128 a, cr_int128 b) {
	return calc_div_rem(a, b, NULL);
}
	
cr_int128 cr_rem128(cr_int128 a, cr_int128 b) {
	cr_int128 rem;

	calc_div_rem(a, b, &rem);

	return rem;
}
