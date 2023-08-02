#ifndef UBUS_UTILS_H
#define UBUS_UTILS_H

#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <jansson.h>

#include <device_utils.h>

void server_main(struct ubus_context *ctx);

#endif // UBUS_UTILS_H