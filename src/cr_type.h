#ifndef cr_type_h
#define cr_type_h

#include "cr_common.h"

typedef enum {
	CR_TK_INT8, CR_TK_INT16, CR_TK_INT32, CR_TK_INT64, 
	CR_TK_FLOAT8, CR_TK_FLOAT16, CR_TK_FLOAT32, CR_TK_FLOAT64,
	CR_TK_INT_LIT, CR_TK_FLOAT_LIT, CR_TK_RUNE_LIT, CR_TK_BOOL_LIT,
	CR_TK_USER_DEF,

	CR_TK_ERR,
} CrTypeKind;

typedef struct {
	CrTypeKind kind;
} CrType;

const char *cr_type_spelling(CrType type);

#endif
