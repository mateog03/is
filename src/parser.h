#pragma once

#include "command.h"

enum parser_status {
	parse_fail,
	parse_error,
	parse_ok,
	parse_over
};

struct parser {
	char *line;
	size_t lsiz;
	size_t lp;

	size_t *offsets;
	size_t osiz;
	size_t op;
};

void parser_init(struct parser *);
enum parser_status parser_readline(struct parser *, struct command *);
