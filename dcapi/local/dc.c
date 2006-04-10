#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "dc_local.h"

/********************************************************************
 * Global variables
 */

DC_ResultCallback	_dc_resultcb;
DC_SubresultCallback	_dc_subresultcb;
DC_MessageCallback	_dc_messagecb;

char project_uuid_str[37];
uuid_t project_uuid;
int sleep_interval;

/********************************************************************
 * API functions
 */

int DC_init(const char *config_file)
{
	const char *cfgval;
	int ret;

	if (!config_file)
		config_file = DC_CONFIG_FILE;

	ret = _DC_parseCfg(config_file);
	if (ret)
	{
		DC_log(LOG_ERR, "The DC-API cannot be initialized without a "
			"config file");
		return ret;
	}

	/* Check the working directory */
	cfgval = _DC_getCfgStr(CFG_WORKDIR);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_WORKDIR);
		return DC_ERR_CONFIG;
	}

	/* Check sleep interval */
	cfgval = _DC_getCfgStr(CFG_SLEEPINTERVAL);
	if (!cfgval)
	{
		DC_log(LOG_WARNING, "%s is not specified in the config file. Default value is %d sec",
			CFG_SLEEPINTERVAL, DEFAULT_SLEEP_INTERVAL);
		sleep_interval = DEFAULT_SLEEP_INTERVAL;
	}
	else
	{
		sleep_interval = atoi(cfgval);
		if (sleep_interval < 1) sleep_interval = 1;
	}

	/* Check the project UUID */
	cfgval = _DC_getCfgStr(CFG_INSTANCEUUID);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not set in the config file",
			CFG_INSTANCEUUID);
		return DC_ERR_CONFIG;
	}

	ret = uuid_parse((char *)cfgval, project_uuid);
	if (ret)
	{
		DC_log(LOG_ERR, "Invalid project UUID");
		return DC_ERR_CONFIG;
	}

	/* Enforce a canonical string representation of the UUID */
	uuid_unparse_lower(project_uuid, project_uuid_str);

	return DC_OK;
}

int DC_getMaxMessageSize(void)
{
	return MAX_MESSAGE_SIZE;
}

int DC_getMaxSubresults(void)
{
	return MAX_SUBRESULTS;
}

DC_GridCapabilities DC_getGridCapabilities(void)
{
	return (DC_GridCapabilities) (DC_GC_EXITCODE | DC_GC_SUBRESULT | DC_GC_MESSAGING);
}

void DC_setcb(DC_ResultCallback resultcb, DC_SubresultCallback subresultcb,
	DC_MessageCallback msgcb)
{
	_dc_resultcb = resultcb;
	_dc_subresultcb = subresultcb;
	_dc_messagecb = msgcb;
}

DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
	const char *path)
{
	DC_PhysicalFile *file;

	file = g_new(DC_PhysicalFile, 1);
	file->label = g_strdup(label);
	file->path = g_strdup(path);
	file->mode = DC_FILE_REGULAR;

	return file;
}

void _DC_destroyPhysicalFile(DC_PhysicalFile *file)
{
	if (!file)
		return;

	g_free(file->label);
	g_free(file->path);
	g_free(file);
}

