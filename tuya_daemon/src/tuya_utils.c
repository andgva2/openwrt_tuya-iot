#include <tuya_utils.h>

volatile sig_atomic_t running = 1;
struct ubus_context *ctx;
uint32_t id;

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

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on connected");
	//TY_LOGI("on connected");

	char execute_topic[64];
	sprintf(execute_topic, "tylink/%s/thing/action/execute", context->config.device_id);
	mqtt_client_subscribe(context->mqtt_client, execute_topic, MQTT_QOS_1);
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data)
{
	syslog(LOG_USER | LOG_INFO, "on disconnect");
	//TY_LOGI("on disconnect");
}

int tuyalink_thing_action_execute_response(tuya_mqtt_context_t *context, const char *device_id,
					   const char *data)
{
	if (context == NULL || data == NULL) {
		return OPRT_INVALID_PARM;
	}

	tuyalink_message_t message = { .type	    = THING_TYPE_ACTION_EXECUTE_RSP,
				       .device_id   = (char *)device_id,
				       .data_string = (char *)data,
				       .ack	    = false };
	return tuyalink_message_send(context, &message);
}

void action_response(tuya_mqtt_context_t *context, int response_status, char *response_message,
		     char *action_code)
{
	char payload[1024];
	if (strcmp(action_code, "set_on") == 0 || strcmp(action_code, "set_off") == 0) {
		/// Send action response (e.g. {"actionCode":"String","outputParams":{"response":Integer,"msg":"String"}}
		snprintf(payload, sizeof(payload),
			 "{\"actionCode\": \"%s\", \"outputParams\": {\"response\": %d, \"msg\": \"%s\"}}",
			 action_code, response_status, response_message);
	} else {
		/// list_devices action
		/// Send action response (e.g. {"actionCode":"String","outputParams":{"device_list":"String"}}
		snprintf(payload, sizeof(payload),
			 "{\"actionCode\": \"%s\", \"outputParams\": {\"device_list\": \"%s\"}}", action_code,
			 response_message);
	}

	tuyalink_thing_action_execute_response(context, NULL, payload);
}

char *execute(char *payload, char *action, int *status)
{
	/// Parse inputParams and actionCode from payload (e.g. {"inputParams":{"pin":12,"port":"abc"},"actionCode":"set_on"})
	cJSON *root = cJSON_Parse(payload);
	if (!root) {
		*status = 1;
		syslog(LOG_USER | LOG_ERR, "Invalid JSON (execute)");
		//TY_LOGE("Invalid JSON (execute)");
		return "Invalid JSON";
	}

	cJSON *input_params = cJSON_GetObjectItem(root, "inputParams");
	if (!input_params) {
		*status = 1;
		syslog(LOG_USER | LOG_ERR, "Invalid inputParams (execute)");
		//TY_LOGE("Invalid inputParams (execute)");
		return "Invalid inputParams";
	}

	if (strcmp(action, "set_on") == 0 || strcmp(action, "set_off") == 0) {
		/// Parse port and pin
		cJSON *pin = cJSON_GetObjectItem(input_params, "pin");
		if (!pin) {
			*status = 1;
			syslog(LOG_USER | LOG_ERR, "Invalid pin (execute)");
			//TY_LOGE("Invalid pin (execute)");
			return "Invalid pin";
		}

		cJSON *port = cJSON_GetObjectItem(input_params, "port");
		if (!port) {
			*status = 1;
			syslog(LOG_USER | LOG_ERR, "Invalid port (execute)");
			//TY_LOGE("Invalid port (execute)");
			return "Invalid port";
		}

		struct set_resp response = { 0 };
		static struct blob_buf b;
		blob_buf_init(&b, 0);
		blobmsg_add_string(&b, "port", port->valuestring);
		blobmsg_add_u32(&b, "pin", pin->valueint);
		ubus_invoke(ctx, id, action, b.head, set_cb, &response, 3000);

		*status = response.status;
		return response.message;
	} else {
		char *port_list[100];
		char *port_list_str = NULL;
		port_list_str	    = malloc(sizeof(char) * 3001);
		strcpy(port_list_str, "\0");

		ubus_invoke(ctx, id, action, NULL, list_devices_cb, &port_list, 3000);

		int size = atoi(port_list[0]);
		free(port_list[0]);

		if (size == 0) {
			return port_list_str;
		} else if (size == 1) {
			strcat(port_list_str, port_list[1]);
			free(port_list[1]);
			return port_list_str;
		} else {
			strcat(port_list_str, port_list[1]);
			free(port_list[1]);
			for (int i = 2; i < size + 1; i++) {
				strcat(port_list_str, ", ");
				strcat(port_list_str, port_list[i]);
				free(port_list[i]);
			}
			return port_list_str;
		}
	}
}

char *action_validation(char *payload, char *action, int *status)
{
	/// Parse inputParams and actionCode from payload (e.g. {"inputParams":{"pin":12,"port":"abc"},"actionCode":"set_on"})
	cJSON *root = cJSON_Parse(payload);
	if (!root) {
		*status = 1;
		syslog(LOG_USER | LOG_INFO, "Invalid JSON (action_validation)");
		//TY_LOGI("Invalid JSON (action_validation)");
		return "Invalid JSON";
	}

	cJSON *action_code = cJSON_GetObjectItem(root, "actionCode");
	if (!action_code) {
		*status = 1;
		syslog(LOG_USER | LOG_INFO, "Invalid actionCode (action_validation)");
		//TY_LOGI("Invalid actionCode (action_validation)");
		return "Invalid actionCode";
	}

	cJSON *input_params = cJSON_GetObjectItem(root, "inputParams");
	if (!input_params) {
		*status = 1;
		syslog(LOG_USER | LOG_INFO, "Invalid inputParams (action_validation)");
		//TY_LOGI("Invalid inputParams (action_validation)");
		return "Invalid inputParams";
	}

	// Switch case like for action
	if (strcmp(action, "set_on") == 0 || strcmp(action, "set_off") == 0) {
		/// Check if inputParams is valid
		cJSON *pin = cJSON_GetObjectItem(input_params, "pin");
		if (!pin || pin->type != cJSON_Number || pin->valueint < 0 || pin->valueint > 16) {
			*status = 1;
			syslog(LOG_USER | LOG_INFO, "Invalid pin (action_validation)");
			//TY_LOGI("Invalid pin (action_validation)");
			return "Invalid pin";
		}

		cJSON *port = cJSON_GetObjectItem(input_params, "port");
		if (!port || port->type != cJSON_String) {
			*status = 1;
			syslog(LOG_USER | LOG_INFO, "Invalid port (action_validation)");
			//TY_LOGI("Invalid port (action_validation)");
			return "Invalid port";
		}

		char *port_list[100];
		ubus_invoke(ctx, id, "list_devices", NULL, list_devices_cb, &port_list, 3000);

		int contains = 0;
		int size     = atoi(port_list[0]);
		free(port_list[0]);
		for (int i = 1; i < size + 1; i++) {
			if (strcmp(port_list[i], port->valuestring) == 0) {
				contains = 1;
			}
			free(port_list[i]);
		}
		if (!contains) {
			syslog(LOG_USER | LOG_INFO, "Invalid port (action_validation)");
			//TY_LOGI("Invalid port (action_validation)");
			*status = 1;
			return "Invalid port";
		}
	} else if (strcmp(action, "list_devices") == 0) {
		// Do nothing, no args, so no validation required
	} else {
		*status = 1;
		syslog(LOG_USER | LOG_INFO, "Unknown actionCode (action_validation)");
		//TY_LOGI("Unknown actionCode (action_validation)");
		return "Unknown actionCode";
	}

	return "Validation SUCCESS";
}

char *get_action(char *payload)
{
	cJSON *root = cJSON_Parse(payload);
	if (!root) {
		syslog(LOG_USER | LOG_INFO, "Invalid JSON");
		//TY_LOGI("Invalid JSON (get_action)");
		return "";
	}

	cJSON *action_code = cJSON_GetObjectItem(root, "actionCode");
	if (!action_code) {
		syslog(LOG_USER | LOG_INFO, "Invalid actionCode (get_action)");
		//TY_LOGI("Invalid actionCode (get_action)");
		return "";
	}

	/// Check if actionCode is valid
	if (strcmp(action_code->valuestring, "set_on") != 0 &&
	    strcmp(action_code->valuestring, "set_off") != 0 &&
	    strcmp(action_code->valuestring, "list_devices") != 0) {
		syslog(LOG_USER | LOG_INFO, "Unknown actionCode (get_action)");
		//TY_LOGI("Unknown actionCode (get_action)");
		return "";
	} else {
		return action_code->valuestring;
	}
}

int execute_action(tuya_mqtt_context_t *context, char *payload)
{
	char *action = get_action(payload);
	if (strcmp(action, "") == 0) {
		return 1;
	}

	syslog(LOG_USER | LOG_INFO, "Action: %s", action);

	int status     = 0;
	char *resp_msg = action_validation(payload, action, &status);

	syslog(LOG_USER | LOG_INFO, "Action validation: %s", resp_msg);
	syslog(LOG_USER | LOG_INFO, "Action validation status: %d", status);

	if (status != 0) {
		action_response(context, status, resp_msg, action);
		return 1;
	}

	resp_msg = execute(payload, action, &status);

	syslog(LOG_USER | LOG_INFO, "Action execution: %s", resp_msg);
	syslog(LOG_USER | LOG_INFO, "Action execution status: %d", status);

	action_response(context, status, resp_msg, action);

	if (strcmp(action, "list_devices") == 0) {
		free(resp_msg);
	}

	return 0;
}

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE:
		syslog(LOG_USER | LOG_INFO, "Execute action received: %s", msg->data_string);
		//TY_LOGI("Execute action received: %s", msg->data_string);

		int ret = execute_action(context, msg->data_string);
		if (ret) {
			syslog(LOG_USER | LOG_ERR, "Execute action failed");
			//TY_LOGE("Execute action failed");
			break;
		}
		syslog(LOG_USER | LOG_INFO, "Execute action success");
		//TY_LOGI("Execute action success");
		break;

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

	// ubus initialization
	ctx = NULL;
	ubus_init(&ctx);

	ret = ubus_lookup_id(ctx, "esp_over_ubus", &id);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "Failed to look up esp_over_ubus id");
		//TY_LOGE("Failed to look up esp_over_ubus id");
		return ret;
	}
	syslog(LOG_USER | LOG_INFO, "ubus_lookup_id success");
	//TY_LOGI("ubus_lookup_id success");

	// tuya loop
	while (running) {
		ret = tuya_mqtt_loop(client);

		if (ret) {
			syslog(LOG_USER | LOG_INFO, "connection failed");
			//TY_LOGI("connection failed");
			return ret;
		}
	}

	// ubus deinitialization
	ubus_deinit(&ctx);
	return OPRT_OK;
}
