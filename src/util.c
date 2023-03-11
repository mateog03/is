#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);

	va_end(ap);
}

void *xmalloc(size_t size)
{
	void *p = malloc(size);
	if (!p) {
		error("memoria non disponibile\n");
		abort();
	}

	return p;
}

void *xrealloc(void *src, size_t size)
{
	void *p = realloc(src, size);
	if (!p) {
		error("memoria non disponibile\n");
		abort();
	}

	return p;
}
