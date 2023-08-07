#ifndef TUYA_UTILS_H
#define TUYA_UTILS_H

#include <tuyalink_core.h>
#include <mqtt_client_interface.h>
#include <system_interface.h>
#include <tuya_error_code.h>
#include <tuya_log.h>
#include <cJSON.h>

#include <sys/sysinfo.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <daemonize.h>
#include <script_utils.h>

void signal_handler(int sig);
int tuya_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret);
int tuya_deinit(tuya_mqtt_context_t *client);
int tuya_loop(tuya_mqtt_context_t *client);

extern const char tuya_cacert_pem[1368];

#endif
