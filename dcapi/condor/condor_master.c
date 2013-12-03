/*
 * condor/condor_master.c
 *
 * DC-API functions of master side
 *
 * (c) Daniel Drotos, 2006
 */


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "dc.h"

#include "condor_master.h"
#include "condor_common.h"
#include "condor_defs.h"
#include "condor_wu.h"
#include "condor_log.h"
#include "condor_utils.h"
#include "condor_event.h"


/* some utils */

static struct {
	DC_WUState state;
	char *name;
}
_DC_state_names[]= {
	{ DC_WU_READY, "READY" },
	{ DC_WU_RUNNING, "RUNNING" },
	{ DC_WU_FINISHED, "FINISHED" },
	{ DC_WU_SUSPENDED, "SUSPENDED" },
	{ DC_WU_ABORTED, "ABORTED" },
	{ DC_WU_UNKNOWN, "UNKNOWN" },
	{ 0, NULL }
};


char *
_DC_state_name(DC_WUState state)
{
	int i;
	for (i= 0; _DC_state_names[i].name; i++)
		if (_DC_state_names[i].state == state)
			return(_DC_state_names[i].name);
	return("(unknown)");
}


/********************************************************************* INIT */

static GHashTable *_DC_wu_table= NULL;
uuid_t _DC_project_uuid;
char *_DC_project_uuid_str= NULL;
char *_DC_config_file= NULL;

DC_ResultCallback	_DC_result_callback= NULL;
DC_SubresultCallback	_DC_subresult_callback= NULL;
DC_MessageCallback	_DC_message_callback= NULL;



/* Initializes the DC-API. */
int
DC_initMaster(const char *configFile)
{
	int ret;
	char *cfgval= NULL;
	GString *gs;

	if (!configFile)
		configFile= DC_CONFIG_FILE;
	if (configFile[0] != '/')
	{
		gs= g_string_new(g_get_current_dir());
		g_string_append(gs, "/");
		g_string_append(gs, configFile);
	}
	else
		gs= g_string_new(configFile);
	_DC_config_file= gs->str;
	g_string_free(gs, FALSE);

	ret= _DC_parseCfg(configFile);
	if (ret)
	{
		DC_log(LOG_ERR, "DC-API config file (%s) parse error",
		       configFile);
		return(ret);
	}
	DC_log(LOG_DEBUG, "DC_initMaster(%s)", configFile);

	_DC_init_utils();
	_DC_init_common();

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
	/*
	ret= uuid_parse((char *)cfgval, _DC_project_uuid);
	if (ret)
	{
		DC_log(LOG_ERR, "Invalid project UUID");
		g_free(cfgval);
		return(DC_ERR_CONFIG);
	}
	*/
	if (_DC_project_uuid_str)
		g_free(_DC_project_uuid_str);
	_DC_project_uuid_str= g_strdup(cfgval);
	g_free(cfgval);

	/* Enforce a canonical string representation of the UUID */
	/*uuid_unparse_lower(_DC_project_uuid, _DC_project_uuid_str);*/
	uuid_parse(_DC_project_uuid_str, _DC_project_uuid);

	return(DC_OK);
}


/****************************************************** Manage WU structure */

/* Creates one work unit. */
DC_Workunit *
DC_createWU(const char *clientName,
	    const char *arguments[], int subresults, const char *tag)
{
	DC_Workunit *wu;
	char uuid_str[37];
	uuid_t wu_uuid;
	int ret;

	wu= g_new0(DC_Workunit, 1);
	_DC_wu_changed(wu);

	DC_log(LOG_DEBUG, "DC_createWU(%s, %p, %d, %s)=%p",
	       clientName, arguments, subresults, tag, wu);

	_DC_wu_set_client_name(wu, clientName);

	wu->argv= g_strdupv((char **)arguments);
	for (_DC_wu_set_argc(wu, 0);
	     arguments && arguments[wu->data.argc];
	     _DC_wu_set_argc(wu, wu->data.argc+1))
		;
	_DC_wu_set_subresults(wu, subresults);
	_DC_wu_set_tag(wu, g_strdup(tag));
	
	uuid_generate(wu_uuid);
	uuid_unparse_lower(wu_uuid, uuid_str);
	{
		char *wd= DC_getCfgStr(CFG_WORKDIR);
		GString *s= g_string_new("");
		GString *d= g_string_new("");

		g_string_printf(d, "%s/.dcapi-%s/%.2s/%s", wd,
				_DC_project_uuid_str, uuid_str, uuid_str);
		g_string_printf(s, "%s", uuid_str);

		_DC_wu_set_uuid_str(wu, s->str);
		g_string_free(s, FALSE);
		/*g_string_free(hd, TRUE);*/
		_DC_wu_set_workdir(wu, d->str);
		g_string_free(d, FALSE);
		free(wd);
	}

	GString *wu_name;
	wu_name = g_string_new("");
	if (tag)
		g_string_printf(wu_name, "%s_%s_%s",_DC_project_uuid_str,	
			wu->data.uuid_str, tag);
	else 
		g_string_printf(wu_name, "%s_%s", _DC_project_uuid_str,
			wu->data.uuid_str);
	wu->data.name = strdup(wu_name->str);

	wu->condor_events= g_array_new(FALSE, FALSE,
				       sizeof(struct _DC_condor_event));

	if (!_DC_wu_table)
		DC_initMaster(NULL);
	g_hash_table_insert(_DC_wu_table, wu_name->str, wu);

	g_string_free(wu_name, FALSE);

	ret= _DC_mkdir_with_parents(wu->data.workdir, 0700);
	if (ret)
	{
		DC_log(LOG_ERR,
		       "Failed to create WU working directory %s: %s",
		       wu->data.workdir, strerror(errno));
		DC_destroyWU(wu);
		return(NULL);
	}

	_DC_wu_make_client_config(wu);
	_DC_wu_make_client_executables(wu);
	_DC_wu_set_state(wu, DC_WU_READY);

	return(wu);
}


/* Releases internal resources allocated to a work unit. */
void
DC_destroyWU(DC_Workunit *wu)
{
	int i;
	GString *s;
	int serialized= FALSE;

	if (!_DC_wu_check(wu))
		return;
	DC_log(LOG_DEBUG, "DC_destroyWU(%p-\"%s\")", wu, wu->data.name);

	if (_DC_wu_table)
		g_hash_table_remove(_DC_wu_table, wu->data.name);

	s= g_string_new("");

	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_management_box));
	if ((i= _DC_nuof_messages(s->str, _DCAPI_SIG_SERIALIZED)) > 0)
	{
		DC_log(LOG_NOTICE, "destroying a serialized "
		       "wu: %s", wu->data.name);
		serialized= TRUE;
	}

	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_master_message_box));
	if ((i= _DC_nuof_messages(s->str, _DCAPI_MSG_MESSAGE)) > 0)
		DC_log(LOG_NOTICE, "%d master messages unhandled by "
		       "destroying wu: %s", i, wu->data.name);

	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_client_message_box));
	if ((i= _DC_nuof_messages(s->str, _DCAPI_MSG_MESSAGE)) > 0)
		DC_log(LOG_NOTICE, "%d client messages unhandled of wu: %s",
		       i, wu->data.name);

	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_subresults_box));
	if ((i= _DC_nuof_messages(s->str, _DCAPI_MSG_LOGICAL)) > 0)
		DC_log(LOG_NOTICE, "%d client subresults unhandled of wu: %s",
		       i, wu->data.name);
	g_string_free(s, TRUE);

	if (wu->data.state == DC_WU_READY ||
	    wu->data.state == DC_WU_UNKNOWN)
		DC_log(LOG_NOTICE, "Destroying an unstarted wu: %s", wu->data.name);

	if (wu->data.state == DC_WU_RUNNING ||
	    wu->data.state == DC_WU_SUSPENDED)
	{
		DC_log(LOG_NOTICE, "Destroying a started but not yet "
		       "finished wu: %s", wu->data.name);
		DC_log(LOG_INFO, "WU has been started but not finished "
		       "not removing its workdir %s", wu->data.workdir);
	}
	else if (wu->data.workdir)
	{
		const char *name;
		char *exec;
		GDir *dir;
		int ret, i;
		GString *fn;
		int leave;

		leave= strtol(_DC_wu_cfg(wu, cfg_leave_files), 0, 0);
		if (leave)
			DC_log(LOG_INFO, "WU's files not deleted");

		/* Removing generated files */
		fn= g_string_new(wu->data.workdir);
		if (!leave &&
		    !serialized)
		{
			fn= g_string_append(fn, "/");
			fn= g_string_append(fn, _DC_wu_cfg(wu, cfg_submit_file));
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					CLIENT_CONFIG_NAME);
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_executable));
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_condor_log));
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					DC_LABEL_STDOUT);
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					DC_LABEL_STDERR);
			unlink(fn->str);
			exec= _DC_wu_cfg(wu, cfg_executable);
			if (!exec)
				exec= wu->data.client_name;
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					exec);
			unlink(fn->str);
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					DC_getClientCfgStr(wu->data.client_name,
							   "LogFile",
							   TRUE));
			unlink(fn->str);
		}
		
		if (!leave &&
		    !serialized)
		{
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_client_message_box));
			i= _DC_rm(fn->str);
			if (i > 0)
				DC_log(LOG_NOTICE, "%d unhandled client messages "
				       "remained", i);
			
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_management_box));
			i= _DC_rm(fn->str);
			if (i > 0)
				DC_log(LOG_NOTICE, "%d unhandled system messages "
				       "remained", i);
			
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_master_message_box));
			i= _DC_rm(fn->str);
			if (i > 0)
				DC_log(LOG_NOTICE, "%d unhandled master messages "
				       "remained", i);
			
			g_string_printf(fn, "%s/%s", wu->data.workdir,
					_DC_wu_cfg(wu, cfg_subresults_box));
			i= _DC_rm(fn->str);
			if (i > 0)
				DC_log(LOG_NOTICE, "%d unhandled client subresults "
				       "remained", i);
		}

		g_string_free(fn, TRUE);
		
		/* The work directory should not contain any extra files, but
		 * just in case */
		if (!leave &&
		    !serialized)
		{
			dir= g_dir_open(wu->data.workdir, 0, NULL);
			while (dir &&
			       (name= g_dir_read_name(dir)))
			{
				GString *str= g_string_new(wu->data.workdir);
				g_string_append_c(str, G_DIR_SEPARATOR);
				g_string_append(str, name);
				DC_log(LOG_INFO, "Removing unknown file %s",
				       str->str);
				unlink(str->str);
				g_string_free(str, TRUE);
			}
			if (dir)
				g_dir_close(dir);

			ret= rmdir(wu->data.workdir);
			if (ret)
			{
				DC_log(LOG_WARNING, "Failed to remove WU working "
				       "directory %s: %s", wu->data.workdir,
				       strerror(errno));
			}
		}
		g_free(wu->data.workdir);
	}


	_DC_wu_set_client_name(wu, NULL);
	g_free(wu->data.uuid_str);
	g_strfreev(wu->argv);
	if (wu->data.name != NULL)
		g_free(wu->data.name);
	if (wu->data.tag != NULL)
		g_free(wu->data.tag);
	g_array_free(wu->condor_events, TRUE);
	g_free(wu);
}


/* Sets an input file for the work unit. */
int
DC_addWUInput(DC_Workunit *wu,
	      const char *logicalFileName,
	      const char *URL,
	      DC_FileMode fileMode, ...)
{
	DC_PhysicalFile *file;
	char *workpath;
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	/* Remote file aren't supported */
	if (DC_FILE_REMOTE == fileMode)
	{
		DC_log(LOG_ERR, "Unsupported file mode for input file %s",
			logicalFileName);
		return(DC_ERR_BADPARAM);
	}

	DC_log(LOG_DEBUG, "DC_addWUInput(%p-\"%s\", %s, %s, %d)",
	       wu, wu->data.name, logicalFileName, URL, fileMode);

	if (wu->data.state != DC_WU_READY)
	{
		DC_log(LOG_INFO, "Modifying started wu %s", wu->data.name);
		return(DC_ERR_BADPARAM);
	}

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
			DC_log(LOG_ERR, "Failed to copy %s to %s",
			       URL, file->path);
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
	case DC_FILE_REMOTE:
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
	DC_log(LOG_DEBUG, "DC_addWUOutput(%p-\"%s\", %s)",
	       wu, wu->data.name, logicalFileName);

	if (wu->data.state != DC_WU_READY)
	{
		DC_log(LOG_INFO, "Modifying started wu %s", wu->data.name);
		return(DC_ERR_BADPARAM);
	}

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
	DC_log(LOG_DEBUG, "DC_setWUPriority(%p-\"%s\", %d)",
	       wu, wu->data.name, priority);

	if (wu->data.state != DC_WU_READY)
	{
		DC_log(LOG_INFO, "Modifying started wu %s", wu->data.name);
		return(DC_ERR_BADPARAM);
	}

	return(DC_OK);
}


/* Sets the callback functions that will be called when a particular event. */
void
DC_setMasterCb(DC_ResultCallback resultcb,
	       DC_SubresultCallback subresultcb,
	       DC_MessageCallback msgcb)
{
	DC_log(LOG_DEBUG, "DC_setMasterCb(%p, %p, %p)",
	       resultcb, subresultcb, msgcb);

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
	return(wu->data.state);
}


/* Queries the low-level ID of the work unit. */
char *
DC_getWUId(const DC_Workunit *wu)
{
	GString *s;
	struct _DC_condor_event *ce;

	if (!_DC_wu_check(wu))
		return(NULL);
	_DC_wu_update_condor_events((DC_Workunit *)wu);
	if (wu->condor_events->len == 0)
		return(NULL);
	ce= &g_array_index(wu->condor_events,
			   struct _DC_condor_event,
			   wu->condor_events->len-1);
	s= g_string_new("");
	g_string_printf(s, "%d.%d", ce->cluster, ce->proc);
	return(g_string_free(s, FALSE));
}


/* Queries the tag of a work unit. */
char *
DC_getWUTag(const DC_Workunit *wu)
{
	if (!_DC_wu_check(wu))
		return(NULL);
	return((wu->data.tag)?strdup(wu->data.tag):NULL);
}


#define _DC_SERIALIZED_SEPARATOR '|'

static void
_DC_sser(GString *ser,
	 _DC_serialized_token tok,
	 char *str)
{
	char *s;
	if (!ser)
		return;
	g_string_append_printf(ser, "%c%c%s",
			       _DC_SERIALIZED_SEPARATOR,
			       'a'+tok,
			       s= _DC_quote_string(str));
	s?free(s):0;
}

static void
_DC_iser(GString *ser,
	 _DC_serialized_token tok,
	 int i)
{
	if (!ser)
		return;
	g_string_append_printf(ser, "%c%c%d",
			       _DC_SERIALIZED_SEPARATOR,
			       'a'+tok,
			       i);
}


/* Get the full path of a file in the WU's working directory */
static char *
_DC_workDirPath(const DC_Workunit *wu, const char *label, WorkdirFile type)
{
	static const char *const pfx[] = { "in_", "out_", "checkpoint", "dc_" };

	if (type == FILE_CKPT)
		return g_strdup_printf("%s%ccheckpoint", wu->data.workdir,
			G_DIR_SEPARATOR);

	return g_strdup_printf("%s%c%s%s", wu->data.workdir, G_DIR_SEPARATOR,
		pfx[type], label);
}


/* Open a file in the WU's working directory */
static FILE *
open_workdir_file(const DC_Workunit *wu, const char *label,
	WorkdirFile type, const char *mode)
{
	char *name;
	FILE *f;

	name = _DC_workDirPath(wu, label, type);
	f = fopen(name, mode);
	if (!f)
		DC_log(LOG_ERR, "Failed to open %s (mode %s): %s",
			name, mode, strerror(errno));
	g_free(name);
	return f;
}

/* Serializes a work unit description. */
static char *
_DC_serializeWU(DC_Workunit *wu)
{
	GString *dn, *ser;
	char *s;
	int i;
	unsigned int u;

	if (!_DC_wu_check(wu)) {
		DC_log(LOG_ERR, "_DC_wu_check() failed for work unit");
		return(NULL);
	}

	dn= g_string_new("");
	g_string_printf(dn, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_management_box));
	if (_DC_nuof_messages(dn->str, _DCAPI_SIG_SERIALIZED) > 0)
	{
		/* serialized already */
		s= _DC_read_message(dn->str, _DCAPI_SIG_SERIALIZED, FALSE);
		g_string_free(dn, TRUE);
		return(s);
	}
	else
	{
		/* new serialization */
		int r;
		ser= g_string_new("");
		s= _DC_quote_string(wu->data.name);
		g_string_append_printf(ser, "%c%s", 'a'+st_name, s);
		s?free(s):NULL;

		_DC_sser(ser, st_workdir, wu->data.workdir);

		_DC_sser(ser, st_tag, wu->data.tag);

		_DC_sser(ser, st_client_name, wu->data.client_name);

		_DC_sser(ser, st_uuid_str, wu->data.uuid_str);

		_DC_iser(ser, st_argc, wu->data.argc);
		for (i= 0; i<wu->data.argc; i++)
		{
			_DC_sser(ser, st_argv,
				 (wu->argv[i])?(wu->argv[i]):NULL);
		}

		_DC_iser(ser, st_state, wu->data.state);
		_DC_sser(ser, st_state_name, _DC_state_name(wu->data.state));

		for (u= r= 0; u < wu->condor_events->len; u++)
		{
			struct _DC_condor_event *ce;
			ce= &g_array_index(wu->condor_events,
					   struct _DC_condor_event,
					   u);
			if (ce->reported)
				r++;
		}
		_DC_iser(ser, st_nuof_reported, r);
		for (u= 0; u < wu->condor_events->len; u++)
		{
			struct _DC_condor_event *ce;
			ce= &g_array_index(wu->condor_events,
					   struct _DC_condor_event,
					   u);
			if (ce->reported)
				_DC_iser(ser, st_reported, u);
		}

		s= ser->str;
		g_string_free(ser, FALSE);

		_DC_create_message(dn->str, _DCAPI_SIG_SERIALIZED,
				   s, NULL);
		g_string_free(dn, TRUE);
		return(s);
	}
	return(0);
}

char *
DC_serializeWU(DC_Workunit *wu)
{
	FILE *f = open_workdir_file(wu, "serialized.txt", FILE_DCAPI, "w");
	if (!f)
		return NULL;		
	char *serialized_data = _DC_serializeWU(wu);
	fprintf(f, "%s", serialized_data);
	fclose(f);
	return strdup(wu->data.name);
}


static int
_DC_iunser(char *s)
{
	int i;
	if (!s)
		return(0);
	i= strtol(s, 0, 0);
	free(s);
	return(i);
}

/* Restores a serialized work unit. */
static DC_Workunit *
_DC_deserializeWU(const char *buf)
{
	char *tok, *s, *b;
	char sep[2]= "\0\0";
	DC_Workunit *wu;
	int argv_i= 0;
	GString *dn;

	wu= g_new0(DC_Workunit ,1);
	_DC_wu_changed(wu);
	wu->condor_events= g_array_new(FALSE, FALSE,
				       sizeof(struct _DC_condor_event));

	b= strdup(buf);
	sep[0]= _DC_SERIALIZED_SEPARATOR;
	tok= strtok(b, sep);
	while (tok)
	{
		_DC_serialized_token st= (_DC_serialized_token)(tok[0]-'a');
		s= _DC_unquote_string(&tok[1]);
		switch (st)
		{
		case st_name:
		{
			_DC_wu_set_name(wu, s);
			break;
		}
		case st_workdir:
		{
			_DC_wu_set_workdir(wu, s);
			_DC_wu_update_condor_events(wu);
			break;
		}
		case st_tag:
		{
			_DC_wu_set_tag(wu, s);
			break;
		}
		case st_client_name:
		{
			_DC_wu_set_client_name(wu, s);
			if (!_DC_wu_table)
				DC_initMaster(NULL);
			g_hash_table_insert(_DC_wu_table, wu->data.name, wu);
			break;
		}
		case st_uuid_str:
		{
			_DC_wu_set_uuid_str(wu, s);
			break;
		}
		case st_argc:
		{
			_DC_wu_set_argc(wu, _DC_iunser(s));
			wu->argv= g_new0(char*, wu->data.argc+1);
			argv_i= 0;
			break;
		}
		case st_argv:
		{
			if (argv_i < wu->data.argc)
			{
				wu->argv[argv_i]= s;
				argv_i++;
			}
			break;
		}
		case st_state:
		{
			DC_log(LOG_DEBUG, "_DC_iunser");
			_DC_wu_set_state(wu, (DC_WUState)_DC_iunser(s));
			break;
		}
		case st_state_name:
		{
			free(s);
			break;
		}
		case st_nuof_reported:
		{
			free(s);
			break;
		}
		case st_reported:
		{
			unsigned int u;
			u= _DC_iunser(s);
			if (u < wu->condor_events->len)
			{
				struct _DC_condor_event *ce;
				ce= &g_array_index(wu->condor_events,
						   struct _DC_condor_event,
						   u);
				ce->reported= TRUE;
			}
		}
		}
		tok= strtok(NULL, sep);
	}
	free(b);

	if (!_DC_file_exists(wu->data.workdir))
	{
		DC_log(LOG_ERR, "Deserialized wu: no workdir (%s)",
		       wu->data.workdir);
		//DC_destroyWU(wu);
		//return(NULL);
	}
	dn= g_string_new("");
	g_string_printf(dn, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_management_box));
	_DC_read_message(dn->str, _DCAPI_SIG_SERIALIZED, TRUE);
	g_string_free(dn, TRUE);
	return(wu);
}

/********************************************************************
 * Helper functions
 */

/* Calculate & create the working directory. The working directory has the
 * form: <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid> Where <hash>
 * is the first 2 hex digits of the uuid
 */
static char *get_workdir(const uuid_t uuid, int create)
{
	char *tmp, uuid_str[37], *cfgval;
	GString *str;
	int ret;

	uuid_unparse_lower(uuid, uuid_str);

	cfgval = DC_getCfgStr(CFG_WORKDIR);
	if (!cfgval)
		return NULL;
	str = g_string_new(cfgval);
	free(cfgval);

	if (create)
	{
		ret = mkdir(str->str, 0755);
		if (ret && errno != EEXIST)
			goto error;
	}

	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, ".dcapi-");
	g_string_append(str, _DC_project_uuid_str);
	if (create)
	{
		ret = mkdir(str->str, 0755);
		if (ret && errno != EEXIST)
			goto error;
	}

	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append_printf(str, "%02x", uuid[0]);
	if (create)
	{
		ret = mkdir(str->str, 0755);
		if (ret && errno != EEXIST)
			goto error;
	}

	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, uuid_str);
	if (create)
	{
		ret = mkdir(str->str, 0755);
		if (ret && errno != EEXIST)
			goto error;
	}

	tmp = str->str;
	g_string_free(str, FALSE);
	return tmp;

error:
	DC_log(LOG_ERR, "Failed to create WU working directory %s: %s",
		str->str, strerror(errno));
	g_string_free(str, TRUE);
	return NULL;
}


DC_Workunit *
DC_deserializeWU(const char *buf)
{
	DC_Workunit *wu;
	char *uuid_str;
	uuid_t uuid;
	int ret;
	char *workdir;
	char *name = buf;
	GString *wu_serialized_data;
	char buffer[256];

	/* Check if the WU belongs to this application */
	uuid_str = g_strndup(name, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name '%s' contains illegal UUID", name);
		return NULL;
	}

	if (uuid_compare(uuid, _DC_project_uuid))
	{
		DC_log(LOG_WARNING, "WU '%s' does not belong to this "
			"application", name);
		return NULL;
	}

	/* WU name syntax: <uuid> '_' <uuid> [ '_' <tag> ] */
	if (name[36] != '_' || (strlen(name) > 73 && name[73] != '_'))
	{
		DC_log(LOG_ERR, "Illegal WU name syntax in '%s'", name);
		return NULL;
	}

	/* Check the WU's UUID */
	uuid_str = g_strndup(name + 37, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name '%s' contains illegal UUID", name);
		return NULL;
	}

	workdir = get_workdir(uuid, 0);
	wu = g_new0(DC_Workunit, 1);
	wu->data.workdir = workdir;
		
	FILE *f = open_workdir_file(wu, "serialized.txt", FILE_DCAPI, "r");
	g_free(wu);
	if (!f)
	{
		DC_log(LOG_ERR, "Could not read serialized work unit from '%s'", workdir);
		return NULL;
	}
	wu_serialized_data = g_string_new("");
	while (fgets(buffer, sizeof(buffer), f)) {
		g_string_append(wu_serialized_data, buffer);
	}
	fclose(f);
	
	wu = _DC_deserializeWU(wu_serialized_data->str);
	return wu;
}

/* iterator for DC_getWUNumber() */
static DC_WUState _DC_dd_look_for_state;
static void _DC_dd_check_state(void *key, void *value, void *ptr)
{
	DC_Workunit *wu=(DC_Workunit *)value;
	int *count= (int *)ptr;
	if (wu->data.state == _DC_dd_look_for_state)
		++(*count);
}

/* Queries the number of WUs known to the API in the given state. */
int
DC_getWUNumber(DC_WUState state)
{
	int val= 0;

	_DC_dd_look_for_state= state;
	g_hash_table_foreach(_DC_wu_table, (GHFunc)_DC_dd_check_state, &val);
	return(val);
}


/************************************************************** Main cycles */

static void
_DC_process_event(DC_MasterEvent *event)
{
	if (!event)
		return;

	DC_log(LOG_DEBUG, "Processing an event %d, calling callback...",
	       event->type);
	switch (event->type)
	{
	case DC_MASTER_RESULT:
	{
		if (_DC_result_callback)
			(*_DC_result_callback)(event->wu,
					       event->result);
		break;
	}
	case DC_MASTER_SUBRESULT:
	{
		if (_DC_subresult_callback)
			(*_DC_subresult_callback)(event->wu,
						  event->subresult->label,
						  event->subresult->path);
		break;
	}
	case DC_MASTER_MESSAGE:
	{
		if (_DC_message_callback)
			(*_DC_message_callback)(event->wu,
						event->message);
		break;
	}
	}
	DC_destroyMasterEvent(event);
}


/* Waits for events and processes them. */
int
DC_processMasterEvents(int timeout)
{
	/* call callback and destroy event */
	time_t start, now;
	DC_MasterEvent *event;

	DC_log(LOG_DEBUG, "DC_processMasterEvents(%d)",
	       timeout);

	if ((event= DC_waitMasterEvent(NULL, 0)) != NULL)
		_DC_process_event(event);
	if (timeout==0)
		return(DC_OK);

	start= time(NULL);
	now= time(NULL);
	while (now-start <= timeout &&
	       g_hash_table_size(_DC_wu_table) > 0)
	{
		sleep(1);
		if ((event= DC_waitMasterEvent(NULL, 0)) != NULL)
			_DC_process_event(event);
		now= time(NULL);
	}
	return(DC_OK);
}


static DC_MasterEvent *_DC_filtered_event;
static int _DC_counter;

static gboolean
_DC_check_filtered_wu_event(gpointer wu_name, gpointer w, gpointer ptr)
{
	DC_Workunit *wu= (DC_Workunit *)w;
	char *filter= (char *)ptr;
	char *tag;
	int match= TRUE;
	gboolean found;

	_DC_counter++;
	tag= DC_getWUTag(wu);
	if (filter)
		if (strcmp(tag, filter) != 0)
			tag= FALSE;
	tag?free(tag):0;
	if (!match)
	{
		return(FALSE);
	}
	if ((_DC_filtered_event= DC_waitWUEvent(wu, 0)) != NULL)
		DC_log(LOG_DEBUG,
		       "DC_waitWUEvent found an event for wu %p-\"%s\"",
		       wu, wu->data.name);
	found= _DC_filtered_event != NULL;
	return(found);
}

/* Checks for events and return them. */
DC_MasterEvent *
DC_waitMasterEvent(const char *wuFilter, int timeout)
{
	/* no callback called! */
	DC_Workunit *wu;
	time_t start, now;

	if (timeout)
		DC_log(LOG_DEBUG, "DC_waitMasterEvent(%s, %d)",
		       wuFilter, timeout);

	start= time(NULL);
	now= time(NULL);
	do
	{
		_DC_filtered_event= NULL;
		_DC_counter= 0;
		wu= (DC_Workunit *)
			g_hash_table_find(_DC_wu_table,
					  _DC_check_filtered_wu_event,
					  (gpointer)wuFilter);
		
		if (_DC_filtered_event)
			return(_DC_filtered_event);
		if (timeout)
			sleep(1);
	}
	while (timeout != 0 &&
	       now-start <= timeout);
	return(NULL);
}


/* Checks for events for a particular WU. */
DC_MasterEvent *
DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	/* no callback called! */
	time_t start, now;
	DC_MasterEvent *me= NULL;

	if (!_DC_wu_check(wu))
		return(NULL);
	if (timeout)
		DC_log(LOG_DEBUG, "DC_waitWUEvent(%p-\"%s\", %d)",
		       wu, wu->data.name, timeout);

	start= time(NULL);
	now= start;
	do
	{
		_DC_wu_update_condor_events(wu);
		if ((me= _DC_wu_check_client_messages(wu)) != NULL)
			DC_log(LOG_DEBUG, "DC_waitWUEvent found a message for "
			       "wu %p-\"%s\"", wu, wu->data.name);
		if (!me)
		{
			if ((me= _DC_wu_condor2api_event(wu)) != NULL)
				DC_log(LOG_DEBUG,
				       "DC_waitWUEvent found a "
				       "condor event for wu %p-\"%s\"",
				       wu, wu->data.name);
		}
		if (me)
		{
			return(me);
		}
		now= time(NULL);
		if (timeout)
			sleep(1);
	}
	while (timeout !=0 &&
	       now-start <= timeout);
	return(NULL);
}


/* Destroys an event. */
void
DC_destroyMasterEvent(DC_MasterEvent *event)
{
	DC_log(LOG_DEBUG, "DC_destroyMasterEvent(%p)", event);
	_DC_event_destroy(event);
}


/**************************************************************** Messaging */

/* Sends a message to a running work unit. */
int
DC_sendWUMessage(DC_Workunit *wu, const char *message)
{
	GString *dn;
	int ret;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	DC_log(LOG_DEBUG, "DC_sendWUMessage(%p-\"%s\", %s)",
	       wu, wu->data.name, message);
	dn= g_string_new(wu->data.workdir);
	g_string_append(dn, "/");
	g_string_append(dn, _DC_wu_cfg(wu, cfg_master_message_box));
	ret= _DC_create_message(dn->str, _DCAPI_MSG_MESSAGE, message, NULL);
	g_string_free(dn, TRUE);
	return(ret);
}


/********************************************************** Result handling */

/* Queries what optional fields are present in the result. */
unsigned
DC_getResultCapabilities(const DC_Result *result)
{
	int cap;
	char *fn;
	
	/* Fix #1106 */
	cap= DC_GC_EXITCODE | DC_GC_LOG;
	/*cap|= DC_GC_STDOUT | DC_GC_STDERR;*/
	fn= DC_getResultOutput(result, DC_LABEL_STDERR);
	if (_DC_file_exists(fn) &&
	    !_DC_file_empty(fn))
		cap|= DC_GC_STDERR;
	free(fn);
	fn= DC_getResultOutput(result, DC_LABEL_STDOUT);
	if (_DC_file_exists(fn))
		cap|= DC_GC_STDOUT;
	free(fn);

	return(cap);
}


/* Returns the WU that generated this result. */
DC_Workunit *
DC_getResultWU(DC_Result *result)
{
	if (!result)
		return(NULL);
	return(result->wu);
}


/* Returns the exit code of the client application. */
int
DC_getResultExit(const DC_Result *result)
{
	int i;

	if (!result)
		return(0);
	if (!_DC_wu_check(result->wu))
		return(0);
	if (_DC_wu_exit_code(result->wu, &i) != DC_OK)
		return(0);
	return(i);
}


/* Returns the local name of an output file. */
char *
DC_getResultOutput(const DC_Result *result, const char *logicalFileName)
{
	GString *fn;

	if (!result)
		return(NULL);
	if (!_DC_wu_check(result->wu))
		return(NULL);
	fn= g_string_new(result->wu->data.workdir);
	fn= g_string_append(fn, "/");
	if (strcmp(logicalFileName, DC_LABEL_CLIENTLOG) == 0)
	{
		/* Fix #1107 */
		char *s= DC_getClientCfgStr(result->wu->data.client_name,
					    "LogFile",
					    FALSE);
		if (s)
		{
			fn= g_string_append(fn, s);
			free(s);
		}
		else
			fn= g_string_append(fn, DC_LABEL_CLIENTLOG);
	}
	else
		fn= g_string_append(fn, logicalFileName);
	return(g_string_free(fn, FALSE));
}


/* Application level configuration parameter */
char *
_DC_acfg(enum _DC_e_param what)
{
	char *v;

	v= DC_getCfgStr(_DC_params[what].name);
	if (v &&
	    *v)
		return(v);
	return(_DC_params[what].def);
}


/* End of condor/condor_master.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
