#include <sys/syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <dc_common.h>
#include <dc_internal.h>

#if CLIENT
#include <dc_client.h>
#endif

/* Set the log level to -1 so the first call to DC_log() will call init_log() */
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

	val = DC_getCfgStr(CFG_LOGLEVEL);
	if (val)
	{
		if (val[0] >= '0' && val[0] <= '9')
			loglevel = atoi(val);
		else
		{
			unsigned i;

			for (i = 0; i < sizeof(levels) / sizeof(levels[0]); i++)
			{
				if (levels[i] && !strcasecmp(levels[i], val))
				{
					loglevel = i;
					break;
				}
			}

			if (i >= sizeof(levels) / sizeof(levels[0]))
				fprintf(stderr, "WARNING: Unknown log level "
					"specified in the config file, "
					"using 'Notice'\n");
		}
		free(val);
	}

	val = DC_getCfgStr(CFG_LOGFILE);
	if (val)
	{
		char *tmp;

#if MASTER
		/* If the log file is not an absolute path, open it
		 * inside the working directory */
		if (val[0] != '/')
		{
			char *dir;

			dir = DC_getCfgStr(CFG_WORKDIR);
			if (dir)
			{
				tmp = malloc(strlen(dir) + 1 + strlen(val) + 1);
				sprintf(tmp, "%s/%s", dir, val);
				free(dir);
				free(val);
				val = tmp;
			}
		}
#endif /* MASTER */
#if CLIENT
		/* Try to resolve the file name first as an output file, and
		 * if that fails, as a temporary file. If both fail, use the
		 * file name verbatim. */
		tmp = DC_resolveFileName(DC_FILE_OUT, val);
		if (!tmp)
			tmp = DC_resolveFileName(DC_FILE_TMP, val);
		if (tmp)
		{
			free(val);
			val = tmp;
		}
#endif /* CLIENT */

		logfile = fopen(val, "a");
		if (!logfile)
		{
			fprintf(stderr, "Failed to open the log file %s: %s\n",
				val, strerror(errno));
			exit(1);
		}
		free(val);
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

	if (level >= 0 && level < (int)(sizeof(levels) / sizeof(levels[0])) && levels[level])
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
