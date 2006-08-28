/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "dc.h"
#include "dc_common.h"

#include "condor_common.h"
#include "condor_master.h"
#include "condor_wu.h"
#include "condor_event.h"
#include "condor_utils.h"


/*****************************************************************************/

void
_DC_wu_changed(DC_Workunit *wu)
{
	unsigned char *p;
	int i, sum= 0;

	if (!wu)
		return;
	p= (unsigned char*)(&(wu->data));
	for (i= 0; i < (signed)sizeof(struct _DC_wu_data); i++)
		sum+= p[i];
	wu->magic= sum;
	wu->chk= 0-sum;
}

int
_DC_wu_check(const DC_Workunit *wu)
{
	if (!wu)
		return(FALSE);
	if ((wu->magic + wu->chk) != 0)
		return(FALSE);
	return(TRUE);
}


int
_DC_wu_set_client_name(DC_Workunit *wu,
		       const char *new_name)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	g_free(wu->data.client_name);
	wu->data.client_name= g_strdup(new_name);
	_DC_wu_changed(wu);
	return(DC_OK);
}

int
_DC_wu_set_argc(DC_Workunit *wu,
		int new_argc)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.argc= new_argc;
	_DC_wu_changed(wu);
	return(DC_OK);
}


int
_DC_wu_set_uuid_str(DC_Workunit *wu,
		    char *new_uuid_str)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.uuid_str= new_uuid_str;
	_DC_wu_changed(wu);
	return(DC_OK);
}

int
_DC_wu_set_name(DC_Workunit *wu,
		char *new_name)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.name= new_name;
	_DC_wu_changed(wu);
	return(DC_OK);
}

int
_DC_wu_set_tag(DC_Workunit *wu,
	       char *new_tag)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.tag= new_tag;
	_DC_wu_changed(wu);
	return(DC_OK);
}

int
_DC_wu_set_subresults(DC_Workunit *wu,
		      int new_subresults)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.subresults= new_subresults;
	_DC_wu_changed(wu);
	return(DC_OK);
}

int
_DC_wu_set_workdir(DC_Workunit *wu,
		   char *new_workdir)
{
	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);
	wu->data.workdir= new_workdir;
	_DC_wu_changed(wu);
	return(DC_OK);
}


DC_WUState
_DC_wu_set_state(DC_Workunit *wu,
		 DC_WUState new_state)
{
	/*DC_WUState act= wu->state;*/
	if (!_DC_wu_check(wu))
		return(DC_WU_UNKNOWN);
	DC_log(LOG_DEBUG, "Set state to %s of %p-\"%s\"", _DC_state_name(new_state), wu, wu->data.name);
	wu->data.state= new_state;
	_DC_wu_changed(wu);
	return(wu->data.state);
}


/* Get a configuration parameter */
char *
_DC_wu_cfg(DC_Workunit *wu,
	   enum _DC_e_param what)
{
	char *v;

	if (!_DC_wu_check(wu))
		return(NULL);
	v= DC_getClientCfgStr(wu->data.client_name,
			      _DC_params[what].name,
			      /*TRUE*/1);
	if (v &&
	    *v)
		return(v);
	return(_DC_params[what].def);
}


/* Check if the logical name is not already registered */
int
_DC_wu_check_logical_name(DC_Workunit *wu,
			  const char *logicalFileName)
{
	GList *l;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	if (strchr(logicalFileName, '/') || strchr(logicalFileName, '\\'))
	{
		DC_log(LOG_ERR, "Illegal characters in logical file name %s",
		       logicalFileName);
		return(DC_ERR_BADPARAM);
	}
	for (l= wu->input_files; l; l= l->next)
	{
		DC_PhysicalFile *file= (DC_PhysicalFile *)l->data;
      
		if (!strcmp(file->label, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
			       "input file", logicalFileName);
			return(DC_ERR_BADPARAM);
		}
	}
	for (l= wu->output_files; l; l= l->next)
	{
		if (!strcmp((char *)l->data, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
			       "output file", logicalFileName);
			return(DC_ERR_BADPARAM);
		}
	}
	DC_log(LOG_DEBUG, "Logical filename %s checked OK",
	       logicalFileName);
	return(0);
}


char *
_DC_wu_get_workdir_path(DC_Workunit *wu,
			const char *label,
			WorkdirFile type)
{
	if (!_DC_wu_check(wu))
		return(NULL);
	return g_strdup_printf("%s%c%s", wu->data.workdir, G_DIR_SEPARATOR, label);
}


int
_DC_wu_gen_condor_submit(DC_Workunit *wu)
{
	FILE *f= NULL;
	GString *fn;
	char *cfgarch;
	char *cfgexec;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	fn= g_string_new(wu->data.workdir);
	fn= g_string_append(fn, "/");
	fn= g_string_append(fn, _DC_wu_cfg(wu, cfg_submit_file));
	if ((f= fopen(fn->str, "w+")) == NULL)
	{
		DC_log(LOG_ERR, "Condor submit file %s creation failed",
		       fn->str);
		g_string_free(fn, TRUE);
		return(DC_ERR_SYSTEM);
	}
	g_string_free(fn, TRUE);

	cfgarch= _DC_acfg(cfg_architectures);
	cfgexec= _DC_wu_cfg(wu, cfg_executable);
	if (!cfgexec)
		cfgexec= wu->data.client_name;
	fprintf(f, "Executable = %s\n", cfgexec);
	if (wu->data.argc > 0)
	{
		int i;
		fprintf(f, "arguments =");
		for (i= 0; wu->argv[i]; i++)
		{
			char *p= wu->argv[i];
			if (!(*(wu->argv[i])))
				fprintf(f, " ''");
			else
			{
				while (*p && !isspace(*p))
				p++;
				if (*p)
				{
					fprintf(f, " '%s'", wu->argv[i]);
				}
				else
				{
					fprintf(f, " %s", wu->argv[i]);
				}
			}
		}
		fprintf(f, "\n");
	}
	fprintf(f, "Universe = vanilla\n");
	fprintf(f, "output = %s\n", DC_LABEL_STDOUT);
	fprintf(f, "error = %s\n", DC_LABEL_STDERR);
	fprintf(f, "log = %s\n", _DC_wu_cfg(wu, cfg_condor_log));
	/* Dosn't work with NFS
	   fprintf(f, "should_transfer_files = YES\n");
	   fprintf(f, "when_to_transfer_output = ON_EXIT_OR_EVICT\n");
	*/
	/* According to debug, it has no effect
	   fprintf(f, "ENABLE_USERLOG_LOCKING = FALSE\n");
	   fprintf(f, "IGNORE_NFS_LOCK_ERRORS = TRUE\n");
	*/
	fprintf(f, "\n");
	fprintf(f, "Queue\n");

	if (f)
		fclose(f);
	return(DC_OK);
}


int
_DC_wu_make_client_executables(DC_Workunit *wu)
{
	char *archs;
	char *exec;
	int ret;
	GString *src, *dst;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	archs= _DC_acfg(cfg_architectures);
	
	exec= _DC_wu_cfg(wu, cfg_executable);
	if (!exec)
		exec= wu->data.client_name;
	src= g_string_new(exec);
	dst= g_string_new(wu->data.workdir);
	g_string_append(dst, "/");
	g_string_append(dst, exec);
	DC_log(LOG_DEBUG, "Copying client executable %s to %s",
	       src->str, dst->str);
	ret= _DC_copyFile(src->str, dst->str);
	g_string_free(src, TRUE);
	g_string_free(dst, TRUE);

	/*
	dst= g_string_new(wu->data.workdir);
	g_string_append(dst, "/");
	g_string_append(dst, CLIENT_CONFIG_NAME);
	DC_log(LOG_DEBUG, "Copying config %s to %s",
	       _DC_config_file, dst->str);
	i= _DC_copyFile(_DC_config_file, dst->str);
	if (i)
		ret= i;
	g_string_free(dst, TRUE);
	*/

	if (!ret)
		return(ret);

	return(DC_OK);
}


int
_DC_wu_make_client_config(DC_Workunit *wu)
{
	GString *fn;
	FILE *f;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	fn= g_string_new(wu->data.workdir);
	g_string_append(fn, "/");
	g_string_append(fn, CLIENT_CONFIG_NAME);
	if ((f= fopen(fn->str, "w")) != NULL)
	{
		if (_DC_initClientConfig(wu->data.client_name, f))
		{
			fclose(f);
			return(DC_ERR_BADPARAM);
		}
		fprintf(f, "LogFile = %s\n",
			DC_getClientCfgStr(wu->data.client_name,
					   "LogFile",
					   TRUE));
		
		fclose(f);
	}

	return(DC_OK);
}


/*static int _DC_counter= 0;*/

DC_MasterEvent *
_DC_wu_check_client_messages(DC_Workunit *wu)
{
	GString *s;
	char *message;
	DC_MasterEvent *e= NULL;

	if (!_DC_wu_check(wu))
		return(NULL);

	s= g_string_new(wu->data.workdir);
	g_string_append_printf(s, "/%s",
			       _DC_wu_cfg(wu, cfg_client_message_box));
	/*DC_log(LOG_DEBUG, "Checking box %s for message\n", s->str);*/
	if ((message= _DC_read_message(s->str, _DCAPI_MSG_MESSAGE, TRUE)))
	{
		e= _DC_event_create(wu, NULL, NULL, message);
		DC_log(LOG_DEBUG, "Message event created: %p "
		       "for wu (%p-\"%s\")", e, wu, wu->data.name);
		DC_log(LOG_DEBUG, "Message of the event: %s", e->message);
	}

	/*
	g_string_printf(s, "%s/%s", wu->data.workdir,
			_DC_wu_cfg(wu, cfg_management_box));
	if ((message= _DC_read_message(s->str, _DCAPI_MSG_ACK, TRUE)))
	{
		DC_log(LOG_DEBUG, "Acknowledge arrived: %s", message);
		if (strcmp(message, _DCAPI_ACK_SUSPEND) == 0)
		{
			wu->asked_to_suspend= TRUE;
		}
	}
	*/

	if (e == NULL/*)
		_DC_counter++;
		  if (_DC_counter % 5 == 0*/)
	{
		g_string_printf(s, "%s/%s", wu->data.workdir,
				_DC_wu_cfg(wu, cfg_subresults_box));
		/*DC_log(LOG_DEBUG, "Checking box %s for logical_name\n",
		  s->str);*/
		if ((message= _DC_read_message(s->str, _DCAPI_MSG_LOGICAL,
					       TRUE)))
		{
			DC_PhysicalFile *f;
			g_string_append_printf(s, "/real_files/%s", message);
			DC_log(LOG_DEBUG, "Got subresult %s %s",
			       message, s->str);
			f= _DC_createPhysicalFile(message, s->str);
			e= _DC_event_create(wu, NULL, f, NULL);
		}
	}

	g_string_free(s, TRUE);
	return(e);
}


/* End of condor/condor_wu.c */
