#include <stdarg.h>
#include <stdio.h>

#include "cr_err.h"

static int calc_line_len(const char *l_start) {
	int i = 0;
	while(l_start[i] != '\n' && l_start[i] != '\0')
		i++;

	return i;
}

static void print_n_times(int n, char c) {
	for(int i = 0; i < n; i++)
		fprintf(stderr, "%c", c);
}

void cr_print_err_valist(int line, const char *l_start, const char* ul_start,
                  int ul_len, const char *fmt, va_list args) {
	fprintf(stderr, "[Line %d] ", line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	int l_len = calc_line_len(l_start);
	fprintf(stderr, " | %.*s\n", l_len, l_start);

	int space_count = (int) (ul_start - l_start);
	fprintf(stderr, " | ");
	print_n_times(space_count, ' ');

	if(l_len < ul_len)
		ul_len = l_len - space_count;

	print_n_times(ul_len, '^');
	fprintf(stderr, "\n");
}

void cr_print_err(int line, const char *l_start, const char* ul_start,
                  int ul_len, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	cr_print_err(line, l_start, ul_start, ul_len, fmt, args);

	va_end(args);
}

