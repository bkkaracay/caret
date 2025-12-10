#ifndef cr_err_h
#define cr_err_h

#include <stdarg.h>

#include "cr_common.h"

void cr_print_err(int line, const char *line_start, const char* underline_start,
                  int underline_len, const char *msg_fmt, ...);

void cr_print_err_valist(int line,
                         const char *line_start, 
                         const char* underline_start,
                         int underline_len, 
			 const char *msg_fmt,
			 va_list args);
#endif
