#include <tuya_utils.h>

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

	char execute_topic[64];
	sprintf(execute_topic, "tylink/%s/thing/action/execute", context->config.device_id);
	mqtt_client_unsubscribe(context->mqtt_client, execute_topic, MQTT_QOS_1);
}

// int tuyalink_thing_action_execute_response(tuya_mqtt_context_t *context, const char *device_id,
// 					   const char *data)
// {
// 	if (context == NULL || data == NULL) {
// 		return OPRT_INVALID_PARM;
// 	}

// 	tuyalink_message_t message = { .type	    = THING_TYPE_ACTION_EXECUTE_RSP,
// 				       .device_id   = (char *)device_id,
// 				       .data_string = (char *)data,
// 				       .ack	    = false };
// 	return tuyalink_message_send(context, &message);
// }

// void action_response(tuya_mqtt_context_t *context, int response_status, char *response_message,
// 		     char *action_code)
// {
// 	char payload[1024];
// 	if (strcmp(action_code, "set_on") == 0 || strcmp(action_code, "set_off") == 0) {
// 		/// Send action response (e.g. {"actionCode":"String","outputParams":{"response":Integer,"msg":"String"}}
// 		snprintf(payload, sizeof(payload),
// 			 "{\"actionCode\": \"%s\", \"outputParams\": {\"response\": %d, \"msg\": \"%s\"}}",
// 			 action_code, response_status, response_message);
// 		tuyalink_thing_action_execute_response(context, NULL, payload);
// 	} else {
// 		/// list_devices action
// 		/// Send action response (e.g. {"actionCode":"String","outputParams":{"device_list":"String"}}
// 		snprintf(payload, sizeof(payload), "{\"port_list\":[%s]}", response_message);
// 		tuyalink_thing_property_report(context, NULL, payload);
// 	}
// }

// char *execute(char *payload, char *action, int *status)
// {
// 	char *return_str;
// 	/// Parse inputParams and actionCode from payload (e.g. {"inputParams":{"pin":12,"port":"abc"},"actionCode":"set_on"})
// 	cJSON *root = cJSON_Parse(payload);
// 	if (!root) {
// 		*status = 1;
// 		syslog(LOG_USER | LOG_ERR, "Invalid JSON (execute)");
// 		//TY_LOGE("Invalid JSON (execute)");

// 		return_str = "Invalid JSON";
// 		goto cleanup;
// 	}

// 	cJSON *input_params = cJSON_GetObjectItem(root, "inputParams");
// 	if (!input_params) {
// 		*status = 1;
// 		syslog(LOG_USER | LOG_ERR, "Invalid inputParams (execute)");
// 		//TY_LOGE("Invalid inputParams (execute)");

// 		return_str = "Invalid inputParams";
// 		goto cleanup;
// 	}

// 	if (strcmp(action, "set_on") == 0 || strcmp(action, "set_off") == 0) {
// 		/// Parse port and pin
// 		cJSON *pin = cJSON_GetObjectItem(input_params, "pin");
// 		if (!pin) {
// 			*status = 1;
// 			syslog(LOG_USER | LOG_ERR, "Invalid pin (execute)");
// 			//TY_LOGE("Invalid pin (execute)");

// 			return_str = "Invalid pin";
// 			goto cleanup;
// 		}

// 		cJSON *port = cJSON_GetObjectItem(input_params, "port");
// 		if (!port) {
// 			*status = 1;
// 			syslog(LOG_USER | LOG_ERR, "Invalid port (execute)");
// 			//TY_LOGE("Invalid port (execute)");

// 			return_str = "Invalid port";
// 			goto cleanup;
// 		}

// 		struct set_resp response = { 0 };

// 		static struct blob_buf b;
// 		blob_buf_init(&b, 0);
// 		blobmsg_add_string(&b, "port", port->valuestring);
// 		blobmsg_add_u32(&b, "pin", pin->valueint);
// 		ubus_invoke(ctx, id, action, b.head, set_cb, &response, 3000);
// 		blob_buf_free(&b);

// 		*status	   = response.status;
// 		return_str = response.message;
// 	} else {
// 		struct device_port port_list = { 0 };

// 		ubus_invoke(ctx, id, action, NULL, list_devices_cb, &port_list, 3000);

// 		char *port_list_str = NULL;
// 		port_list_str	    = malloc(sizeof(char) * 512);
// 		if (port_list_str == NULL) {
// 			*status = 1;
// 			syslog(LOG_USER | LOG_ERR, "Memory allocation failed (execute)");
// 			//TY_LOGE("Memory allocation failed (execute)");

// 			return_str = "Memory allocation failed";
// 			goto cleanup;
// 		}

// 		strcpy(port_list_str, "");

// 		for (int i = 0; i < port_list.port_count; i++) {
// 			if (i + 1 == port_list.port_count) {
// 				strcat(port_list_str, "\"");
// 				strcat(port_list_str, port_list.port_list[i]);
// 				strcat(port_list_str, "\"");
// 				free(port_list.port_list[i]);
// 			} else {
// 				strcat(port_list_str, "\"");
// 				strcat(port_list_str, port_list.port_list[i]);
// 				strcat(port_list_str, "\"");
// 				strcat(port_list_str, ",");
// 				free(port_list.port_list[i]);
// 			}
// 		}

// 		return_str = strdup(port_list_str);
// 		free(port_list_str);
// 		free(port_list.port_list);
// 		goto cleanup_duped;
// 	}

// cleanup:;
// 	cJSON_Delete(root);
// 	return strdup(return_str);
// cleanup_duped:;
// 	cJSON_Delete(root);
// 	return return_str;
// }

// char *action_validation(char *payload, char *action, int *status)
// {
// 	char *return_str;
// 	/// Parse inputParams and actionCode from payload (e.g. {"inputParams":{"pin":12,"port":"abc"},"actionCode":"set_on"})
// 	cJSON *root = cJSON_Parse(payload);
// 	if (!root) {
// 		*status = 1;
// 		syslog(LOG_USER | LOG_INFO, "Invalid JSON (action_validation)");
// 		//TY_LOGI("Invalid JSON (action_validation)");

// 		return_str = "Invalid JSON";
// 		goto cleanup;
// 	}

// 	cJSON *input_params = cJSON_GetObjectItem(root, "inputParams");
// 	if (!input_params) {
// 		*status = 1;
// 		syslog(LOG_USER | LOG_INFO, "Invalid inputParams (action_validation)");
// 		//TY_LOGI("Invalid inputParams (action_validation)");

// 		return_str = "Invalid inputParams";
// 		goto cleanup;
// 	}

// 	// Switch case like for action
// 	if (strcmp(action, "set_on") == 0 || strcmp(action, "set_off") == 0) {
// 		/// Check if inputParams is valid
// 		cJSON *pin = cJSON_GetObjectItem(input_params, "pin");
// 		if (!pin || pin->type != cJSON_Number || pin->valueint < 0 || pin->valueint > 16) {
// 			*status = 1;
// 			syslog(LOG_USER | LOG_INFO, "Invalid pin (action_validation)");
// 			//TY_LOGI("Invalid pin (action_validation)");

// 			return_str = "Invalid pin";
// 			goto cleanup;
// 		}

// 		cJSON *port = cJSON_GetObjectItem(input_params, "port");
// 		if (!port || port->type != cJSON_String) {
// 			*status = 1;
// 			syslog(LOG_USER | LOG_INFO, "Invalid port (action_validation)");
// 			//TY_LOGI("Invalid port (action_validation)");

// 			return_str = "Invalid port";
// 			goto cleanup;
// 		}

// 		struct device_port port_list = { 0 };
// 		ubus_invoke(ctx, id, "list_devices", NULL, list_devices_cb, &port_list, 3000);

// 		int contains = 0;
// 		for (int i = 0; i < port_list.port_count; i++) {
// 			if (strcmp(port_list.port_list[i], port->valuestring) == 0) {
// 				contains = 1;
// 			}
// 			free(port_list.port_list[i]);
// 		}
// 		free(port_list.port_list);
// 		if (!contains) {
// 			syslog(LOG_USER | LOG_INFO, "Invalid port (action_validation)");
// 			//TY_LOGI("Invalid port (action_validation)");
// 			*status = 1;

// 			return_str = "Invalid port";
// 			goto cleanup;
// 		}
// 	} else if (strcmp(action, "list_devices") == 0) {
// 		// Do nothing, no args, so no validation required
// 	} else {
// 		*status = 1;
// 		syslog(LOG_USER | LOG_INFO, "Unknown actionCode (action_validation)");
// 		//TY_LOGI("Unknown actionCode (action_validation)");

// 		return_str = "Unknown actionCode";
// 		goto cleanup;
// 	}

// cleanup:;
// 	cJSON_Delete(root);
// 	if (*status == 0) {
// 		return_str = "Validation SUCCESS";
// 	}
// 	return strdup(return_str);
// }

// char *get_action(char *payload)
// {
// 	char *ret;
// 	cJSON *root = cJSON_Parse(payload);
// 	if (!root) {
// 		syslog(LOG_USER | LOG_INFO, "Invalid JSON");
// 		//TY_LOGI("Invalid JSON (get_action)");

// 		ret = "";
// 		goto cleanup;
// 	}

// 	cJSON *action_code = cJSON_GetObjectItem(root, "actionCode");
// 	if (!action_code) {
// 		syslog(LOG_USER | LOG_INFO, "Invalid actionCode (get_action)");
// 		//TY_LOGI("Invalid actionCode (get_action)");

// 		ret = "";
// 		goto cleanup;
// 	}

// 	/// Check if actionCode is valid
// 	if (strcmp(action_code->valuestring, "set_on") != 0 &&
// 	    strcmp(action_code->valuestring, "set_off") != 0 &&
// 	    strcmp(action_code->valuestring, "list_devices") != 0) {
// 		syslog(LOG_USER | LOG_INFO, "Unknown actionCode (get_action)");
// 		//TY_LOGI("Unknown actionCode (get_action)");

// 		ret = "";
// 		goto cleanup;
// 	} else {
// 		ret = strdup(action_code->valuestring);
// 	}

// cleanup:;
// 	cJSON_Delete(root);
// 	return ret;
// }

// int execute_action(tuya_mqtt_context_t *context, char *payload)
// {
// 	if (!context || strlen(payload) == 0) {
// 		syslog(LOG_USER | LOG_ERR, "NULL passed to execute_action");
// 		return 1;
// 	}

// 	char *action = NULL;
// 	action	     = get_action(payload);
// 	if (!action || strlen(action) == 0) {
// 		syslog(LOG_USER | LOG_ERR, "Couldn't retrieve action");
// 		return 1;
// 	}

// 	syslog(LOG_USER | LOG_INFO, "Action: %s", action);

// 	int status	     = 0;
// 	char *valid_resp_msg = NULL;
// 	valid_resp_msg	     = action_validation(payload, action, &status);

// 	syslog(LOG_USER | LOG_INFO, "Action validation: %s", valid_resp_msg);
// 	syslog(LOG_USER | LOG_INFO, "Action validation status: %d", status);

// 	if (status != 0) {
// 		action_response(context, status, valid_resp_msg, action);
// 		free(valid_resp_msg);
// 		free(action);
// 		return 1;
// 	}
// 	free(valid_resp_msg);

// 	char *exec_resp_msg = NULL;
// 	exec_resp_msg	    = execute(payload, action, &status);

// 	syslog(LOG_USER | LOG_INFO, "Action execution: %s", exec_resp_msg);
// 	syslog(LOG_USER | LOG_INFO, "Action execution status: %d", status);

// 	action_response(context, status, exec_resp_msg, action);

// 	free(exec_resp_msg);
// 	free(action);

// 	return 0;
// }

void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	switch (msg->type) {
	// case THING_TYPE_ACTION_EXECUTE:
	// 	syslog(LOG_USER | LOG_INFO, "Execute action received: %s", msg->data_string);
	// 	//TY_LOGI("Execute action received: %s", msg->data_string);

	// 	int ret = execute_action(context, msg->data_string);
	// 	if (ret) {
	// 		syslog(LOG_USER | LOG_ERR, "Execute action failed");
	// 		//TY_LOGE("Execute action failed");
	// 		break;
	// 	}
	// 	syslog(LOG_USER | LOG_INFO, "Execute action success");
	// 	//TY_LOGI("Execute action success");
	// 	break;

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

		char **files = NULL;
		int files_count = 0
		get_lua_files("/path/to/dir", &files, &files_count);

	}

	return OPRT_OK;
}
