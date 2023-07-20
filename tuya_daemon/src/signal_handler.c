#include <utils/signal_handler.h>

extern int is_daemon;
volatile sig_atomic_t running = 1;

void signal_handler(int sig)
{
	TY_LOGI("signal %d received, stopping", sig);
	if (is_daemon) {
		syslog(LOG_USER | LOG_INFO, "signal %d received, stopping", sig);
	}
	running = 0;
}