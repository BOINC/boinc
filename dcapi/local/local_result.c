/*
 * local/local_result.c
 *
 * DC-API functions related to result handling
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <glib.h>
#include "local_master.h"
#include "local_result.h"


DC_Result *
_DC_result_create(DC_Workunit *wu)
{
	DC_Result *r;
	
	r= g_new0(DC_Result, 1);
	r->wu= wu;
	return(r);
}


void
_DC_result_destroy(DC_Result *result)
{
	if (!result)
		return;
	g_free(result);
}


/********************************************************************
 * Functions
 */

DC_Result *_DC_createResult(const char *wu_name)
{
	char *logicalName;
	GList *l;
	DC_Result *result;
	DC_PhysicalFile *file;
	FILE *f;

	result = g_new0(DC_Result, 1);
	result->wu = _DC_getWUByName(wu_name);
	if (!result->wu)
	{
		DC_log(LOG_ERR, "Received result for unknown WU %s", wu_name);
		g_free(result);
		return NULL;
	}

	for (l = result->wu->output_files; l; l = l->next)
	{
		logicalName = g_strdup_printf("%s/%s", result->wu->workdir, (char *)l->data);
		f = fopen(logicalName, "r");
		if (f != NULL) /* File exists */
		{
			file = _DC_createPhysicalFile((char *)l->data, logicalName);
			result->output_files = g_list_append(result->output_files, file);
			result->num_outputs++;
			fclose(f);
		}
		g_free(logicalName);
	}

	return result;
}

void _DC_destroyResult(DC_Result *result)
{
	while (result->output_files)
	{
		_DC_destroyPhysicalFile((DC_PhysicalFile *)result->output_files->data);
		result->output_files = g_list_delete_link(result->output_files,
			result->output_files);
	}
	g_free(result);
}

unsigned DC_getResultCapabilities(const DC_Result *result)
{
	return (DC_GC_STDOUT | DC_GC_STDERR | DC_GC_EXITCODE);
}

DC_Workunit *DC_getResultWU(DC_Result *result)
{
	return result->wu;
}

static int wait_result(DC_Result *result)
{
	struct rusage rusage;
	pid_t pid;

	if (!result->wu->pid)
		return 0;

	pid = wait4(result->wu->pid, &result->exit_code, WNOHANG, &rusage);
	if (pid != result->wu->pid)
	{
		DC_log(LOG_ERR, "wu with pid %d not exited according to waitpid. retval: %d",
			 result->wu->pid, pid);
		return DC_ERR_SYSTEM;
	}

	result->cpu_time = rusage.ru_utime.tv_sec +
		(double)rusage.ru_utime.tv_usec / 1000000 +
		rusage.ru_stime.tv_sec +
		(double)rusage.ru_stime.tv_usec / 1000000;

	/* Make sure we do not call wait4() again */
	result->wu->pid = 0;

	return 0;
}

int DC_getResultExit(const DC_Result *result)
{
	int ret;

	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return 0.0;
	}

	ret = wait_result((DC_Result *)result); /* XXX */
	if (ret)
		return -1;

	if (!WIFEXITED(result->exit_code))
	{
		DC_log(LOG_WARNING, "DC_getResultExit: wu with pid %d terminated not normally.",
			result->wu->pid); /* XXX pid is 0 at this point */
		return -1;
	}

	return WEXITSTATUS(result->exit_code);
}

double DC_getResultCPUTime(const DC_Result *result)
{
	int ret;

	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return 0.0;
	}

	ret = wait_result((DC_Result *)result); /* XXX */
	if (ret)
		return -1;

	return result->cpu_time;
}

char *DC_getResultOutput(const DC_Result *result, const char *logicalFileName)
{
	char *physicalFileName;
	GList *l;

	for (l = result->output_files; l; l = l->next)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;

		if(!strcmp(file->label, logicalFileName))
		{
			physicalFileName = strdup(file->path);
			return physicalFileName;
		}
	}	

	if (!strcmp(logicalFileName, DC_LABEL_STDOUT))
	{
		char *tmp = g_strdup_printf("%s%c%s", result->wu->workdir, G_DIR_SEPARATOR, STDOUT_LABEL);
		physicalFileName = strdup(tmp);
		g_free(tmp);
		return physicalFileName;
	}

	if (!strcmp(logicalFileName, DC_LABEL_STDERR))
	{
		char *tmp = g_strdup_printf("%s%c%s", result->wu->workdir, G_DIR_SEPARATOR, STDERR_LABEL);
		physicalFileName = strdup(tmp);
		g_free(tmp);
		return physicalFileName;
	}

	DC_log(LOG_ERR, "DC_getResultOutput: The %s file is not part of the given result", logicalFileName);

	return NULL;
}


/* End of local/local_result.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
