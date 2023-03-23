#include "util.h"
#include "parser.h"
#include "builtin.h"
#include "command.h"
#include "options.h"
#include "job.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/wait.h>

static sigjmp_buf sjbuf;

static void prompt(void);
static void sigint_handler(int);
static void sigchld_handler(int);

int main(int argc, char **argv)
{
	struct command cmd;
	struct parser pa;

	const struct builtin *b;
	enum parser_status ps;

	options_init(argc, argv);

	parser_init(&pa);
	command_init(&cmd);

	signal(SIGCHLD, sigchld_handler);
	signal(SIGINT, sigint_handler);

	sigsetjmp(sjbuf, 1);

	for (;;) {
		prompt();

		do {
			ps = parser_readline(&pa, &cmd);
			if (ps == parse_fail)
				builtin_exec_str("exit");

			if (ps == parse_error || !command_name(&cmd))
				break;

			if (builtin_exists(&b, &cmd))
				builtin_exec(b, &cmd);
			else if (command_exists(&cmd))
				command_exec(&cmd);
			else
				error("%s: eh?\n", command_name(&cmd));
		} while (ps != parse_over);

		job_update();
	}

	return 0;
}

static void prompt()
{
	const char sym = getuid() == 0 ? '#' : '$';

	printf("%c ", sym);
}

static void sigint_handler(int sig)
{
	(void)sig;

	putchar('\n');
	siglongjmp(sjbuf, 0);
}

static void sigchld_handler(int sig)
{
	(void)sig;

	pid_t pid = waitpid(0, NULL, WNOHANG);

	job_maybe_remove(pid);
}
