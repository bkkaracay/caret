#ifndef cr_int128_h
#define cr_int128_h

#include "cr_common.h"

typedef struct {
	uint64_t low;
	uint64_t high;
} cr_int128;

static const cr_int128 CR_ZERO128 = {0, 0};
static const cr_int128 CR_INT128_MIN = {.high = 1ULL << 63, .low = 0};


void cr_print128(cr_int128 a);


static inline cr_int128 cr_u64_to_i128(uint64_t a) {
	return (cr_int128){.high = 0, .low = a};
}

static inline cr_int128 cr_i64_to_i128(int64_t a) {
	return (cr_int128){
		.high = -((uint64_t)a >> 63),
		.low = a
	};
}

static inline uint64_t cr_i128_to_u64(cr_int128 a) { return a.low; }
static inline int64_t cr_i128_to_i64(cr_int128 a) { return a.low; }


static inline cr_int128 cr_neg128(cr_int128 a) {
	a.low = ~a.low + 1;
	a.high = ~a.high + (uint64_t) (a.low == 0);

	return a;
}

static inline cr_int128 cr_add128(cr_int128 a, cr_int128 b) {
	cr_int128 res;

	res.low = a.low + b.low;
	res.high = a.high + b.high + (uint64_t) (res.low < a.low);

	return res;
}

static inline cr_int128 cr_sub128(cr_int128 a, cr_int128 b) {
	return cr_add128(a, cr_neg128(b));
}

cr_int128 cr_mul128(cr_int128 a, cr_int128 b);
cr_int128 cr_div128(cr_int128 a, cr_int128 b);
cr_int128 cr_rem128(cr_int128 a, cr_int128 b);

static inline cr_int128 cr_lshift128(cr_int128 a, unsigned int b) {
	b &= 127;

	if(b >= 64) {
		a.high = a.low << (b - 64);
		a.low = 0;
	} else if(b > 0) {
		a.high = a.high << b | a.low >> (64 - b);
		a.low <<= b;
	}

	return a;
}

static inline cr_int128 cr_rshift128(cr_int128 a, unsigned int b) {
	b &= 127;

	if(b >= 64) {
		a.low = a.high >> (b - 64);
		a.high = 0;
	} else if(b > 0) {
		a.low = a.low >> b | a.high << (64 - b);
		a.high >>= b;
	}

	return a;
}

static inline bool cr_eq128(cr_int128 a, cr_int128 b) {
	return a.high == b.high && a.low == b.low;
}

static inline bool cr_gt128(cr_int128 a, cr_int128 b) {
	int64_t a_hi = a.high;
	int64_t b_hi = b.high;
	
	return (a_hi > b_hi) | ((a_hi == b_hi) & (a.low > b.low));
}

static inline bool cr_gte128(cr_int128 a, cr_int128 b) {
	return cr_gt128(a, b) | cr_eq128(a, b);
}

static inline bool cr_lt128(cr_int128 a, cr_int128 b) {
	return !cr_gte128(a, b);
}

static inline bool cr_lte128(cr_int128 a, cr_int128 b) {
	return !cr_gt128(a, b);
}

#endif
