#pragma once

#include <stdbool.h>
#include "command.h"

struct builtin {
	const char *name;
	void (*callback)(struct command *);
};

bool builtin_exists(const struct builtin **, const struct command *);
void builtin_exec(const struct builtin *, struct command *);
void builtin_exec_str(const char *);
