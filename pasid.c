/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

	 _________________________________________
	( parsing :pactl list sink-inputs: output )
	( is just inconvenient                    )
	 -----------------------------------------
	     o
	      o
	  /\          /\
	 ( \\        // )
	  \ \\      // /
	   \_\\||||//_/
	     / _  _ \/
	     |(o)(o)|\/
	     |      | \/
	     \      /  \/_____________________
	      |____|     \\                  \\
	     /      \     ||                  \\
	     \ 0  0 /     |/                  |\\
	      \____/ \    V           (       / \\
	       / \    \     )          \     /   \\
	      / | \    \_|  |___________\   /     ""
	                  ||  |     \   /\  \
	                  ||  /      \  \ \  \
	                  || |        | |  | |
	                  || |        | |  | |
	                  ||_|        |_|  |_|
	                 //_/        /_/  /_/

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <pulse/pulseaudio.h>

#define UNUSED __attribute__((unused))

#define PASID_EXIT_SUCCESS      (0)
#define PASID_EXIT_FAILURE      (1)
#define PASID_EXIT_NO_MATCH     (2)

static char *query = NULL;
static int found = 0;
static pa_mainloop_api *api;

static void
die(const char *err)
{
	fprintf(stderr, "pasid: %s\n", err);
	exit(PASID_EXIT_FAILURE);
}

static void
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

static int
strcontains(const char *str, const char *x)
{
	int slen, xlen, attempts;

	slen = (int)(strlen(str));
	xlen = (int)(strlen(x));
	attempts = slen - xlen + 1;

	while (attempts-- > 0) {
		for (int i = 0; i < xlen; ++i) {
			if (tolower(str[attempts + i]) != tolower(x[i])) break;
			if (i == xlen - 1) return 1;
		}
	}

	return 0;
}

static void
get_sink_in_cb(pa_context *c,
               const pa_sink_input_info *i,
               int eol,
               UNUSED void *data)
{
	uint32_t     sink_id;
	const char  *sink_appname;

	if (eol < 0) {
		dief("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(c)));
	}

	if (eol > 0 || NULL == i) {
		api->quit(api, 0);
		return;
	}

	sink_id = i->index;
	sink_appname = pa_proplist_gets(i->proplist, PA_PROP_APPLICATION_NAME);

	if (NULL == query) {
		printf("%u - %s\n", sink_id, sink_appname);
		return;
	}

	if (!found && (found = strcontains(sink_appname, query))) {
		printf("%u\n", sink_id);
	}
}

static void
context_ready_cb(pa_context *c)
{
	pa_operation *po;

	po = pa_context_get_sink_input_info_list(c, get_sink_in_cb, NULL);

	if (NULL == po) {
		dief("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(c)));
	}

	pa_operation_unref(po);
}

static void
context_state_cb(pa_context *c, UNUSED void *data)
{
	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_READY:
			context_ready_cb(c);
			break;
		case PA_CONTEXT_TERMINATED:
			api->quit(api, 0);
			break;
		case PA_CONTEXT_FAILED:
		default:
			dief("pa_context_connect failed: %s",
					pa_strerror(pa_context_errno(c)));
			break;
	}
}

static inline void
print_opt(const char *sh, const char *lo, const char *desc)
{
	printf("%7s | %-25s %s\n", sh, lo, desc);
}

static int
match_opt(const char *in, const char *sh, const char *lo)
{
	return (strcmp(in, sh) == 0) || (strcmp(in, lo) == 0);
}

static void
usage(void)
{
	puts("Usage: pasid [ -hv ] [ -m QUERY ]");
	puts("Options are:");
	print_opt("-m", "--match", "get sink id by application name");
	print_opt("-h", "--help", "display this message and exit");
	print_opt("-v", "--version", "display the program version");
	exit(PASID_EXIT_SUCCESS);
}

static void
version(void)
{
	puts("pasid version "VERSION);
	exit(PASID_EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	pa_mainloop *m;
	pa_context *context;

	if (++argv, --argc > 0) {
		if (match_opt(*argv, "-h", "--help")) usage();
		else if (match_opt(*argv, "-v", "--version")) version();
		else if (match_opt(*argv, "-m", "--match") && --argc > 0) query = *++argv;
		else if (**argv == '-') dief("invalid option %s", *argv);
		else dief("unexpected argument: %s", *argv);
	}

	if (NULL == (m = pa_mainloop_new())) {
		die("pa_mainloop_new failed");
	}

	api = pa_mainloop_get_api(m);

	if (NULL == (context = pa_context_new(api, NULL))) {
		die("pa_context_new failed");
	}

	pa_context_set_state_callback(context, context_state_cb, NULL);

	if (pa_context_connect(context, NULL, 0, NULL) < 0) {
		dief("pa_context_connect failed: %s",
				pa_strerror(pa_context_errno(context)));
	}

	if (pa_mainloop_run(m, NULL) < 0) {
		die("pa_mainloop_run failed");
	}

	pa_context_unref(context);
	pa_mainloop_free(m);

	if (NULL != query && !found) {
		return PASID_EXIT_NO_MATCH;
	}

	return PASID_EXIT_SUCCESS;
}
