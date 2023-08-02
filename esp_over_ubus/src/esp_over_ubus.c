#include <stdio.h>

#include <libubus.h>

#include <ubus_utils.h>

int main(void)
{
	struct ubus_context *ctx;
	const char *LOGNAME = "ESP DAEMON";
	openlog(LOGNAME, LOG_PID, LOG_USER);
	syslog(LOG_USER | LOG_INFO, "starting");

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		syslog(LOG_USER | LOG_ERR, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	server_main(ctx);

	uloop_run();

	ubus_free(ctx);
	uloop_done();

	return 0;
}