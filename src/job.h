#pragma once

#include <unistd.h>

void job_add(pid_t);
void job_maybe_remove(pid_t);
void job_update(void);
