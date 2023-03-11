#pragma once

#include <stddef.h>

#define PATHSIZ  4096
#define ALLOCSIZ 1024

void error(const char *, ...);

void *xmalloc(size_t);
void *xrealloc(void *, size_t);
