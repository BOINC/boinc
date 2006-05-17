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


/********************************************************************* INIT */

static GHashTable *wu_table = NULL;
uuid_t project_uuid;
char project_uuid_str[37] = "";

/* Initializes the DC-API. */
int
DC_init (const char *configFile)
{
	int ret;
	char *cfgval = NULL;

	if (!wu_table)
		wu_table =
			g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
					       NULL);

	ret = uuid_parse ((char *) cfgval, project_uuid);
	if (ret)
	{
		DC_log (LOG_ERR, "Invalid project UUID");
		g_free (cfgval);
		return (DC_ERR_CONFIG);
	}
	g_free (cfgval);

	/* Enforce a canonical string representation of the UUID */
	uuid_unparse_lower (project_uuid, project_uuid_str);

	return (0);
}


/****************************************************** Manage WU structure */

/* Creates one work unit. */
DC_Workunit *
DC_createWU (const char *clientName,
	     const char *arguments[], int subresults, const char *tag)
{
	DC_Workunit *wu;
	char uuid_str[37];
	char *cfgval;
	GString *str;
	int ret;

	wu = g_new0 (DC_Workunit, 1);

	wu->argv = g_strdupv ((char **) arguments);
	for (wu->argc = 0; arguments && arguments[wu->argc]; wu->argc++)
		;
	wu->subresults = subresults;
	wu->tag = g_strdup (tag);

	uuid_generate (wu->uuid);
	uuid_unparse_lower (wu->uuid, uuid_str);
	wu->uuid_str = g_strdup (uuid_str);

	if (tag)
		wu->name =
			g_strdup_printf ("%s_%s_%s", project_uuid_str,
					 uuid_str, tag);
	else
		wu->name =
			g_strdup_printf ("%s_%s", project_uuid_str, uuid_str);

	/* Calculate & create the working directory. The working directory
	 * has the form:
	 * <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid>
	 * Where <hash> is the first 2 hex digits of the uuid
	 */
	cfgval = DC_getCfgStr (CFG_WORKDIR);
	str = g_string_new (cfgval);
	free (cfgval);
	g_string_append_c (str, G_DIR_SEPARATOR);
	g_string_append (str, ".dcapi-");
	g_string_append (str, project_uuid_str);
	g_string_append_c (str, G_DIR_SEPARATOR);
	g_string_append_printf (str, "%02x", wu->uuid[0]);
	g_string_append_c (str, G_DIR_SEPARATOR);
	g_string_append (str, uuid_str);

	ret = g_mkdir_with_parents (str->str, 0700);
	if (ret)
	{
		DC_log (LOG_ERR,
			"Failed to create WU working directory %s: %s",
			str->str, strerror (errno));
		DC_destroyWU (wu);
		return NULL;
	}

	wu->workdir = str->str;
	g_string_free (str, FALSE);

	if (!wu_table)
		DC_init (NULL);
	g_hash_table_insert (wu_table, wu->name, wu);

	return (wu);
}


/* Releases internal resources allocated to a work unit. */
void
DC_destroyWU (DC_Workunit * wu)
{
	if (!wu)
		return;

	if (wu_table)
		g_hash_table_remove (wu_table, wu->name);

	if (wu->workdir)
	{
		const char *name;
		GDir *dir;
		int ret;

		dir = g_dir_open (wu->workdir, 0, NULL);
		/* The work directory should not contain any extra files, but
		 * just in case */
		while (dir && (name = g_dir_read_name (dir)))
		{
			GString *str = g_string_new (wu->workdir);
			g_string_append_c (str, G_DIR_SEPARATOR);
			g_string_append (str, name);
			DC_log (LOG_INFO, "Removing unknown file %s",
				str->str);
			unlink (str->str);
			g_string_free (str, TRUE);
		}
		if (dir)
			g_dir_close (dir);

		ret = rmdir (wu->workdir);
		if (ret)
			DC_log (LOG_WARNING, "Failed to remove WU working "
				"directory %s: %s", wu->workdir,
				strerror (errno));
		g_free (wu->workdir);
	}

	g_free (wu->client_name);
	g_free (wu->uuid_str);
	g_strfreev (wu->argv);
	g_free (wu->tag);
	g_free (wu->name);

	g_free (wu);
}


/* Sets an input file for the work unit. */
int
DC_addWUInput (DC_Workunit * wu,
	       const char *logicalFileName,
	       const char *URL, DC_FileMode fileMode)
{
	DC_PhysicalFile *file;
	char *workpath;
	int ret;

	/* Sanity checks */
	ret = wu_check_logical_name (wu, logicalFileName);
	if (ret)
		return (ret);

	workpath = wu_get_workdir_path (wu, logicalFileName, FILE_IN);
	file = _DC_createPhysicalFile (logicalFileName, workpath);
	g_free (workpath);
	if (!file)
		return (DC_ERR_INTERNAL);

	switch (fileMode)
	{
	case DC_FILE_REGULAR:
		ret = copy_file (URL, file->path);
		if (ret)
		{
			_DC_destroyPhysicalFile (file);
			return (DC_ERR_BADPARAM);	/* XXX */
		}
		break;
	case DC_FILE_PERSISTENT:
		ret = link (URL, file->path);
		if (ret)
		{
			DC_log (LOG_ERR, "Failed to link %s to %s: %s",
				URL, file->path, strerror (errno));
			_DC_destroyPhysicalFile (file);
			return (DC_ERR_BADPARAM);	/* XXX */
		}
		/* Remember the file mode */
		file->mode = DC_FILE_PERSISTENT;
		break;
	case DC_FILE_VOLATILE:
		ret = rename (URL, file->path);
		if (ret)
		{
			DC_log (LOG_ERR, "Failed to rename %s to %s: %s",
				URL, file->path, strerror (errno));
			_DC_destroyPhysicalFile (file);
			return (DC_ERR_BADPARAM);
		}
		break;
	}

	wu->input_files = g_list_append (wu->input_files, file);
	/*wu->num_inputs++; */

	return (0);
}


/* Defines an output file for the work unit. */
int
DC_addWUOutput (DC_Workunit * wu, const char *logicalFileName)
{
	return (0);
}


/* Sets the priority for the work unit. */
int
DC_setWUPriority (DC_Workunit * wu, int priority)
{
	return (0);
}


/* Sets the callback functions that will be called when a particular event. */
void
DC_setcb (DC_ResultCallback resultcb,
	  DC_SubresultCallback subresultcb, DC_MessageCallback msgcb)
{
}


/* Queries the state of a work unit. */
DC_WUState
DC_getWUState (DC_Workunit * wu)
{
	return (DC_WU_UNKNOWN);
}


/* Queries the low-level ID of the work unit. */
char *
DC_getWUId (const DC_Workunit * wu)
{
	return (0);
}


/* Queries the tag of a work unit. */
char *
DC_getWUTag (const DC_Workunit * wu)
{
	return (0);
}


/* Serializes a work unit description. */
char *
DC_serializeWU (DC_Workunit * wu)
{
	return (0);
}


/* Restores a serialized work unit. */
DC_Workunit *
DC_deserializeWU (const char *buf)
{
	return (0);
}


/* Queries the number of WUs known to the API in the given state. */
int
DC_getWUNumber (DC_WUState state)
{
	return (0);
}


/************************************************************** Main cycles */

/* Waits for events and processes them. */
int
DC_processEvents (int timeout)
{
	return (0);
}


/* Checks for events and return them. */
DC_Event *
DC_waitEvent (const char *wuFilter, int timeout)
{
	return (0);
}


/* Checks for events for a particular WU. */
DC_Event *
DC_waitWUEvent (DC_Workunit * wu, int timeout)
{
	return (0);
}


/* Destroys an event. */
void
DC_destroyEvent (DC_Event * event)
{
}


/*************************************************************** Manage WUs */

/* Submits a work unit. */
int
DC_submitWU (DC_Workunit * wu)
{
	return (0);
}


/* Cancels all computations for a given work unit. */
int
DC_cancelWU (DC_Workunit * wu)
{
	return (0);
}


/* Temporarily suspends the execution of a work unit. */
int
DC_suspendWU (DC_Workunit * wu)
{
	return (0);
}


/* Resumes computation of a previously suspended work unit. */
int
DC_resumeWU (DC_Workunit * wu)
{
	return (0);
}


/**************************************************************** Messaging */

/* Sends a message to a running work unit. */
int
DC_sendWUMessage (DC_Workunit * wu, const char *message)
{
	return (0);
}


/********************************************************** Result handling */

/* Queries what optional fields are present in the result. */
unsigned
DC_getResultCapabilities (const DC_Result * result)
{
	return (0);
}


/* Returns the WU that generated this result. */
DC_Workunit *
DC_getResultWU (DC_Result * result)
{
	return (0);
}


/* Returns the exit code of the client application. */
int
DC_getResultExit (const DC_Result * result)
{
	return (0);
}


/* Returns the local name of an output file. */
char *
DC_getResultOutput (const DC_Result * result, const char *logicalFileName)
{
	return (0);
}


/****************************************************************************/

DC_PhysicalFile *
_DC_createPhysicalFile (const char *label, const char *path)
{
	DC_PhysicalFile *file;

	file = g_new (DC_PhysicalFile, 1);
	file->label = g_strdup (label);
	file->path = g_strdup (path);
	file->mode = DC_FILE_REGULAR;

	return (file);
}


void
_DC_destroyPhysicalFile (DC_PhysicalFile * file)
{
	if (!file)
		return;

	g_free (file->label);
	g_free (file->path);
	g_free (file);
}


/* End of condor/condor_master.cc */
