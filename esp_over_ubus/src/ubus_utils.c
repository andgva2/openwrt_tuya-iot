#include <ubus_utils.h>

static int device_list_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			  const char *method, struct blob_attr *msg);

static int set_on_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		     const char *method, struct blob_attr *msg);

static int set_off_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		      const char *method, struct blob_attr *msg);

enum {
	SET_PORT,
	SET_PIN,
	__SET_MAX,
};

static const struct blobmsg_policy set_policy[__SET_MAX] = {
	[SET_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
	[SET_PIN]  = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
};

static const struct ubus_method esp_over_ubus_methods[] = {
	UBUS_METHOD_NOARG("list_devices", device_list_cb),
	UBUS_METHOD("set_on", set_on_cb, set_policy),
	UBUS_METHOD("set_off", set_off_cb, set_policy),
};

static struct ubus_object_type esp_over_ubus_object_type =
	UBUS_OBJECT_TYPE("esp_over_ubus", esp_over_ubus_methods);

static struct ubus_object esp_over_ubus_object = {
	.name	   = "esp_over_ubus",
	.type	   = &esp_over_ubus_object_type,
	.methods   = esp_over_ubus_methods,
	.n_methods = ARRAY_SIZE(esp_over_ubus_methods),
};

static int device_list_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			  const char *method, struct blob_attr *msg)
{
	struct Device *deviceList = NULL;
	struct blob_buf buf	  = {};
	blob_buf_init(&buf, 0);

	void *list_cookie = blobmsg_open_array(&buf, "Devices");
	deviceList	  = get_device_list();
	if (deviceList == NULL) {
		goto cleanup;
	}

	struct Device *current = deviceList;

	while (current != NULL) {
		void *device_cookie = blobmsg_open_table(&buf, NULL);
		
		char *malloced_device_name = get_device_name(&current);
		blobmsg_add_string(&buf, "Name", malloced_device_name);
		free(malloced_device_name);
		blobmsg_add_string(&buf, "Port", current->port);
		blobmsg_add_string(&buf, "Vendor ID", current->vid);
		blobmsg_add_string(&buf, "Product ID", current->pid);

		current = current->next;
		blobmsg_close_table(&buf, device_cookie);
	}

cleanup:;
	blobmsg_close_array(&buf, list_cookie);
	ubus_send_reply(ctx, req, buf.head);
	free_device_list(&deviceList);
	blob_buf_free(&buf);

	return 0;
}

int deserialize_resp_msg(char *serialized_msg, int *resp, char **msg)
{
	int ret = 0;
	*msg	= malloc(sizeof(char) * 300);

	json_t *root;
	json_error_t error;
	root = json_loads(serialized_msg, 0, &error);
	if (!root) {
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		syslog(LOG_USER | LOG_ERR, "error: on line %d: %s\n", error.line, error.text);
		ret = 1;
		goto exit;
	}
	if (!json_is_object(root)) {
		fprintf(stderr, "error: root is not an object\n");
		syslog(LOG_USER | LOG_ERR, "error: root is not an object\n");
		json_decref(root);
		ret = 1;
		goto exit;
	}

	json_t *response, *resp_message;
	response = json_object_get(root, "response");
	if (!json_is_integer(response)) {
		fprintf(stderr, "error: response is not an integer\n");
		syslog(LOG_USER | LOG_ERR, "error: response is not an integer\n");
		json_decref(root);
		ret = 1;
		goto exit;
	}
	resp_message = json_object_get(root, "msg");
	if (!json_is_string(resp_message)) {
		fprintf(stderr, "error: msg is not a string\n");
		syslog(LOG_USER | LOG_ERR, "error: msg is not a string\n");
		json_decref(root);
		ret = 1;
		goto exit;
	}

	*resp = json_integer_value(response);
	strcpy(*msg, json_string_value(resp_message));
	json_decref(root);
exit:;
	return ret;
}

static int set_on_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		     const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__SET_MAX];
	struct blob_buf buf = {};
	blob_buf_init(&buf, 0);

	blobmsg_parse(set_policy, __SET_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[SET_PORT] || !tb[SET_PIN]) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	struct Device *deviceList = get_device_list();
	int result		  = 0;
	if (deviceList == NULL) {
		goto cleanup_device_list;
	}

	struct Device *current = deviceList;
	int is_found	       = 0;
	while (current != NULL) {
		if (strcmp(current->port, blobmsg_get_string(tb[SET_PORT])) == 0) {
			is_found = 1;
			break;
		}

		is_found = 0;
		current	 = current->next;
	}

	if (is_found == 0) {
		goto cleanup_device_list;
	}

	char message[300];

	snprintf(message, sizeof(message), "{\"action\": \"on\", \"pin\": %d}", blobmsg_get_u32(tb[SET_PIN]));

	result = send_to_device(&current, message);

	char *retrieved = receive_from_device(&current, 300);
	fprintf(stdout, "%s", retrieved);
	if (strcmp(retrieved, "") == 0) {
		goto cleanup_response;
	}

	int response;
	char *resp_message;

	deserialize_resp_msg(retrieved, &response, &resp_message);

	blobmsg_add_u32(&buf, "response", response);
	blobmsg_add_string(&buf, "msg", resp_message);

	ubus_send_reply(ctx, req, buf.head);

cleanup_response:;
	free(retrieved);
	free(resp_message);
cleanup_device_list:;
	blob_buf_free(&buf);
	free_device_list(&deviceList);

	return result;
}

static int set_off_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		      const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__SET_MAX];
	struct blob_buf buf = {};
	blob_buf_init(&buf, 0);

	blobmsg_parse(set_policy, __SET_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[SET_PORT] || !tb[SET_PIN]) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	struct Device *deviceList = get_device_list();
	int result		  = 0;
	if (deviceList == NULL) {
		goto cleanup_device_list;
	}

	struct Device *current = deviceList;
	int is_found	       = 0;
	while (current != NULL) {
		if (strcmp(current->port, blobmsg_get_string(tb[SET_PORT])) == 0) {
			is_found = 1;
			break;
		}

		is_found = 0;
		current	 = current->next;
	}

	if (is_found == 0) {
		goto cleanup_device_list;
	}

	char message[300];

	snprintf(message, sizeof(message), "{\"action\": \"off\", \"pin\": %d}",
		 blobmsg_get_u32(tb[SET_PIN]));

	result = send_to_device(&current, message);

	char *retrieved = receive_from_device(&current, 300);
	fprintf(stdout, "%s", retrieved);
	if (strcmp(retrieved, "") == 0) {
		result = 1;
		goto cleanup_response;
	}

	int response;
	char *resp_message;

	deserialize_resp_msg(retrieved, &response, &resp_message);

	blobmsg_add_u32(&buf, "response", response);
	blobmsg_add_string(&buf, "msg", resp_message);

	ubus_send_reply(ctx, req, buf.head);

cleanup_response:;
	free(retrieved);
	free(resp_message);
cleanup_device_list:;
	blob_buf_free(&buf);
	free_device_list(&deviceList);

	return result;
}

void server_main(struct ubus_context *ctx)
{
	int ret;

	ret = ubus_add_object(ctx, &esp_over_ubus_object);
	if (ret) {
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
		syslog(LOG_USER | LOG_ERR, "Failed to add object: %s\n", ubus_strerror(ret));
	}

	uloop_run();
}