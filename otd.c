#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "otd.h"
#include "drm.h"
#include "event.h"
#include "session.h"

struct otd *otd_start(void)
{
	struct otd *otd = calloc(1, sizeof *otd);
	if (!otd)
		return NULL;

	if (!otd_new_session(otd, "/dev/dri/card0")) {
		fprintf(stderr, "Could not create new session\n");
		goto error;
	}

	if (!init_renderer(otd)) {
		fprintf(stderr, "Could not initalise renderer\n");
		goto error_session;
	}

	scan_connectors(otd);

	return otd;

error_session:
	otd_close_session(otd);
error:
	free(otd);
	return NULL;
}

void otd_finish(struct otd *otd)
{
	if (!otd)
		return;

	for (size_t i = 0; i < otd->display_len; ++i) {
		destroy_display_renderer(otd, &otd->displays[i]);
	}

	destroy_renderer(otd);
	otd_close_session(otd);

	close(otd->fd);
	free(otd->events);
	free(otd->displays);
	free(otd);
}

