#include "cr_strops.h"

static bool is_digit(char c) {
	return '0' <= c && c <= '9';
}

static int char2int(char c) {
	return c - 48;
}

static const char *consume_uint(const char *start, const char *end,
                                uint64_t *val) {
	if(start == NULL || end == NULL)
		return NULL; 

	uint64_t out = 0;

	for(; start < end; start++) {
		if(!is_digit(*start))
			break;
		
		int digit = char2int(*start);
		if((UINT64_MAX - digit) / 10 < out)
			return NULL;

		out = out * 10 + digit;
	}

	if(val != NULL)
		*val = out;

	return start;
}


bool cr_str2uint(const char *start, size_t len, uint64_t *val) {
	if(start == NULL ||  len == 0 || val == NULL )
		return false;

	const char *end = start + len;
	const char *stop;

	if(stop = consume_uint(start, end, val)) {
		if(stop != end)
			return false;

		return true;
	}

	return false;
}

const double pow_table[] = {
	1.0,
	10.0,
	100.0,
	1000.0,
	10000.0,
	100000.0,
	1000000.0, 
	10000000.0, 
	100000000.0,
	1000000000.0,
	10000000000.0,
	100000000000.0,
	1000000000000.0,
	10000000000000.0,
	100000000000000.0,
};

double pow_10(uint32_t digit_count) {
	double out = 1;
	for(uint32_t i = 0; i < digit_count; i++)
		out *= 10;
	
	return out;
}

static const char *consume_as_double(const char *start, const char *end,
                                     double *val) {
	if(start == NULL || end == NULL)
		return NULL; 

	double out = 0;

	for(; start < end; start++) {
		if(!is_digit(*start))
			break;
		
		out = out * 10 + char2int(*start);
	}

	if(val != NULL)
		*val = out;

	return start;
}


bool cr_str2float(const char *start, size_t len, double *val) {
	if(start == NULL ||  len == 0 || val == NULL )
		return false;
	
	const char *end = start + len;
	const char *stop;
	
	double int_part;
	if(!(stop = consume_as_double(start, end, &int_part)))
		return false;

	*val = int_part;


	if(stop + 1 < end && stop[0] == '.') {
		start = stop + 1;
		
		double fract_part;
		if(!(stop = consume_as_double(start, end, &fract_part)))
			return false;

		if(stop != end)
			return false;


		uint32_t digit_count = (uint32_t)(end - start);
		
		if(digit_count < sizeof(pow_table)/sizeof(double))
			*val += fract_part / pow_table[digit_count];
		else 
			*val += fract_part / pow_10(digit_count);

		return true;
	}

	return false;
}
	


		



