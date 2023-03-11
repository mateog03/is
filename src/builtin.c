#include "util.h"
#include "builtin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void builtin_cd(struct command *);
static void builtin_exit(struct command *);
static void builtin_help(struct command *);

static const struct builtin builtins[] = {
	{"cd"  , builtin_cd},
	{"exit", builtin_exit},
	{"help", builtin_help},
	{NULL  , NULL}
};

bool builtin_exists(const struct builtin **b, const struct command *cmd)
{
	*b = NULL;

	for (const struct builtin *p = builtins; p->name; p++) {
		if (strcmp(p->name, command_name(cmd)) == 0) {
			*b = p;
			break;
		}
	}

	return *b != NULL;
}

void builtin_exec(const struct builtin *b, struct command *cmd)
{
	b->callback(cmd);
}

void builtin_exec_str(const char *s)
{
	for (const struct builtin *p = builtins; p->name; p++) {
		if (strcmp(p->name, s) == 0) {
			p->callback(NULL);
			break;
		}
	}
}

static void builtin_exit(struct command *cmd)
{
	(void)cmd;

	puts("uscita");
	exit(EXIT_SUCCESS);
}

static void builtin_cd(struct command *cmd)
{
	const char *target = cmd->argv[1];

	if (!target)
		target = getenv("HOME");

	if (chdir(target) < 0)
		error("%s: directory non esistente\n", target);
}

static void builtin_help(struct command *cmd)
{
	(void)cmd;

	puts("Mateo Gjika, IS versione 0.1");
}
