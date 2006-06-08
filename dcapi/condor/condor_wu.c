/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "dc.h"
#include "dc_common.h"

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
	fprintf(f, "Universe = vanilla\n");
	fprintf(f, "output = internal_output.txt\n");
	fprintf(f, "error = internal_error.txt\n");
	fprintf(f, "log = internal_log.txt\n");
	/*
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
	if (!ret)
		return(ret);

	return(DC_OK);
}


/* End of condor/condor_wu.c */
