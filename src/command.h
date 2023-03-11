#pragma once

#include <stdio.h>
#include <stdbool.h>

struct command {
	char **argv;
	size_t argc;

	FILE *fout;
	FILE *fin;
};

const char *command_name(const struct command *);

bool command_exists(const struct command *);
bool command_set_fout(struct command *, const char *, int);
bool command_set_fin(struct command *, const char *);

void command_exec(struct command *);
void command_push_arg(struct command *, char *, size_t);
void command_init(struct command *);
