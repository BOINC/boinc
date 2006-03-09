/*
 * Wrapper code for accessing Boinc project configuration values (config.xml)
 * from C
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sched_config.h>

#include "dc_boinc.h"

static SCHED_CONFIG config;

int _DC_parseConfigXML(const char *file)
{
	int ret;

	ret = config.parse_config_file(file);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to parse Boinc project "
			"configuration file %s", file);
		return DC_ERR_CONFIG;
	}
	return 0;
}

const char *_DC_getUploadDir(void)
{
	return config.upload_dir;
}

int _DC_getUldlDirFanout(void)
{
	return config.uldl_dir_fanout;
}

const char *_DC_getDBName(void)
{
	return config.db_name;
}

const char *_DC_getDBHost(void)
{
	return config.db_host;
}

const char *_DC_getDBUser(void)
{
	return config.db_user;
}

const char *_DC_getDBPasswd(void)
{
	return config.db_passwd;
}
