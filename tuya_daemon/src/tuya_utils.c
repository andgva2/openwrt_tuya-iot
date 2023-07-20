#include <utils/tuya_utils.h>

extern int is_daemon;
extern const char tuya_cacert_pem[];
extern const size_t tuya_cacert_pem_size;

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	TY_LOGI("on connected");
	if (is_daemon) {
		syslog(LOG_USER | LOG_INFO, "on connected");
	}
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data)
{
	TY_LOGI("on disconnect");
	if (is_daemon) {
		syslog(LOG_USER | LOG_INFO, "on disconnect");
	}
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
	if (is_daemon) {
		syslog(LOG_USER | LOG_INFO, "on message id:%s, type:%d, code:%d", msg->msgid, msg->type,
		       msg->code);
	}
	switch (msg->type) {
	case THING_TYPE_PROPERTY_REPORT_RSP:
		syslog(LOG_INFO, "Property report resonse: %s", msg->data_string);
		break;

	case THING_TYPE_PROPERTY_SET:
		syslog(LOG_INFO, "Device property set: %s", msg->data_string);
	default:
		break;
	}
}

void send_data(tuya_mqtt_context_t *context, void *user_data)
{
	char data[300];
	struct sysinfo info;

	if (sysinfo(&info) != 0) {
		TY_LOGW("sysinfo could not gather system information");
		if (is_daemon) {
			syslog(LOG_USER | LOG_WARNING, "sysinfo could not gather system information");
		}
	} else {
		float free_ram	= (info.freeram * 1.0) / 1024 / 1024;
		float total_ram = (info.totalram * 1.0) / 1024 / 1024;

		snprintf(data, sizeof(data), "{\"device_status\": {\"value\": \"Ram usage: %.2fMB/%.2fMB\"}}",
			 free_ram, total_ram);

		tuyalink_thing_property_report_with_ack(context, NULL, data);
	}
}

int tuya_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret)
{
	int ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t){ .host	  = "m1.tuyacn.com",
								      .port	  = 8883,
								      .cacert	  = tuya_cacert_pem,
								      .cacert_len = tuya_cacert_pem_size,
								      .device_id  = deviceId,
								      .device_secret = deviceSecret,
								      .keepalive     = 100,
								      .timeout_ms    = 2000,
								      .on_connected  = on_connected,
								      .on_disconnect = on_disconnect,
								      .on_messages   = on_messages });

	if (ret) {
		TY_LOGE("tuya_mqtt_init failed");
		if (is_daemon) {
			syslog(LOG_USER | LOG_ERR, "tuya_mqtt_init failed");
		}
		return ret;
	}

	ret = tuya_mqtt_connect(client);

	if (ret) {
		TY_LOGE("tuya_mqtt_connect failed");
		if (is_daemon) {
			syslog(LOG_USER | LOG_ERR, "tuya_mqtt_connect failed");
		}
		return ret;
	}

	return OPRT_OK;
}

int tuya_deinit(tuya_mqtt_context_t *client)
{
	int ret;
	ret = tuya_mqtt_disconnect(client);
	if (ret) {
		TY_LOGE("tuya_mqtt_disconnect failed");
		if (is_daemon) {
			syslog(LOG_USER | LOG_ERR, "tuya_mqtt_disconnect failed");
		}
	} else {
		TY_LOGI("tuya_mqtt_disconnect success");
		if (is_daemon) {
			syslog(LOG_USER | LOG_INFO, "tuya_mqtt_disconnect success");
		}
	}
	if (ret) {
		TY_LOGE("tuya_mqtt_deinit failed");
		if (is_daemon) {
			syslog(LOG_USER | LOG_ERR, "tuya_mqtt_deinit failed");
		}
	} else {
		TY_LOGI("tuya_mqtt_deinit success");
		if (is_daemon) {
			syslog(LOG_USER | LOG_INFO, "tuya_mqtt_deinit success");
		}
	}
	if (is_daemon) {
		closelog();
	}

	return ret;
}

int tuya_loop(tuya_mqtt_context_t *client)
{	
        int ret;
        ret = tuya_mqtt_loop(client);
	send_data(client, NULL);
        return ret;
}
