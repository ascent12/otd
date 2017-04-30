#include <libudev.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "otd.h"
#include "udev.h"
#include "session.h"

static bool device_is_kms(struct otd *otd, struct udev_device *dev)
{
	const char *path = udev_device_get_devnode(dev);
	int fd;

	if (!path)
		return false;

	fd = take_device(otd, path, &otd->paused);
	if (fd < 0)
		return false;

	drmModeRes *res = drmModeGetResources(fd);
	if (!res)
		goto out_fd;

	if (res->count_crtcs <= 0 || res->count_connectors <= 0 ||
	    res->count_encoders <= 0)
		goto out_res;

	if (otd->fd >= 0)
		release_device(otd, otd->fd);

	otd->fd = fd;

	drmModeFreeResources(res);
	return true;

out_res:
	drmModeFreeResources(res);
out_fd:
	release_device(otd, fd);
	return false;
}

void otd_udev_find_gpu(struct otd *otd)
{
	otd->fd = -1;

	struct udev *udev = udev_new();
	if (!udev)
		return;

	otd->udev = udev;

	struct udev_enumerate *en = udev_enumerate_new(udev);
	if (!en)
		return;

	udev_enumerate_add_match_subsystem(en, "drm");
	udev_enumerate_add_match_sysname(en, "card[0-9]*");

	udev_enumerate_scan_devices(en);
	struct udev_list_entry *entry;
	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(en)) {
		bool is_boot_vga = false;

		const char *path = udev_list_entry_get_name(entry);
		struct udev_device *dev = udev_device_new_from_syspath(udev, path);
		if (!dev)
			continue;

		const char *seat = udev_device_get_property_value(dev, "ID_SEAT");
		if (!seat)
			seat = "seat0";
		if (strcmp(otd->session.seat, seat) != 0) {
			udev_device_unref(dev);
			continue;
		}

		struct udev_device *pci =
			udev_device_get_parent_with_subsystem_devtype(dev,
								      "pci", NULL);

		if (pci) {
			const char *id = udev_device_get_sysattr_value(pci, "boot_vga");
			if (id && strcmp(id, "1") == 0)
				is_boot_vga = true;
			udev_device_unref(pci);
		}

		if (!is_boot_vga && otd->fd >= 0) {
			udev_device_unref(dev);
			continue;
		}

		if (!device_is_kms(otd, dev)) {
			udev_device_unref(dev);
			continue;
		}

		if (is_boot_vga) {
			break;
		}
	}

	udev_enumerate_unref(en);
}
