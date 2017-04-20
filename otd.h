#ifndef LIBOTD_H
#define LIBOTD_H

#include <stddef.h>

struct otd {
	int fd;

	// Priority Queue (Max-heap)
	size_t event_cap;
	size_t event_len;
	struct otd_event *events;

	size_t display_len;
	struct otd_display *displays;
};

#endif
