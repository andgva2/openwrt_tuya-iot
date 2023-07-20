#include <sdk/tuyalink_core.h>
#include <sdk/mqtt_client_interface.h>

#include <sdk/system_interface.h>
#include <sdk/tuya_error_code.h>
#include <sdk/tuya_log.h>
#include <sdk/cJSON.h>

#include <sys/sysinfo.h>
#include <syslog.h>

void on_connected(tuya_mqtt_context_t *context, void *user_data);
void on_disconnect(tuya_mqtt_context_t *context, void *user_data);
void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg);
void send_data(tuya_mqtt_context_t *context, void *user_data);
int tuya_init(tuya_mqtt_context_t *client, const char *deviceId, const char *deviceSecret);
int tuya_deinit(tuya_mqtt_context_t *client);
int tuya_loop(tuya_mqtt_context_t *client);