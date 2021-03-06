/*
 * Copyright (c) 2012-2013 Varnish Software AS
 * All rights reserved.
 *
 * Author: Kristian Lyngstøl <kristian@bohemians.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "plugins.h"
#include "ipc.h"
#include "http.h"
#include "helpers.h"

#define DIRECT_HELP							\
	"You can issue verbatim varnish CLI commands (varnishadm) by "	\
	"posting a single line\nto /direct.\n"

struct vdirect_priv_t {
	int logger;
	int vadmin;
};

static unsigned int
vdirect_reply(struct http_request *request, const char * arg, void *data)
{
	struct agent_core_t *core = data;
	struct vdirect_priv_t *vdirect;
	char *cmd, *p;

	(void)arg;

	GET_PRIV(core, vdirect);
	DUP_OBJ(cmd, request->body, request->bodylen);
	p = strchr(cmd, '\n');
	if (p)
		*p = '\0';
	run_and_respond(vdirect->vadmin, request->connection, cmd);
	free(cmd);
	return (0);
}

void
vdirect_init(struct agent_core_t *core)
{
	struct agent_plugin_t *plug;
	struct vdirect_priv_t *priv;

	ALLOC_OBJ(priv);
	plug = plugin_find(core, "vdirect");
	priv->logger = ipc_register(core, "logger");
	priv->vadmin = ipc_register(core, "vadmin");
	plug->data = (void *)priv;
	http_register_path(core, "/direct", M_POST, vdirect_reply, core);
	http_register_path(core, "/help/direct", M_GET, help_reply,
	    strdup(DIRECT_HELP));
}
