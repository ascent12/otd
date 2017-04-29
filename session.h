#ifndef SESSION_H
#define SESSION_H

#include <systemd/sd-bus.h>

struct otd_session {
	char *id;
	char *path;

	sd_bus *bus;
};

struct otd;
bool otd_new_session(struct otd *otd, const char *path);
void otd_close_session(struct otd *otd);

#endif
