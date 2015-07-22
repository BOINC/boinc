/*
 * condor/condor_wu.c
 *
 * DC-API functions to handle DC_Workunit data type
 *
 * (c) Daniel Drotos, 2006
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

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
	if (!_DC_wu_check(wu)) {
		DC_log(LOG_ERR, "Cannot change wu name! ('%s')", new_name);
		return(DC_ERR_UNKNOWN_WU);
	}
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
	if (!_DC_wu_check(wu))
		return(NULL);
	if (what >= cfg_nuof)
		return(NULL);
	if (!_DC_params[what].name)
		return(NULL);

	if (_DC_params[what].gvalue)
		return(_DC_params[what].gvalue);

	_DC_params[what].gvalue=
		DC_getClientCfgStr(wu->data.client_name,
				   _DC_params[what].name,
				   /*TRUE*/1);
	if (_DC_params[what].gvalue)
		return(_DC_params[what].gvalue);
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


static char *_DC_condor_submit_template=
"# Built-in template\n"
"# Date: %d\n"
"# WU name: %n\n"
"# WU id: %i\n"
"# WU workdir: %w\n"
"# Client name: %c\n"
"# Number of args: %r\n"
"# List of input files: \"%I\"\n"
"# List of output files: \"%O\"\n"
"\n"
"Executable = %x\n"
"arguments = %a\n"
"Universe = %u\n"
"output = %o\n"
"error = %e\n"
"log = %l\n"
"Queue\n"
;

static int
_DC_wu_process_template(DC_Workunit *wu, char *tmpl, FILE *f)
{
	char *cfgarch;
	char *cfgexec, *cfglog;
	char *c= tmpl;
    GString *filelist;

	cfgarch= _DC_acfg(cfg_architectures);
	cfgexec= _DC_wu_cfg(wu, cfg_executable);
	if (!cfgexec)
		cfgexec= wu->data.client_name;
	cfglog= _DC_wu_cfg(wu, cfg_condor_log);

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	if (!tmpl ||
	    !f)
		return(DC_ERR_INTERNAL);

	while (*c)
	{
		if (*c == '%')
		{
			c++;
			if (*c == '\0')
				break;
			switch (*c)
			{
			case 'n':
			{
				fprintf(f, "%s", wu->data.name);
				break;
			}
			case 'r':
			{
				fprintf(f, "%d", wu->data.argc);
				break;
			}
			case 'i':
			{
				fprintf(f, "%s", wu->data.uuid_str);
				break;
			}
			case 'w':
			{
				fprintf(f, "%s", wu->data.workdir);
				break;
			}
			case 'c':
			{
				fprintf(f, "%s", wu->data.client_name);
				break;
			}
			case 'd':
			{
				time_t t;
				char *ft;
				t= time(NULL);
				ft= ctime(&t);
				ft[strlen(ft)-1]= '\0';
				fprintf(f, "%s", ft);
				break;
			}
			case 'x':
			{
				fprintf(f, "%s", cfgexec);
				break;
			}
			case 'a':
			{
				int i;
				for (i= 0; wu->data.argc > 0 &&
					     wu->argv[i]; i++)
				{
					char *p= wu->argv[i];
					if (!(*(wu->argv[i])))
						fprintf(f, " ''");
					else
					{
						while (*p && !isspace(*p))
							p++;
						if (*p)
							fprintf(f, " '%s'", wu->argv[i]);
						else
							fprintf(f, " %s", wu->argv[i]);
					}
				}
				break;
			}
			case 'u':
			{
				fprintf(f, "vanilla");
				break;
			}
			case 'o':
			{
				fprintf(f, "%s", DC_LABEL_STDOUT);
				break;
			}
			case 'e':
			{
				fprintf(f, "%s", DC_LABEL_STDERR);
				break;
			}
			case 'l':
			{
				fprintf(f, "%s", cfglog);
				break;
			}
            /* capital 'i' */
			case 'I':
			{
                filelist = g_string_new(NULL);
                g_list_foreach(wu->input_files, _DC_wu_input_list_foreach, filelist);
                /* remove last comma from the string */
                filelist = g_string_erase(filelist, filelist->len-1, -1);
                fprintf(f, "%s", filelist->str);    
                g_string_free(filelist, TRUE);
                break;
			}
			case 'O':
			{
                filelist = g_string_new(NULL);
                g_list_foreach(wu->output_files, _DC_wu_output_list_foreach, filelist);
                /* remove last comma from the string */
                filelist = g_string_erase(filelist, filelist->len-1, -1);
                fprintf(f, "%s", filelist->str);    
                g_string_free(filelist, TRUE);
                break;
			}
			default:
			{
				fprintf(f, "%c", *c);
				break;
			}
			}
		}
		else
		{
			fprintf(f, "%c", *c);
		}
		c++;
	}
	return(DC_OK);
}

int
_DC_wu_gen_condor_submit(DC_Workunit *wu)
{
	FILE *f= NULL;
	GString *fn;
	char *cfgtmpl;
	char *tmpl= 0;
	int ret;

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

	cfgtmpl= _DC_wu_cfg(wu, cfg_condor_submit_template);
	if (!cfgtmpl)
		tmpl= _DC_condor_submit_template;
	else
	{
		tmpl= _DC_get_file(cfgtmpl);
	}
	ret= DC_OK;
	if (tmpl)
	{
		ret= _DC_wu_process_template(wu, tmpl, f);
	}
	else
	{
	}

	if (f)
		fclose(f);
	return(ret);
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
	char *s;

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
		/* Fix #1107 */
		s= DC_getClientCfgStr(wu->data.client_name,
				      "LogFile",
				      FALSE);
		if (s)
		{
			fprintf(f, "LogFile = %s\n", s);
			free(s);
		}
		else
		{
			fprintf(f, "LogFile = %s\n", DC_LABEL_CLIENTLOG);
		}
		
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


/* User function for g_list_foreach() in _DC_wu_process_template(): "%I"
 *
 * @data DC_PhysicalFile*
 * @user_data GString
 */
void _DC_wu_input_list_foreach(gpointer data, gpointer user_data)
{
    if (data != NULL && ((DC_PhysicalFile*)data)->path != NULL) 
    {
        user_data = g_string_append((GString *)user_data, ((DC_PhysicalFile*)data)->path);
        /* we want a comma separated list */
        user_data = g_string_append((GString *)user_data, ",");
    }
}


/* User function for g_list_foreach() in _DC_wu_process_template(): "%O"
 *
 * @data char* data to be appended 
 * @user_data GString the string to append to
 */
void _DC_wu_output_list_foreach(gpointer data, gpointer user_data)
{
    if (data != NULL) 
    {
        user_data = g_string_append((GString *)user_data, (char*)data);    
        /* we want a comma separated list */
        user_data = g_string_append((GString *)user_data, ",");
    }
}


/* End of condor/condor_wu.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
