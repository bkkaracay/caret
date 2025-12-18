#ifndef cr_checker_h
#define cr_checker_h

#include "cr_ast.h"
#include "cr_intern.h"
#include "cr_symtab.h"

typedef struct {
	CrSymTab *symtab;
	CrInterner *intr;
	bool had_err;
} CrChecker;

CrChecker cr_new_checker(CrSymTab *symtab, CrInterner *intr);

void cr_scan_decl(CrChecker *checker, CrNode *ast);
bool cr_check_ast(CrChecker *checker, CrNode *ast);

#endif
