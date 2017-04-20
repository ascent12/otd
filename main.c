#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>

#include "otd.h"
#include "drm.h"
#include "event.h"

struct otd *otd_start(void)
{
	struct otd *otd = malloc(sizeof *otd);
	if (!otd)
		return NULL;

	otd->fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (otd->fd < 0)
		goto error_fd;

	otd->event_cap = 0;
	otd->event_len = 0;
	otd->events = NULL;

	otd->display_len = 0;
	otd->displays = NULL;

	scan_connectors(otd);

	return otd;

error_fd:
	free(otd);
	return NULL;
}

void otd_finish(struct otd *otd)
{
	if (!otd)
		return;

	close(otd->fd);
	free(otd->events);
	free(otd);
}

int main()
{
	struct otd *otd = otd_start();

	struct otd_event ev;
	while (otd_get_event(otd, &ev)) {
		switch (ev.type) {
		case OTD_EV_RENDER:
			printf("%s rendered\n", ev.display->name);
			break;
		case OTD_EV_DISPLAY_REM:
			printf("%s removed\n", ev.display->name);
			break;
		case OTD_EV_DISPLAY_ADD:
			printf("%s added\n", ev.display->name);
			break;
		default:
			break;
		}
	}

	otd_finish(otd);
}
