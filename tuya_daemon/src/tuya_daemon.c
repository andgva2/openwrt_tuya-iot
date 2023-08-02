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

	// mqtt loop
	ret = tuya_loop(client);

	// deinitialize mqtt client
	tuya_deinit(client);
	return ret;
}
