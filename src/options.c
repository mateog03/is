#include "options.h"
#include "builtin.h"
#include <getopt.h>

void options_init(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
			builtin_exec_str("help");
			builtin_exec_str("exit");
			break;
		default:
			builtin_exec_str("exit");
			break;
		}
	}
}
