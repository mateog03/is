#include "util.h"
#include "command.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

static bool command_is_executable(const char *);

void command_exec(struct command *cmd)
{
	pid_t pid = fork();

	if (pid == 0) {
		if (cmd->fin) {
			dup2(fileno(cmd->fin), STDIN_FILENO);
			fclose(cmd->fin);
		}

		if (cmd->fout) {
			dup2(fileno(cmd->fout), STDOUT_FILENO);
			fclose(cmd->fout);
		}

		if (execvp(command_name(cmd), cmd->argv) < 0) {
			error("%s: impossible eseguire il programma\n", command_name(cmd));
			exit(EXIT_FAILURE);
		}
	} else if (pid > 0) {
		waitpid(pid, NULL, 0);

		if (cmd->fin)
			fclose(cmd->fin);

		if (cmd->fout)
			fclose(cmd->fout);
	} else
		error("impossibile creare un processo\n");
}

const char *command_name(const struct command *cmd)
{
	return cmd->argv[0];
}

bool command_exists(const struct command *cmd)
{
	char buf[PATHSIZ], path[PATHSIZ];

	if (strchr(command_name(cmd), '/'))
		return command_is_executable(command_name(cmd));

	strncpy(path, getenv("PATH"), sizeof(path));
	for (char *tok = strtok(path, ":"); tok; tok = strtok(NULL, ":")) {
		snprintf(buf, PATHSIZ, "%s/%s", tok, command_name(cmd));

		if (command_is_executable(buf))
			return true;
	}

	return false;
}

bool command_set_fout(struct command *cmd, const char *filename, int append)
{
	if (cmd->fout)
		fclose(cmd->fout);

	cmd->fout = fopen(filename, append ? "a" : "w");
	return cmd->fout != NULL;
}

bool command_set_fin(struct command *cmd, const char *filename)
{
	if (cmd->fin)
		fclose(cmd->fin);

	cmd->fin = fopen(filename, "r");
	return cmd->fin != NULL;
}

void command_init(struct command *cmd)
{
	cmd->argc = ALLOCSIZ;
	cmd->argv = xmalloc(sizeof(char *) * cmd->argc);
}

void command_push_arg(struct command *cmd, char *s, size_t i)
{
	if (i >= cmd->argc) {
		cmd->argc *= 2;
		cmd->argv = xrealloc(cmd->argv, sizeof(char *) * cmd->argc);
	}

	cmd->argv[i] = s;
}

static bool command_is_executable(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && (st.st_mode & S_IXUSR);
}
