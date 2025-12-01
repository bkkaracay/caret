#include <stdio.h>

#include "cr_err.h"

void cr_print_err(const char *message, int line, const char *line_start,
                  const char* underline_start, int underline_len) {
	fprintf(stderr, "[Line %d] ", line);
	fprintf(stderr, message);
	fprintf(stderr, "\n");
}
