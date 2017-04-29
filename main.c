#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <GLES3/gl3.h>
#include <pthread.h>
#include <stdatomic.h>

#include "otd.h"
#include "drm.h"
#include "event.h"

static void *waiting(void *arg)
{
	atomic_bool *done = arg;

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	t.tv_sec += 5;

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
	atomic_store(done, 1);

	return NULL;
}

int main()
{
	struct otd *otd = otd_start();
	atomic_bool done = ATOMIC_VAR_INIT(0);

	if (!otd)
		return 1;

	float colour[3] = {1.0, 0.0, 0.0};
	int dec = 0;

	struct timespec last;
	clock_gettime(CLOCK_MONOTONIC, &last);

	pthread_t thrd;
	pthread_create(&thrd, NULL, waiting, &done);

	while (!atomic_load(&done)) {
		struct otd_event ev;
		if (!otd_get_event(otd, &ev))
			continue;

		struct otd_display *disp = ev.display;

		switch (ev.type) {
		case OTD_EV_RENDER:
			rendering_begin(disp);

			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);

			long ms = (now.tv_sec - last.tv_sec) * 1000 +
				(now.tv_nsec - last.tv_nsec) / 1000000;

			int inc = (dec + 1) % 3;

			colour[dec] -= ms / 2000.0f;
			colour[inc] += ms / 2000.0f;

			if (colour[dec] < 0.0f) {
				colour[dec] = 0.0f;
				colour[inc] = 1.0f;

				dec = (dec + 1) % 3;
			}

			last = now;

			glViewport(0, 0, disp->width, disp->height);
			glClearColor(colour[0], colour[1], colour[2], 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			rendering_end(disp);
			break;
		case OTD_EV_DISPLAY_REM:
			break;
		case OTD_EV_DISPLAY_ADD:
			modeset_str(otd, ev.display, "preferred");
			break;
		default:
			break;
		}
	}

	otd_finish(otd);
	pthread_join(thrd, NULL);
}
