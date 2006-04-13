/*
 * config. C - Wrapper code for accessing Boinc project configuration values
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sched_config.h>

#include "dc_boinc.h"

SCHED_CONFIG dc_boinc_config;

int _DC_parseConfigXML(const char *file)
{
	int ret;

	ret = dc_boinc_config.parse_config_file(file);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to parse Boinc project "
			"configuration file %s", file);
		return DC_ERR_CONFIG;
	}
	return 0;
}

char *_DC_getUploadDir(void)
{
	return dc_boinc_config.upload_dir;
}

char *_DC_getDownloadDir(void)
{
	return dc_boinc_config.download_dir;
}

int _DC_getUldlDirFanout(void)
{
	return dc_boinc_config.uldl_dir_fanout;
}

char *_DC_getDBName(void)
{
	return dc_boinc_config.db_name;
}

char *_DC_getDBHost(void)
{
	return dc_boinc_config.db_host;
}

char *_DC_getDBUser(void)
{
	return dc_boinc_config.db_user;
}

char *_DC_getDBPasswd(void)
{
	return dc_boinc_config.db_passwd;
}
