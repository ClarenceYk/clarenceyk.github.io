#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define WATCHDOG_DEV     "/dev/watchdog0"
#define WATCHDOG_TIMEOUT 2

static int watchdog_fd = -1;

int api_watchdog_open(const char *watchdog_dev)
{
	if (watchdog_fd >= 0) {
		fprintf(stderr, "Watchdog already opened!\n");
		return -1;
	}

	watchdog_fd = open(watchdog_dev, O_RDWR);
	if (watchdog_fd < 0)
		fprintf(stderr, "Could not open %s: %s\n", watchdog_dev, strerror(errno));

	return watchdog_fd;
}

int api_watchdog_settimeout(int seconds)
{
	int ret = -1;

	if (watchdog_fd < 0) {
		fprintf(stderr, "Open watchdog first!\n");
		return ret;
	}

	ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &seconds);
	if (ret < 0)
		fprintf(stderr, "Could not set timeout: %s\n", strerror(errno));
	printf("Watchdog set timeout to %ds\n", seconds);

	return ret;
}

int api_watchdog_feed(void)
{
	int ret = -1;

	if (watchdog_fd < 0) {
		fprintf(stderr, "Open watchdog first!\n");
		return ret;
	}

	ret = ioctl(watchdog_fd, WDIOC_KEEPALIVE, NULL);
	if (ret < 0)
		fprintf(stderr, "Could not feed watchdog: %s\n", strerror(errno));

	return ret;
}

int api_watchdog_init(const char *watchdog_dev, int timeout)
{
	int ret = -1;

	ret = api_watchdog_open(watchdog_dev);
	if (ret < 0)
		return ret;

	ret = api_watchdog_settimeout(timeout);
	if (ret < 0)
		return ret;

	ret = api_watchdog_feed();
	return ret;
}

int main(void)
{
	int ret = -1;
	struct timespec timeout = { 1, 0 };

	ret = api_watchdog_init(WATCHDOG_DEV, WATCHDOG_TIMEOUT);
	if (ret < 0)
		return ret;

	printf("Watchdog opened!\n");

	while (1) {
		api_watchdog_feed();
		nanosleep(&timeout, NULL);
	}
}

