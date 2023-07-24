#include <daemonize.h>

int daemonize(int flag)
{
	int ret;
        ret = become_daemon(flag);
	if (ret) {
		syslog(LOG_USER | LOG_ERR, "error starting tuya daemon");
		closelog();
	}
        return ret;
}
