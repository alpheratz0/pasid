#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

extern void
dief(const char *err, ...)
{
	va_list list;
	fputs("pasid: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputc('\n', stderr);
	exit(1);
}
