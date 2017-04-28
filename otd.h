#ifndef LIBOTD_H
#define LIBOTD_H

#include <stddef.h>
#include <EGL/egl.h>
#include <gbm.h>

struct otd {
	int fd;

	// Priority Queue (Max-heap)
	size_t event_cap;
	size_t event_len;
	struct otd_event *events;

	size_t display_len;
	struct otd_display *displays;

	uint32_t taken_crtcs;

	struct gbm_device *gbm;
	struct {
		EGLDisplay disp;
		EGLConfig conf;
		EGLContext context;
	} egl;
};

struct otd *otd_start(void);
void otd_finish(struct otd *otd);

#endif
