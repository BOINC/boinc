/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "dc.h"

#include "condor_master.h"
#include "condor_common.h"
#include "condor_defs.h"
#include "condor_wu.h"
#include "condor_log.h"
#include "condor_utils.h"


/********************************************************************* INIT */

static GHashTable *_DC_wu_table= NULL;
uuid_t _DC_project_uuid;
char _DC_project_uuid_str[37]= "";

DC_ResultCallback	_DC_result_callback= NULL;
DC_SubresultCallback	_DC_subresult_callback= NULL;
DC_MessageCallback	_DC_message_callback= NULL;


/* Initializes the DC-API. */
int
DC_initMaster(const char *configFile)
{
	int ret;
	char *cfgval= NULL;

	if (!configFile)
		configFile= DC_CONFIG_FILE;
	ret= _DC_parseCfg(configFile);
	if (ret)
	{
		DC_log(LOG_ERR, "DC-API config file (%s) parse error",
		       configFile);
		return(ret);
	}
	DC_log(LOG_DEBUG, "DC_initMaster(%s)", configFile);

	if (!_DC_wu_table)
		_DC_wu_table= g_hash_table_new_full(g_str_hash,
						    g_str_equal,
						    NULL,
						    NULL);

	cfgval= DC_getCfgStr(CFG_INSTANCEUUID);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "Setting of %s is missing from config file %s",
		       CFG_INSTANCEUUID, configFile);
		return(DC_ERR_CONFIG);
	}
	ret= uuid_parse((char *)cfgval, _DC_project_uuid);
	if (ret)
	{
		DC_log(LOG_ERR, "Invalid project UUID");
		g_free(cfgval);
		return(DC_ERR_CONFIG);
	}
	g_free(cfgval);

	/* Enforce a canonical string representation of the UUID */
	uuid_unparse_lower(_DC_project_uuid, _DC_project_uuid_str);

	return(0);
}


/****************************************************** Manage WU structure */

/* Creates one work unit. */
DC_Workunit *
DC_createWU(const char *clientName,
	    const char *arguments[], int subresults, const char *tag)
{
	DC_Workunit *wu;
	char uuid_str[37];
	char *cfgval;
	GString *str;
	int ret;

	wu= g_new0(DC_Workunit, 1);
	_DC_wu_changed(wu);

	_DC_wu_set_client_name(wu, clientName);

	wu->argv= g_strdupv((char **) arguments);
	for (wu->argc= 0; arguments && arguments[wu->argc]; wu->argc++)
		;
	wu->subresults= subresults;
	wu->tag= g_strdup(tag);

	uuid_generate(wu->uuid);
	uuid_unparse_lower(wu->uuid, uuid_str);
	wu->uuid_str= g_strdup(uuid_str);

	if (tag)
		wu->name= g_strdup_printf("%s_%s_%s", _DC_project_uuid_str,
					  uuid_str, tag);
	else
		wu->name= g_strdup_printf("%s_%s", _DC_project_uuid_str,
					  uuid_str);

	/* Calculate & create the working directory. The working directory
	 * has the form:
	 * <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid>
	 * Where <hash> is the first 2 hex digits of the uuid
	 */
	cfgval= DC_getCfgStr(CFG_WORKDIR);
	str= g_string_new(cfgval);
	free(cfgval);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, ".dcapi-");
	g_string_append(str, _DC_project_uuid_str);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append_printf(str, "%02x", wu->uuid[0]);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, uuid_str);

	ret= _DC_mkdir_with_parents(str->str, 0700);
	if (ret)
	{
		DC_log(LOG_ERR,
		       "Failed to create WU working directory %s: %s",
		       str->str, strerror(errno));
		DC_destroyWU(wu);
		return NULL;
	}

	wu->workdir= str->str;
	g_string_free(str, FALSE);

	if (!_DC_wu_table)
		DC_initMaster(NULL);
	g_hash_table_insert(_DC_wu_table, wu->name, wu);

	wu->condor_events= g_array_new(FALSE, FALSE,
				       sizeof(struct _DC_condor_event));
	_DC_wu_make_client_executables(wu);
	wu->state= DC_WU_READY;

	return(wu);
}


/* Releases internal resources allocated to a work unit. */
void
DC_destroyWU(DC_Workunit * wu)
{
	if (!_DC_wu_check(wu))
		return;

	if (_DC_wu_table)
		g_hash_table_remove(_DC_wu_table, wu->name);

	if (wu->workdir)
	{
		const char *name;
		GDir *dir;
		int ret;
		GString *fn;

		/* Removing generated files */
		fn= g_string_new(wu->workdir);
		fn= g_string_append(fn, "/condor_submit.txt");
		unlink(fn->str);
		g_string_free(fn, TRUE);

		dir= g_dir_open(wu->workdir, 0, NULL);
		/* The work directory should not contain any extra files, but
		 * just in case */
		while (dir &&
		       (name= g_dir_read_name(dir)))
		{
			GString *str= g_string_new(wu->workdir);
			g_string_append_c(str, G_DIR_SEPARATOR);
			g_string_append(str, name);
			DC_log(LOG_INFO, "Removing unknown file %s",
			       str->str);
			unlink(str->str);
			g_string_free(str, TRUE);
		}
		if (dir)
			g_dir_close(dir);

		ret= rmdir(wu->workdir);
		if (ret)
			DC_log(LOG_WARNING, "Failed to remove WU working "
			       "directory %s: %s", wu->workdir,
			       strerror(errno));
		g_free(wu->workdir);
	}

	_DC_wu_set_client_name(wu, NULL);
	g_free(wu->uuid_str);
	g_strfreev(wu->argv);
	g_free(wu->tag);
	g_free(wu->name);
	g_array_free(wu->condor_events, TRUE);
	g_free(wu);
}


/* Sets an input file for the work unit. */
int
DC_addWUInput(DC_Workunit * wu,
	      const char *logicalFileName,
	      const char *URL, DC_FileMode fileMode)
{
	DC_PhysicalFile *file;
	char *workpath;
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	/* Sanity checks */
	ret= _DC_wu_check_logical_name(wu, logicalFileName);
	if (ret)
		return(ret);
	workpath= _DC_wu_get_workdir_path(wu, logicalFileName, FILE_IN);
	file= _DC_createPhysicalFile(logicalFileName, workpath);
	g_free(workpath);
	if (!file)
		return(DC_ERR_INTERNAL);

	switch (fileMode)
	{
	case DC_FILE_REGULAR:
		DC_log(LOG_DEBUG, "Copying regular file %s to %s",
		       URL, file->path);
		ret= _DC_copyFile(URL, file->path);
		if (ret)
		{
			_DC_destroyPhysicalFile(file);
			return(ret/*DC_ERR_BADPARAM*/);	/* XXX */
		}
		break;
	case DC_FILE_PERSISTENT:
		DC_log(LOG_DEBUG, "Copying persistent file %s to %s",
		       URL, file->path);
		ret= _DC_copyFile(URL, file->path);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to link %s to %s: %s",
			       URL, file->path, strerror(errno));
			_DC_destroyPhysicalFile(file);
			return(DC_ERR_BADPARAM);	/* XXX */
		}
		/* Remember the file mode */
		file->mode= DC_FILE_PERSISTENT;
		break;
	case DC_FILE_VOLATILE:
		DC_log(LOG_DEBUG, "Renaming %s to %s",
		       URL, file->path);
		ret= rename(URL, file->path);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to rename %s to %s: %s",
			       URL, file->path, strerror(errno));
			_DC_destroyPhysicalFile(file);
			return(DC_ERR_BADPARAM);
		}
		break;
	}

	wu->input_files= g_list_append(wu->input_files, file);
	/*wu->num_inputs++; */

	return(0);
}


/* Defines an output file for the work unit. */
int
DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName)
{
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	/* Sanity checks */
	ret= _DC_wu_check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;
	DC_log(LOG_DEBUG, "Adding out file %s",
	       logicalFileName);
	wu->output_files= g_list_append(wu->output_files,
					g_strdup(logicalFileName));
	/*wu->num_outputs++;*/
	return(0);
}


/* Sets the priority for the work unit. */
int
DC_setWUPriority(DC_Workunit *wu, int priority)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	return(0);
}


/* Sets the callback functions that will be called when a particular event. */
void
DC_setMasterCb(DC_ResultCallback resultcb,
	       DC_SubresultCallback subresultcb,
	       DC_MessageCallback msgcb)
{
	_DC_result_callback= resultcb;
	_DC_subresult_callback= subresultcb;
	_DC_message_callback= msgcb;
}


/* Queries the state of a work unit. */
DC_WUState
DC_getWUState(DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(DC_WU_UNKNOWN);
	return(wu->state);
}


/* Queries the low-level ID of the work unit. */
char *
DC_getWUId(const DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(NULL);
	return(NULL);
}


/* Queries the tag of a work unit. */
char *
DC_getWUTag(const DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(NULL);
	return(wu->tag);
}


/* Serializes a work unit description. */
char *
DC_serializeWU(DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(NULL);
	return(0);
}


/* Restores a serialized work unit. */
DC_Workunit *
DC_deserializeWU(const char *buf)
{
	return(0);
}


/* iterator for DC_getWUNumber() */
static DC_WUState _DC_dd_look_for_state;
static void _DC_dd_check_state(void *key, void *value, void *ptr)
{
	DC_Workunit *wu=(DC_Workunit *)value;
	int *count= (int *)ptr;
	if (wu->state == _DC_dd_look_for_state)
		++(*count);
}

/* Queries the number of WUs known to the API in the given state. */
int
DC_getWUNumber(DC_WUState state)
{
	int val;

	_DC_dd_look_for_state= state;
	g_hash_table_foreach(_DC_wu_table, (GHFunc)_DC_dd_check_state, &val);
	return(val);
}


/************************************************************** Main cycles */

/* Waits for events and processes them. */
int
DC_processMasterEvents(int timeout)
{
	return(0);
}


/* Checks for events and return them. */
DC_MasterEvent *
DC_waitMasterEvent(const char *wuFilter, int timeout)
{
	return(0);
}


/* Checks for events for a particular WU. */
DC_MasterEvent *
DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	time_t start, now;
	int events;

	if (!_DC_wu_check(wu))
		return(NULL);

	events= wu->condor_events->len;
	start= time(NULL);
	now= start;
	while (now-start <= timeout)
	{
		int e;

		_DC_wu_update_condor_events(wu);
		e= wu->condor_events->len;
		if (e != events)
		{
			DC_log(LOG_DEBUG, "%d condor events occured during "
			       "waitWUEvent",
			       e-events);
			return(0);
		}
		sleep(1);
	}
	return(0);
}


/* Destroys an event. */
void
DC_destroyMasterEvent(DC_MasterEvent *event)
{
}


/**************************************************************** Messaging */

/* Sends a message to a running work unit. */
int
DC_sendWUMessage(DC_Workunit *wu, const char *message)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	return(DC_ERR_NOTIMPL);
}


/********************************************************** Result handling */

/* Queries what optional fields are present in the result. */
unsigned
DC_getResultCapabilities(const DC_Result *result)
{
	return(DC_ERR_NOTIMPL);
}


/* Returns the WU that generated this result. */
DC_Workunit *
DC_getResultWU(DC_Result *result)
{
	return(NULL);
}


/* Returns the exit code of the client application. */
int
DC_getResultExit(const DC_Result *result)
{
	return(DC_ERR_NOTIMPL);
}


/* Returns the local name of an output file. */
char *
DC_getResultOutput(const DC_Result *result, const char *logicalFileName)
{
	return(NULL);
}


/* End of condor/condor_master.c */
