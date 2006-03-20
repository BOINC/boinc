#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "dc_boinc.h"

/********************************************************************
 * Global variables
 */

DC_ResultCallback	_dc_resultcb;
DC_SubresultCallback	_dc_subresultcb;
DC_MessageCallback	_dc_messagecb;

static uuid_t project_uuid;
char project_uuid_str[37];

/* Must match the definition of WorkdirFile */
static const char *const workdir_prefixes[] =
{
	"in_", "out_", "ckpt", "dc_"
};


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

	if (!_DC_getCfgStr(CFG_PROJECTROOT))
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_PROJECTROOT);
		return DC_ERR_CONFIG;
	}

	/* Check & switch to the working directory */
	cfgval = _DC_getCfgStr(CFG_WORKDIR);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_WORKDIR);
		return DC_ERR_CONFIG;
	}

	/* If we are started as a BOINC daemon, then the current dir is
	 * <projectroot>/bin, so we must change to the working directory */
	if (chdir(cfgval))
	{
		DC_log(LOG_ERR, "Failed to set the current directory to %s: %s",
			cfgval, strerror(errno));
		return DC_ERR_CONFIG;
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

	/* Check the project's configuration */
	cfgval = _DC_getCfgStr(CFG_CONFIGXML);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not set in the config file",
			CFG_CONFIGXML);
		return DC_ERR_CONFIG;
	}

	ret = access(cfgval, R_OK);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to access the project's configuration "
			"at %s: %s", cfgval, strerror(errno));
		return DC_ERR_CONFIG;
	}

	ret = _DC_parseConfigXML(cfgval);
	if (ret)
		return ret;

	/* Now connect to the Boinc database */
	ret = _DC_initDB();
	if (ret)
		return ret;

	return 0;
}

int DC_getMaxMessageSize(void)
{
	return 32768; /* XXX */
}

int DC_getMaxSubresults(void)
{
	return 20; /* XXX */
}

DC_GridCapabilities DC_getGridCapabilities(void)
{
	return DC_GC_EXITCODE | DC_GC_SUBRESULT | DC_GC_MESSAGING;
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

