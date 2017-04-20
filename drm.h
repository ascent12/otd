#ifndef DRM_H
#define DRM_H

#include <stdint.h>

enum otd_display_state {
	OTD_DISP_INVALID,
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

void scan_connectors(struct otd *otd);

#endif
