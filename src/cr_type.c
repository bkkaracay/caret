#include "cr_type.h"
#include "cr_intern.h"

const char *cr_type_spelling(const CrType *type) {
	switch(type->kind) {
		case CR_TK_INT8:      return "int8";
		case CR_TK_INT16:     return "int16";
		case CR_TK_INT32:     return "int32";
		case CR_TK_INT64:     return "int64"; 
		case CR_TK_UINT8:     return "uint8";
		case CR_TK_UINT16:    return "uint16";
		case CR_TK_UINT32:    return "uint32";
		case CR_TK_UINT64:    return "uint64"; 
		case CR_TK_FLOAT32:   return "float32";
		case CR_TK_FLOAT64:   return "float64";
		case CR_TK_INT_LIT:   return "int literal";
		case CR_TK_FLOAT_LIT: return "float literal";
		case CR_TK_RUNE:      return "rune";
		case CR_TK_BOOL:      return "bool";
		case CR_TK_USER_DEF:  return "<TODO>";
		case CR_TK_ERR:       return "internal err";
	}

	return "invalid";
}

const CrType *cr_basic_type(CrInterner *in, CrTypeKind kind) {
	return cr_intern_type(in, (CrType) { .kind = kind });
}

