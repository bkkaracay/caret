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
	CrStringPool strpool = cr_new_string_pool(&str_arena);

	CrParser parser = cr_new_parser(&lexer, &arena, &strpool);
	CrNode *node = cr_produce_ast(&parser);
	
	cr_free_arena(&str_arena);
	cr_free_arena(&arena);
}
