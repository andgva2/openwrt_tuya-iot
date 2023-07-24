#include <tuya_utils.h>

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on connected");
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on disconnect");
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	switch (msg->type) {
	case THING_TYPE_PROPERTY_REPORT_RSP:
		syslog(LOG_INFO, "Property report sent and aknowledged by the cloud.");
		break;

	default:
		break;
	}
}

void send_data(tuya_mqtt_context_t *context, char *user_data)
{
	char template[1024];

	snprintf(template, sizeof(template), "{\"device_status\": {\"value\": \"%s\"}}", user_data);

	tuyalink_thing_property_report_with_ack(context, NULL, template);
}

int tuya_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret)
{
	int ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t){ .host	  = "m1.tuyacn.com",
								      .port	  = 8883,
								      .cacert	  = tuya_cacert_pem,
								      .cacert_len = sizeof(tuya_cacert_pem),
								      .device_id  = deviceId,
								      .device_secret = deviceSecret,
								      .keepalive     = 100,
								      .timeout_ms    = 2000,
								      .on_connected  = on_connected,
								      .on_disconnect = on_disconnect,
								      .on_messages   = on_messages });

	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_init failed");

		return ret;
	}

	syslog(LOG_USER | LOG_INFO, "tuya_mqtt_init success");

	ret = tuya_mqtt_connect(client);

	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_connect failed");

		return ret;
	}

	syslog(LOG_USER | LOG_INFO, "tuya_mqtt_connect success");

	return OPRT_OK;
}

int tuya_deinit(tuya_mqtt_context_t *client)
{
	int ret;
	ret = tuya_mqtt_disconnect(client);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_disconnect failed");

	} else {
		syslog(LOG_USER | LOG_INFO, "tuya_mqtt_disconnect success");
	}
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_deinit failed");

	} else {
		syslog(LOG_USER | LOG_INFO, "tuya_mqtt_deinit success");
	}

	closelog();

	return ret;
}

int tuya_loop(tuya_mqtt_context_t *client, struct MemData user_data)
{
	int ret;

	ret = tuya_mqtt_loop(client);
	if (ret) {
		syslog(LOG_USER | LOG_INFO, "connection failed");
		return ret;
	}

	char data[992];
	snprintf(data, sizeof(data), "Memory usage: %.2fMB/%.2fMB", (user_data.total - user_data.free) / 1024 / 1024.0,
		 user_data.total / 1024 / 1024.0);

	send_data(client, data);
	return OPRT_OK;
}
