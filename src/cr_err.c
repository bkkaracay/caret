#include <stdarg.h>
#include <stdio.h>

#include "cr_err.h"

#define TAB_SIZE 4

static int print_replace_tabs(const char *str, int len) {
	int tab_count = 0;
	for(int i = 0; i < len; i++) {
		if(str[i] != '\t') {
			putc(str[i], stderr);
			continue;
		}

		for(int j = 0; j < TAB_SIZE; j++)
			putc(' ', stderr);

		tab_count++;
	}

	return tab_count;
}

static int calc_line_len(const char *l_start) {
	int i = 0;
	while(l_start[i] != '\n' && l_start[i] != '\0')
		i++;

	return i;
}

static int calc_space_count(const char *l_start, const char *ul_start) {
	int count = 0;

	const char *iter = l_start;
	while(iter != ul_start) {
		if(*iter == '\t')
			count += TAB_SIZE;
		else
			count++;

		iter++;
	}

	return count;
}

static void print_n_times(int n, char c) {
	for(int i = 0; i < n; i++)
		fprintf(stderr, "%c", c);
}

void cr_print_err_valist(int line, const char *l_start, const char *ul_start,
                         int ul_len, const char *fmt, va_list args) {
	fprintf(stderr, "[Line %d] ", line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	int l_len = calc_line_len(l_start);
	fprintf(stderr, " | ");
	print_replace_tabs(l_start, l_len);

	int space_count = calc_space_count(l_start, ul_start);
	fprintf(stderr, "\n | ");
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

	cr_print_err_valist(line, l_start, ul_start, ul_len, fmt, args);

	va_end(args);
}

