#ifndef cr_type_h
#define cr_type_h

#include "cr_map.h"

typedef struct CrInterner CrInterner;

//Warning: Reorder carefully
typedef enum {
	CR_TK_ERR,

	//==== int family ====
	CR_TK_INT_LIT,
	
	//intx
	CR_TK_INT8, CR_TK_INT16, CR_TK_INT32, CR_TK_INT64,

	//uintx
	CR_TK_UINT8, CR_TK_UINT16, CR_TK_UINT32, CR_TK_UINT64, 
	
	//==== float family ====
	CR_TK_FLOAT_LIT,
	CR_TK_FLOAT32, CR_TK_FLOAT64,
	
	//==== other ==== 
	CR_TK_RUNE, CR_TK_BOOL,
	CR_TK_USER_DEF,
} CrTypeKind;

typedef struct {
	CrHashable h;
	CrTypeKind kind;
} CrType;

const char *cr_type_spelling(const CrType *type);
const CrType *cr_basic_type(CrInterner *intr, CrTypeKind kind);

#endif
