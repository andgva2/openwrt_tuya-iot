#include <tuya_utils.h>

#define LUA_SCRIPTS_DIR "/usr/lib/tuya/lua"

volatile sig_atomic_t running = 1;

void signal_handler(int sig)
{
	if (sig != SIGTERM && sig != SIGINT) {
		syslog(LOG_USER | LOG_ERR, "signal %d handling error", sig);
		//TY_LOGE("signal %d handling error", sig);
		return;
	}

	syslog(LOG_USER | LOG_INFO, "signal %d received, stopping", sig);
	//TY_LOGI("signal %d received, stopping", sig);
	running = 0;
}

int validate_json(char *payload)
{
	int ret = 0;

	cJSON *root = cJSON_Parse(payload);
	if (root == NULL) {
		ret = 1;
		goto cleanup;
	}

cleanup:;
	cJSON_Delete(root);
	return ret;
}

void tuya_property_report(tuya_mqtt_context_t *context, char *payload)
{
	tuyalink_thing_property_report(context, NULL, payload);
}

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on connected");
	//TY_LOGI("on connected");
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on disconnect");
	//TY_LOGI("on disconnect");
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	switch (msg->type) {
	default:
		break;
	}
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
		//TY_LOGE("tuya_mqtt_init failed");
		return ret;
	}

	syslog(LOG_USER | LOG_INFO, "tuya_mqtt_init success");
	//TY_LOGI("tuya_mqtt_init success");

	ret = tuya_mqtt_connect(client);

	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_connect failed");
		//TY_LOGE("tuya_mqtt_connect failed");
		return ret;
	}

	syslog(LOG_USER | LOG_INFO, "tuya_mqtt_connect success");
	//TY_LOGI("tuya_mqtt_connect success");

	return OPRT_OK;
}

int tuya_deinit(tuya_mqtt_context_t *client)
{
	int ret;
	ret = tuya_mqtt_disconnect(client);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_disconnect failed");
		//TY_LOGE("tuya_mqtt_disconnect failed");

	} else {
		syslog(LOG_USER | LOG_INFO, "tuya_mqtt_disconnect success");
		//TY_LOGI("tuya_mqtt_disconnect success");
	}
	ret = tuya_mqtt_deinit(client);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "tuya_mqtt_deinit failed");
		//TY_LOGE("tuya_mqtt_deinit failed");
	} else {
		syslog(LOG_USER | LOG_INFO, "tuya_mqtt_deinit success");
		//TY_LOGI("tuya_mqtt_deinit success");
	}

	closelog();

	return ret;
}

int tuya_loop(tuya_mqtt_context_t *client)
{
	// return value
	int ret;

	// tuya loop
	while (running) {
		ret = tuya_mqtt_loop(client);

		if (ret) {
			syslog(LOG_USER | LOG_INFO, "connection failed");
			//TY_LOGI("connection failed");
			return ret;
		}

		char **files	= NULL;
		int files_count = 0;
		if (get_lua_files(LUA_SCRIPTS_DIR, &files, &files_count)) {
			syslog(LOG_USER | LOG_ERR, "Failed to get lua files.");
			continue;
		}
		syslog(LOG_USER | LOG_INFO, "Files count: %d", files_count);
		char *payload = NULL;

		for (int i = 0; i < files_count; i++) {
			syslog(LOG_USER | LOG_INFO, "File: %s", files[i]);

			if (get_data_from_lua_file(files[i], &payload)) {
				syslog(LOG_USER | LOG_ERR, "Failed to get data from lua file.");
				continue;
			}

			if (payload == NULL) {
				syslog(LOG_USER | LOG_ERR, "Payload is NULL.");
				continue;
			}
			syslog(LOG_USER | LOG_INFO, "Payload: %s", payload);

			if (validate_json(payload)) {
				syslog(LOG_USER | LOG_ERR, "Payload is not valid json.");
				free(payload);
				continue;
			}

			tuya_property_report(client, payload);

			free(payload);
		}

		for (int i = 0; i < files_count; i++) {
			free(files[i]);
		}
		free(files);
	}

	return OPRT_OK;
}
