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
	return g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, label);
}


int
_DC_wu_gen_condor_submit(DC_Workunit *wu)
{
	FILE *f= NULL;
	GString *fn;
	char *cfgval;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	fn= g_string_new(wu->workdir);
	fn= g_string_append(fn, "/condor_submit.txt");
	if ((f= fopen(fn->str, "w+")) == NULL)
	{
		DC_log(LOG_ERR, "Condor submit file %s creation failed",
		       fn->str);
		g_string_free(fn, TRUE);
		return(DC_ERR_SYSTEM);
	}
	g_string_free(fn, TRUE);

	cfgval= DC_getCfgStr(CFG_ARCHITECTURES);
	fprintf(f, "Executable = %s\n", wu->data.client_name);
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
	fprintf(f, "log = %s\n", DC_LABEL_INTLOG);
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
	int ret;
	GString *src, *dst;

	if (!_DC_wu_check(wu))
		return(DC_ERR_UNKNOWN_WU);

	archs= DC_getCfgStr(CFG_ARCHITECTURES);

	src= g_string_new(wu->data.client_name);
	dst= g_string_new(wu->workdir);
	g_string_append(dst, "/");
	g_string_append(dst, wu->data.client_name);
	DC_log(LOG_DEBUG, "Copying client executable %s to %s",
	       src->str, dst->str);
	ret= _DC_copyFile(src->str, dst->str);
	g_string_free(src, TRUE);
	g_string_free(dst, TRUE);

	/*
	dst= g_string_new(wu->workdir);
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

	fn= g_string_new(wu->workdir);
	g_string_append(fn, "/");
	g_string_append(fn, CLIENT_CONFIG_NAME);
	if ((f= fopen(fn->str, "w")) != NULL)
	{
		fprintf(f, "LogLevel = %s\n", DC_getClientCfgStr("upper_case",
								 "LogLevel",
								 TRUE));
		fprintf(f, "LogFile = %s\n", DC_getClientCfgStr("upper_case",
								"LogFile",
								TRUE));
		
		fclose(f);
	}

	return(DC_OK);
}


DC_MasterEvent *
_DC_wu_check_client_messages(DC_Workunit *wu)
{
	GString *dn;
	DIR *d;
	struct dirent *de;
	int min_id;
	DC_MasterEvent *e= NULL;

	if (!_DC_wu_check(wu))
		return(NULL);

	dn= g_string_new(wu->workdir);
	g_string_append(dn, "/client_messages");
	d= opendir(dn->str);
	min_id= -1;
	while (d &&
	       (de= readdir(d)) != NULL)
	{
		char *found= strstr(de->d_name, "message.");
		if (found == de->d_name)
		{
			char *pos= strrchr(de->d_name, '.');
			if (pos)
			{
				int id= 0;
				pos++;
				id= strtol(pos, NULL, 10);
				if (id > 0)
				{
					if (min_id < 0)
						min_id= id;
					else
						if (id < min_id)
							min_id= id;
				}
			}
		}
	}
	if (d)
		closedir(d);
	/*else
	  DC_log(LOG_DEBUG, "No client message box: %s", dn->str);*/
	if (min_id >= 0)
	{
		FILE *f;
		g_string_append(dn, "/message.");
		g_string_append_printf(dn, "%d", min_id);
		if ((f= fopen(dn->str, "r")) != NULL)
		{
			gchar *cont;
			GError *err;
			if (g_file_get_contents(dn->str,
						&cont,
						NULL,
						&err))
			{
				/*e= g_new0(DC_MasterEvent, 1);*/
				e= _DC_event_create(wu, NULL, NULL,
						    cont);
				DC_log(LOG_DEBUG, "Message event created: %p "
				       "for wu (%p-\"%s\")", e, wu, wu->name);
				/*e->type= DC_MASTER_MESSAGE;
				e->wu= wu;
				e->message= cont;*/
				DC_log(LOG_DEBUG, "Message of the event: %s",
				       e->message);
			}
			else
				DC_log(LOG_ERR, "Failed to read client "
				       "message from %s",
				       dn->str);
			fclose(f);
			if (e)
				unlink(dn->str);
		}
		else
			DC_log(LOG_ERR, "Failed to open client message %s",
			       dn->str);
	}
	/*else
	  DC_log(LOG_DEBUG, "No message found in box %s", dn->str);*/
	g_string_free(dn, TRUE);
	return(e);
}


/* End of condor/condor_wu.c */
