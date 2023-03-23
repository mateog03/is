#include "job.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct job {
	pid_t pid;
	int jnum;
	bool del;

	struct job *next;
};

static struct job *jobs;

static void job_print(const struct job *);
static void job_free(struct job *);

void job_add(pid_t pid)
{
	struct job *j = xmalloc(sizeof(struct job));

	j->pid = pid;
	j->del = false;
	j->next = jobs;
	j->jnum = jobs ? jobs->jnum + 1 : 1;

	jobs = j;
	printf("[%d] %d\n", j->jnum, j->pid);
}

void job_update(void)
{
	struct job *prev, *next, *j;

	for (prev = j = jobs; j; ) {
		next = j->next;

		if (j->del) {
			if (j == jobs)
				jobs = next;
			else
				prev->next = next;

			job_print(j);
			job_free(j);

			j = next;
		} else {
			prev = j;
			j = next;
		}
	}
}

void job_maybe_remove(pid_t pid)
{
	for (struct job *j = jobs; j; j = j->next) {
		if (j->pid == pid) {
			j->del = true;
			break;
		}
	}
}

static void job_print(const struct job *j)
{
	printf("[%d] Completato\n", j->jnum);
}

static void job_free(struct job *j)
{
	free(j);
}
