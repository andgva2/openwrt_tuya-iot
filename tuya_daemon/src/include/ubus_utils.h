#ifndef UBUS_UTILS_H
#define UBUS_UTILS_H

#include <tuya_log.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include <syslog.h>
#include <string.h>

struct set_resp {
	int status;
	char *message;
};

int ubus_init(struct ubus_context **ctx);
void ubus_deinit(struct ubus_context **ctx);
void list_devices_cb(struct ubus_request *req, int type, struct blob_attr *msg);
void set_cb(struct ubus_request *req, int type, struct blob_attr *msg);

#endif /* UBUS_UTILS_H */