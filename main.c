#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>

#include "otd.h"
#include "event.h"

enum otd_display_state {
	OTD_DISP_DISCONNECTED,
	OTD_DISP_NEEDS_MODESET,
	OTD_DISP_CONNECTED,
};

struct otd_display {
	enum otd_display_state state;
	uint32_t connector;
	char name[16];

	uint32_t crtc;
};

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
		printf("%d\n", ev.type);
	}

	otd_finish(otd);
}
