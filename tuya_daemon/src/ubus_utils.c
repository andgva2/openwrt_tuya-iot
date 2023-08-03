#include <ubus_utils.h>

enum {
	DEVICES,
	__DEVICES_MAX,
};

enum {
	DEVICE_PORT,
	DEVICE_VID,
	DEVICE_PID,
	__DEVICE_INFO_MAX,
};

enum {
	SET_RESPONSE,
	SET_RESPONSE_MSG,
	__SET_RESPONSE_MAX,
};

static const struct blobmsg_policy devices_policy[__DEVICES_MAX] = {
	[DEVICES] = { .name = "Devices", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy device_info_policy[__DEVICE_INFO_MAX] = {
	[DEVICE_PORT] = { .name = "Port", .type = BLOBMSG_TYPE_STRING },
	[DEVICE_VID]  = { .name = "Vendor ID", .type = BLOBMSG_TYPE_STRING },
	[DEVICE_PID]  = { .name = "Product ID", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy set_response_policy[__SET_RESPONSE_MAX] = {
	[SET_RESPONSE]	   = { .name = "response", .type = BLOBMSG_TYPE_INT32 },
	[SET_RESPONSE_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_STRING },
};

void set_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct set_resp *response = (struct set_resp *)req->priv;
	struct blob_attr *tb[__SET_RESPONSE_MAX];

	blobmsg_parse(set_response_policy, __SET_RESPONSE_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[SET_RESPONSE_MSG] || !tb[SET_RESPONSE]) {
		syslog(LOG_USER | LOG_ERR, "No response received");
		//TY_LOGE("No response received");
		return;
	}

	response->status  = blobmsg_get_u32(tb[SET_RESPONSE]);
	response->message = blobmsg_get_string(tb[SET_RESPONSE_MSG]);
}

void list_devices_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct device_port *port_list = (struct device_port *)req->priv;
	port_list->port_list	      = malloc(16 * sizeof(*port_list->port_list));
	if (port_list->port_list == NULL) {
		syslog(LOG_USER | LOG_ERR, "Failed to allocate memory for port list");
		//TY_LOGE("Failed to allocate memory for port list");
		return;
	}

	struct blob_attr *tb[__DEVICES_MAX];

	blobmsg_parse(devices_policy, __DEVICES_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[DEVICES]) {
		syslog(LOG_USER | LOG_ERR, "No devices data received");
		//TY_LOGE("No devices data received");
		return;
	}

	struct blob_attr *cur;
	size_t rem;
	int count = 0;

	blobmsg_for_each_attr (cur, tb[DEVICES], rem) {
		if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE) {
			continue;
		}

		struct blob_attr *dev_cur;
		size_t dev_rem;

		blobmsg_for_each_attr (dev_cur, cur, dev_rem) {
			struct blob_attr *dev[__DEVICE_INFO_MAX];

			blobmsg_parse(device_info_policy, __DEVICE_INFO_MAX, dev, blob_data(msg),
				      blob_len(msg));

			if (blobmsg_type(dev_cur) == BLOBMSG_TYPE_STRING &&
			    strcmp(blobmsg_name(dev_cur), "Port") == 0) {
				port_list->port_list[count] = malloc(sizeof(char) * 32);
				if (port_list->port_list[count] == NULL) {
					syslog(LOG_USER | LOG_ERR, "malloc failed");
					//TY_LOGE("malloc failed");
					return;
				}

				strcpy(port_list->port_list[count], blobmsg_get_string(dev_cur));
			}
		}
		count++;
	}

	port_list->port_count = count;
}

int ubus_init(struct ubus_context **ctx)
{
	int ret;
	struct ubus_object tuya_daemon_client_object = { 0 };

	*ctx = ubus_connect(NULL);
	if (!*ctx) {
		syslog(LOG_USER | LOG_ERR, "failed to connect to ubus");
		//TY_LOGE("failed to connect to ubus");
		return -1;
	}

	ret = ubus_add_object(*ctx, &tuya_daemon_client_object);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "failed to add object: %s", ubus_strerror(ret));
		//TY_LOGE("failed to add object: %s", ubus_strerror(ret));
		return ret;
	}
	return 0;
}

void ubus_deinit(struct ubus_context **ctx)
{
	ubus_free(*ctx);
}