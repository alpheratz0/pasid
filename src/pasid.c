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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <pulse/pulseaudio.h>

#include "debug.h"

#define UNUSED __attribute__((unused))

#define PRINTOPT(sh,lo,desc) do { \
	printf("%7s | %-25s %s\n", (sh), (lo), (desc)); \
} while (0)

static char *query = NULL;
static bool found = false;
static pa_mainloop_api *api;

static bool
strcontains(const char *str, const char *x)
{
	int slen, xlen, attempts;

	slen = (int)(strlen(str));
	xlen = (int)(strlen(x));
	attempts = slen - xlen + 1;

	while (attempts-- > 0) {
		for (int i = 0; i < xlen; ++i) {
			if (tolower(str[attempts + i]) != tolower(x[i])) break;
			if (i == xlen - 1) return true;
		}
	}

	return false;
}

static void
get_sink_in_cb(pa_context *c, const pa_sink_input_info *i, int eol, UNUSED void *data)
{
	uint32_t 	 sink_id;
	const char	*sink_appname;

	if (eol < 0) {
		dief("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(c)));
	}

	if (eol > 0 || NULL == i) {
		api->quit(api, 0);
		return;
	}

	sink_id = i->index;
	sink_appname = pa_proplist_gets(i->proplist, "application.name");

	if (NULL == query) {
		printf("%d - %s\n", sink_id, sink_appname);
		return;
	}

	if (!found && (found = strcontains(sink_appname, query))) {
		printf("%d\n", sink_id);
	}
}

static void
context_ready_cb(pa_context *c)
{
	pa_operation_unref(
		pa_context_get_sink_input_info_list(
			c, get_sink_in_cb, NULL
		)
	);
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

static bool
match_opt(const char *in, const char *sh, const char *lo)
{
	return (strcmp(in, sh) == 0) ||
		   (strcmp(in, lo) == 0);
}

static void
usage(void)
{
	puts("Usage: pasid [ -hv ] [ -m QUERY ]");
	puts("Options are:");

	PRINTOPT(
		"-m", "--match",
		"get the sink id of the application that matches the query"
	);

	PRINTOPT("-h", "--help", "display this message and exit");
	PRINTOPT("-v", "--version", "display the program version");

	exit(0);
}

static void
version(void)
{
	puts("pasid version "VERSION);
	exit(0);
}

int
main(int argc, char **argv)
{
	/* skip program name */
	--argc; ++argv;

	if (argc > 0) {
		if (match_opt(*argv, "-h", "--help")) usage();
		else if (match_opt(*argv, "-v", "--version")) version();
		else if (match_opt(*argv, "-m", "--match") && --argc > 0) query = *++argv;
		else if (**argv == '-') dief("invalid option %s", *argv);
		else dief("unexpected argument: %s", *argv);
	}

	pa_mainloop *m;
	pa_context *context;

	m = pa_mainloop_new();
	api = pa_mainloop_get_api(m);
	context = pa_context_new(api, NULL);

	pa_context_set_state_callback(context, context_state_cb, NULL);
	pa_context_connect(context, NULL, 0, NULL);

	/* retval not used */
	pa_mainloop_run(m, (int[1]) { 0 });

	pa_context_unref(context);
	pa_mainloop_free(m);

	if (NULL != query && !found) {
		return 2;
	}

	return 0;
}
