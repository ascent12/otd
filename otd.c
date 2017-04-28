#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "otd.h"
#include "drm.h"
#include "event.h"

struct otd *otd_start(void)
{
	struct otd *otd = calloc(1, sizeof *otd);
	if (!otd)
		return NULL;

	otd->fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (otd->fd < 0)
		goto error;

	if (!init_renderer(otd)) {
		goto error;
	}

	scan_connectors(otd);

	return otd;

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

	close(otd->fd);
	free(otd->events);
	free(otd->displays);
	free(otd);
}

