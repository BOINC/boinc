#include <stdio.h>
#include <stdarg.h>
#include <sys/syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cfg.h"
#include "dc.h"

static int loglevel = -1;
static FILE *logfile;

static const char *levels[] =
{
	[LOG_DEBUG] = "Debug",
	[LOG_INFO] = "Info",
	[LOG_NOTICE] = "Notice",
	[LOG_WARNING] = "Warning",
	[LOG_ERR] = "Error"
};

static void init_log(void)
{
	char *val;

	/* Default level */
	loglevel = LOG_NOTICE;

	val = dc_cfg_get("LogLevel");
	if (val)
	{
		if (val[0] >= '0' && val[0] <= '9')
			loglevel = atoi(val);
		else
		{
			int i;

			for (i = 0; i < (int)sizeof(levels) / sizeof(levels[0]); i++)
			{
				if (levels[i] && !strcasecmp(levels[i], val))
				{
					loglevel = i;
					break;
				}
			}
		}
	}

	val = dc_cfg_get("LogFile");
	if (val)
	{
		logfile = fopen(val, "a");
		if (!logfile)
		{
			fprintf(stderr, "Failed to open the log file %s: %s",
				val, strerror(errno));
			exit(1);
		}
	}
	else
		logfile = stdout;
}

void DC_log(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	DC_vlog(level, fmt, ap);
	va_end(ap);
}

void DC_vlog(int level, const char *fmt, va_list ap)
{
	const char *levstr;
	char timebuf[32];
	struct tm *tm;
	time_t now;

	if (loglevel < 0)
		init_log();
	if (level > loglevel)
		return;

	if (level >= 0 && level < sizeof(levels) / sizeof(levels[0]) && levels[level])
		levstr = levels[level];
	else
		levstr = "Unknown";

	now = time(NULL);
	tm = localtime(&now);
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);

	fprintf(logfile, "%s [%s] ", timebuf, levstr);
	vfprintf(logfile, fmt, ap);
	fprintf(logfile, "\n");
	fflush(logfile);
}
