#ifndef cr_err_h
#define cr_err_h

#include "cr_common.h"

void cr_print_err(const char *message, int line, const char *line_start,
                  const char* underline_start, int underline_len);

#endif
