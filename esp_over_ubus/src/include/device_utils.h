#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libserialport.h>

#define VENDOR_ID  "10C4"
#define PRODUCT_ID "EA60"

struct Device {
	char port[30];
	char vid[30];
	char pid[30];
	struct Device *next;
};

char *get_device_name(struct Device **device);
struct Device *get_device_list(void);
void free_device_list(struct Device **device_list);
int send_to_device(struct Device **device, char *message);
char *receive_from_device(struct Device **device, int size);

#endif // DEVICE_INFO_H