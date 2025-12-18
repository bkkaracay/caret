#include <stdlib.h>
#include <stdio.h>

#include "cr_checker.h"
#include "cr_debug.h"
#include "cr_parser.h"

static void print_help() {
	printf("Usage: caret [file]\n");
}

static const char *read_file(const char *file_path) {
	FILE *file = fopen(file_path, "r");

	if(file == NULL) {
		fprintf(stderr, "Couldn't open the file: '%s'\n", file_path);
		exit(74);
	}
	
	if(fseek(file, 0, SEEK_END)) {
		fprintf(stderr, "Couldn't seek in the file: '%s'\n", file_path);
		exit(74);
	}

	size_t len = ftell(file);
	char *buff = cr_malloc(len);

	fseek(file, 0, SEEK_SET);

	size_t readed = fread(buff, sizeof(char), len, file);
	if(readed < len) {
		fprintf(stderr, "File '%s' was modified while reading.",
			file_path);
		cr_free(buff);
		exit(74);
	}

	fclose(file);
	return buff;
}
	

int main(int argc, char* argv[]) {
	if(argc != 2) {
		print_help();
		return 0;
	}

	const char *content = read_file(argv[1]);

	CrLexer lexer = cr_new_lexer(content);

	CrArena intr_arena = cr_new_arena();
	CrInterner intr = cr_new_interner(&intr_arena);

	CrArena ast_arena = cr_new_arena();
	CrParser parser = cr_new_parser(&lexer, &ast_arena, &intr);
	CrNode *node = cr_produce_ast(&parser);

	CrArena symtab_arena = cr_new_arena();
	CrSymTab symtab;
	cr_new_sym_tab(&symtab, &symtab_arena);

	CrChecker checker = cr_new_checker(&symtab, &intr);
	cr_scan_decl(&checker, node);
	cr_check_ast(&checker, node);

	cr_free_sym_tab(&symtab);
	cr_free_arena(&symtab_arena);

	
	cr_free((void *)content);
	cr_free_interner(&intr);
	cr_free_arena(&intr_arena);
	cr_free_arena(&ast_arena);
}
