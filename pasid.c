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

*/

#include <stdio.h>
#include <pulse/pulseaudio.h>

static void
get_sink_input_info_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
	pa_mainloop_api *api;
	api = (pa_mainloop_api *)(userdata);

	if (eol < 0) {
		fprintf(stderr, "pasid: pa_context_get_sink_input_info_list failed: %s\n", pa_strerror(pa_context_errno(c)));
		api->quit(api, 1);
		return;
	}

	if (eol > 0) {
		api->quit(api, 0);
		return;
	}

	if (i) {
		printf("%d - %s\n", i->index, pa_proplist_gets(i->proplist, "application.name"));
	}
}

static void
context_ready_callback(pa_context *c, void *userdata) {
	pa_operation_unref(pa_context_get_sink_input_info_list(c, get_sink_input_info_callback, userdata));
}

static void
context_state_callback(pa_context *c, void *userdata) {
	pa_mainloop_api *api;
	api = (pa_mainloop_api *)(userdata);

	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_READY:
			context_ready_callback(c, userdata);
			break;
		case PA_CONTEXT_TERMINATED:
			api->quit(api, 0);
			break;
		case PA_CONTEXT_FAILED:
		default:
			fprintf(stderr, "pasid: pa_context_connect failed: %s\n", pa_strerror(pa_context_errno(c)));
			api->quit(api, 1);
			break;
	}
}

int
main(void) {
	int retval;
	pa_mainloop_api *api;
	pa_mainloop *m;
	pa_context *context;

	m = pa_mainloop_new();
	api = pa_mainloop_get_api(m);
	context = pa_context_new(api, NULL);

	pa_context_set_state_callback(context, context_state_callback, (void *)(api));
	pa_context_connect(context, NULL, 0, NULL);
	pa_mainloop_run(m, &retval);

	pa_context_unref(context);
	pa_mainloop_free(m);

	return retval;
}
