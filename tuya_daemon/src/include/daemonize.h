#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <become_daemon.h>
#include <syslog.h>

extern int is_daemon;

int daemonize(int flag);

#endif /* DAEMONIZE_H */