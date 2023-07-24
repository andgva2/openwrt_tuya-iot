#include <tuyalink_core.h>
#include <tuya_log.h>
#include <tuya_cacert.h>

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include <argp_utils.h>
#include <ubus_utils.h>
#include <tuya_utils.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>

volatile sig_atomic_t running = 1;

void signal_handler(int sig)
{
	if (sig != SIGTERM && sig != SIGINT) {
		syslog(LOG_USER | LOG_ERROR, "signal %d handling error", sig);
		return;
	}

	syslog(LOG_USER | LOG_INFO, "signal %d received, stopping", sig);
	running = 0;
}

tuya_mqtt_context_t client_instance;

int main(int argc, char **argv)
{
	// signal handling
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

	// initialize argp
	struct arguments arguments;

	arguments.device_id   = NULL;
	arguments.secret      = NULL;
	arguments.product_id  = NULL;
	arguments.daemon_flag = 0;
	int is_daemon	      = 1;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	const char *deviceId	 = arguments.device_id;
	const char *deviceSecret = arguments.secret;
	const char *productId	 = arguments.product_id;
	is_daemon		 = arguments.daemon_flag;

	// return value
	int ret;

	// turn this process into a daemon
	if (is_daemon) {
		ret = daemonize(0);
		if (ret) {
			TY_LOGE("daemonization failed");
			TY_LOGI("running in terminal");
			is_daemon = 0;
		} else {
			const char *LOGNAME = "TUYA DAEMON";
			openlog(LOGNAME, LOG_PID, LOG_USER);
			syslog(LOG_USER | LOG_INFO, "starting");
		}
	}

	tuya_mqtt_context_t *client = &client_instance;

	// initialize mqtt client
	ret = tuya_init(client, deviceId, deviceSecret);

	if (ret) {
		tuya_deinit(client);
		return ret;
	}

	// ubus initailization
	struct ubus_context *ctx;
	uint32_t id;

	struct MemData memory = { 0 };

	ctx = ubus_connect(NULL);
	if (!ctx) {
		syslog(LOG_USER | LOG_ERROR, "failed to connect to ubus");
		return -1;
	}

	// mqtt loop
	while (running) {
		if (ubus_lookup_id(ctx, "system", &id) ||
		    ubus_invoke(ctx, id, "info", NULL, board_cb, &memory, 3000)) {
			syslog(LOG_USER | LOG_ERR, "cannot request memory info from procd");
			break;
		}
		ret = tuya_loop(client, memory);
	}

	tuya_deinit(client);
	ubus_free(ctx);
	return ret;
}
