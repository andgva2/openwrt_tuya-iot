#ifndef TUYA_UTILS_H
#define TUYA_UTILS_H

#include <tuyalink_core.h>
#include <tuya_cacert.h>
#include <mqtt_client_interface.h>
#include <system_interface.h>
#include <tuya_error_code.h>
#include <tuya_log.h>
#include <cJSON.h>

#include <sys/sysinfo.h>
#include <syslog.h>
#include <stdio.h>

#include <daemonize.h>

void on_connected(tuya_mqtt_context_t *context, void *user_data);
void on_disconnect(tuya_mqtt_context_t *context, void *user_data);
void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);
void send_data(tuya_mqtt_context_t *context, char *user_data);
int tuya_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret);
int tuya_deinit(tuya_mqtt_context_t *client);
int tuya_loop(tuya_mqtt_context_t *client);

#endif