#ifndef cr_strops_h
#define cr_strops_h

#include "cr_common.h"

bool cr_str2uint(const char *str, size_t len, uint64_t *val);
bool cr_str2float(const char *str, size_t len, double *val);

#endif
