#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cr_lexer.h"
#include "cr_debug.h"
#include "cr_parser.h"
#include "cr_object.h"

int main() {
	char *s = strdup(
		"hello =  2 + 4\n"
		
		"if 2 + 3 { \n"
			"2 + 4 \n"
		"} \n"

		"int8 hello = 2 + 3\n"
	);
	CrLexer lexer = cr_new_lexer(s);
	CrArena arena = cr_new_arena();

	CrArena str_arena = cr_new_arena();
	CrInterner intr = cr_new_interner(&str_arena);

	CrParser parser = cr_new_parser(&lexer, &arena, &intr);
	CrNode *node = cr_produce_ast(&parser);
	
	free(s);
	cr_free_interner(&intr);
	cr_free_arena(&str_arena);
	cr_free_arena(&arena);
}
