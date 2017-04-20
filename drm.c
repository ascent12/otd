#include "otd.h"
#include "drm.h"
#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_mode.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

static const char *conn_name[] = {
	[DRM_MODE_CONNECTOR_Unknown]     = "Unknown",
	[DRM_MODE_CONNECTOR_VGA]         = "VGA",
	[DRM_MODE_CONNECTOR_DVII]        = "DVI-I",
	[DRM_MODE_CONNECTOR_DVID]        = "DVI-D",
	[DRM_MODE_CONNECTOR_DVIA]        = "DVI-A",
	[DRM_MODE_CONNECTOR_Composite]   = "Composite",
	[DRM_MODE_CONNECTOR_SVIDEO]      = "SVIDEO",
	[DRM_MODE_CONNECTOR_LVDS]        = "LVDS",
	[DRM_MODE_CONNECTOR_Component]   = "Component",
	[DRM_MODE_CONNECTOR_9PinDIN]     = "DIN",
	[DRM_MODE_CONNECTOR_DisplayPort] = "DP",
	[DRM_MODE_CONNECTOR_HDMIA]       = "HDMI-A",
	[DRM_MODE_CONNECTOR_HDMIB]       = "HDMI-B",
	[DRM_MODE_CONNECTOR_TV]          = "TV",
	[DRM_MODE_CONNECTOR_eDP]         = "eDP",
	[DRM_MODE_CONNECTOR_VIRTUAL]     = "Virtual",
	[DRM_MODE_CONNECTOR_DSI]         = "DSI",
};

void scan_connectors(struct otd *otd)
{
	drmModeRes *res = drmModeGetResources(otd->fd);
	if (!res)
		return;

	// I don't know if this needs to be allocated with realloc like this,
	// as it may not even be possible for the number of connectors to change.
	// I'll just have to see how DisplayPort MST works with DRM.
	if ((size_t)res->count_connectors > otd->display_len) {
		struct otd_display *new = realloc(otd->displays, sizeof *new * res->count_connectors);
		if (!new)
			return;

		for (int i = otd->display_len; i < res->count_connectors; ++i)
			new[i].state = OTD_DISP_INVALID;

		otd->display_len = res->count_connectors;
		otd->displays = new;
	}

	for (int i = 0; i < res->count_connectors; ++i) {
		struct otd_display *disp = &otd->displays[i];
		drmModeConnector *conn = drmModeGetConnector(otd->fd, res->connectors[i]);
		if (!conn)
			continue;

		if (otd->displays[i].state == OTD_DISP_INVALID) {
			disp->state = OTD_DISP_DISCONNECTED;
			disp->connector = res->connectors[i];
			snprintf(disp->name, sizeof disp->name, "%s-%"PRIu32,
				 conn_name[conn->connector_type],
				 conn->connector_type_id);
		}

		if (disp->state == OTD_DISP_DISCONNECTED &&
		    conn->connection == DRM_MODE_CONNECTED) {
			struct otd_event ev = {
				.type = OTD_EV_DISPLAY_ADD,
				.display = disp,
			};

			disp->state = OTD_DISP_NEEDS_MODESET;
			event_add(otd, ev);
		} else if (disp->state == OTD_DISP_CONNECTED &&
		    conn->connection != DRM_MODE_CONNECTED) {
			struct otd_event ev = {
				.type = OTD_EV_DISPLAY_REM,
				.display = disp,
			};

			disp->state = OTD_DISP_DISCONNECTED;
			event_add(otd, ev);
		}
	}
}
