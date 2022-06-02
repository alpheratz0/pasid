#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"
#include "exit_status.h"

extern void
die(const char *err)
{
	fprintf(stderr, "pasid: %s\n", err);
	exit(PASID_EXIT_FAILURE);
}

extern void
dief(const char *err, ...)
{
	va_list list;
	fputs("pasid: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputc('\n', stderr);
	exit(PASID_EXIT_FAILURE);
}
