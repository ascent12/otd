#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "session.h"
#include "otd.h"

static bool take_device(struct otd *otd, uint32_t major, uint32_t minor)
{
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	struct otd_session *s = &otd->session;

	printf("Attempting to aquire %u %u\n", major, minor);

	ret = sd_bus_call_method(s->bus,
				 "org.freedesktop.login1",
				 s->path,
				 "org.freedesktop.login1.Session",
				 "TakeDevice",
				 &error, &msg,
				 "uu", major, minor);
	if (ret < 0) {
		fprintf(stderr, "%s\n", error.message);
		goto error;
	}

	int fd = -1, paused = 0;
	ret = sd_bus_message_read(msg, "hb", &fd, &paused);
	if (ret < 0) {
		fprintf(stderr, "%s\n", strerror(-ret));
		goto error;
	}

	// The original fd seem to be closed when the message is freed
	// so we just clone it.
	otd->fd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
	otd->paused = paused;

error:
	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret >= 0;
}

static void release_device(struct otd *otd, uint32_t major, uint32_t minor)
{
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	struct otd_session *s = &otd->session;

	ret = sd_bus_call_method(s->bus,
				 "org.freedesktop.login1",
				 s->path,
				 "org.freedesktop.login1.Session",
				 "ReleaseDevice",
				 &error, &msg,
				 "uu", major, minor);
	if (ret < 0) {
		/* Log something */;
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
}

static bool session_activate(struct otd *otd)
{
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	struct otd_session *s = &otd->session;

	ret = sd_bus_call_method(s->bus,
				 "org.freedesktop.login1",
				 s->path,
				 "org.freedesktop.login1.Session",
				 "Activate",
				 &error, &msg,
				 "");
	if (ret < 0) {
		fprintf(stderr, "%s\n", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret >= 0;
}

static bool take_control(struct otd *otd)
{
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	struct otd_session *s = &otd->session;

	ret = sd_bus_call_method(s->bus,
				 "org.freedesktop.login1",
				 s->path,
				 "org.freedesktop.login1.Session",
				 "TakeControl",
				 &error, &msg,
				 "b", false);
	if (ret < 0) {
		/* Log something */;
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret >= 0;
}

static void release_control(struct otd *otd)
{
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	struct otd_session *s = &otd->session;

	ret = sd_bus_call_method(s->bus,
				 "org.freedesktop.login1",
				 s->path,
				 "org.freedesktop.login1.Session",
				 "ReleaseControl",
				 &error, &msg,
				 "");
	if (ret < 0) {
		/* Log something */;
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
}

void otd_close_session(struct otd *otd)
{
	struct stat st;
	if (fstat(otd->fd, &st) >= 0) {
		release_device(otd, major(st.st_rdev), minor(st.st_rdev));
	}

	release_control(otd);

	sd_bus_unref(otd->session.bus);
	free(otd->session.id);
	free(otd->session.path);
}

bool otd_new_session(struct otd *otd, const char *path)
{
	int ret;
	struct otd_session *s = &otd->session;

	ret = sd_pid_get_session(getpid(), &s->id);
	if (ret < 0) {
		fprintf(stderr, "Could not get session\n");
		goto error;
	}

	// This could be done using asprintf, but I don't want to define _GNU_SOURCE

	const char *fmt = "/org/freedesktop/login1/session/%s";
	int len = snprintf(NULL, 0, fmt, s->id);

	s->path = malloc(len + 1);
	if (!s->path)
		goto error;

	sprintf(s->path, fmt, s->id);
	printf("%s\n", s->path);

	ret = sd_bus_open_system(&s->bus);
	if (ret < 0) {
		fprintf(stderr, "Could not open bus\n");
		goto error;
	}

	if (!session_activate(otd)) {
		fprintf(stderr, "Could not activate session\n");
		goto error_bus;
	}

	if (!take_control(otd)) {
		fprintf(stderr, "Could not take control of session\n");
		goto error_bus;
	}

	struct stat st;
	if (stat(path, &st) < 0)
		goto error_bus;

	if (!take_device(otd, major(st.st_rdev), minor(st.st_rdev))) {
		fprintf(stderr, "Could not take device\n");
		release_control(otd);
		goto error_bus;
	}

	return true;

error_bus:
	sd_bus_unref(s->bus);

error:
	free(s->path);
	free(s->id);
	return false;
}
