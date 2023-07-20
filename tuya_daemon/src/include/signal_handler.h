#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
#include <tuya_log.h>
#include <syslog.h>
#include <daemonize.h>

extern volatile sig_atomic_t running;

void signal_handler(int sig);

#endif /* SIGNAL_HANDLER_H */