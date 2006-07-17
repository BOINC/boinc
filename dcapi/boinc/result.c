/*
 * result.c - BOINC result management
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "dc_boinc.h"

/********************************************************************
 * Functions
 */

DC_Result *_DC_createResult(const char *wu_name, int db_id,
	const char *xml_doc_in)
{
	DC_Result *result;
	GList *l;

	result = g_new0(DC_Result, 1);
	result->wu = _DC_getWUByName(wu_name);
	if (!result->wu)
	{
		DC_log(LOG_ERR, "Received result for unknown WU %s", wu_name);
		g_free(result);
		return NULL;
	}

	if (!result->wu->db_id)
		result->wu->db_id = db_id;

	result->wu->state = DC_WU_FINISHED;

	result->output_files = _DC_parseFileRefs(xml_doc_in,
		&result->num_outputs);
	if (!result->output_files)
	{
		_DC_destroyResult(result);
		return NULL;
	}

	for (l = result->output_files; l; l = l->next)
	{
		char *upload_path, *workdir_path;
		DC_PhysicalFile *file;
		int ret;

		file = l->data;
		workdir_path = _DC_workDirPath(result->wu, file->label,
			FILE_OUT);
		upload_path = _DC_hierPath(file->path, TRUE);

		ret = link(upload_path, workdir_path);
		if (ret && errno != ENOENT)
		{
			ret = _DC_copyFile(upload_path, workdir_path);
			if (ret)
				DC_log(LOG_ERR, "Failed to copy the output "
					"file %s to %s", upload_path,
					workdir_path);
		}
	}
	return result;
}

void _DC_destroyResult(DC_Result *result)
{
	/* Mark the work unit as completed in the database */
	_DC_resultCompleted(result);

	while (result->output_files)
	{
		_DC_destroyPhysicalFile(result->output_files->data);
		result->output_files = g_list_delete_link(result->output_files,
			result->output_files);
	}
	g_free(result);
}

DC_GridCapabilities DC_getResultCapabilities(const DC_Result *result)
{
	unsigned caps;
	char *path;

	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return 0;
	}

	caps = DC_GC_EXITCODE;
	path = DC_getResultOutput(result, DC_LABEL_STDOUT);
	if (path)
	{
		caps |= DC_GC_STDOUT;
		free(path);
	}
	path = DC_getResultOutput(result, DC_LABEL_STDERR);
	if (path)
	{
		caps |= DC_GC_STDERR;
		free(path);
	}
	path = DC_getResultOutput(result, DC_LABEL_CLIENTLOG);
	if (path)
	{
		caps |= DC_GC_LOG;
		free(path);
	}

	return caps;
}

DC_Workunit *DC_getResultWU(DC_Result *result)
{
	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return NULL;
	}

	return result->wu;
}

int DC_getResultExit(const DC_Result *result)
{
	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return -1;
	}

	return result->exit_code;
}

char *DC_getResultOutput(const DC_Result *result, const char *logicalFileName)
{
	char *path, *wuname;
	struct stat st;
	GList *l;
	int ret;

	if (!result)
	{
		DC_log(LOG_ERR, "%s: Missing result", __func__);
		return NULL;
	}
	if (!logicalFileName)
	{
		DC_log(LOG_ERR, "%s: Missing logical file name", __func__);
		return NULL;
	}

	for (l = result->output_files; l; l = l->next)
	{
		DC_PhysicalFile *file = l->data;

		if (strcmp(file->label, logicalFileName))
			continue;

		path = _DC_workDirPath(result->wu, file->label, FILE_OUT);

		/* Treat empty files as non-existent */
		ret = stat(path, &st);
		if (ret || !st.st_size)
		{
			g_free(path);
			return NULL;
		}

		if (!g_mem_is_system_malloc())
		{
			char *tmp;

			tmp = strdup(path);
			g_free(path);
			path = tmp;
		}
		return path;
	}

	wuname = _DC_getWUName(result->wu);
	DC_log(LOG_WARNING, "WU %s does not have an output file named %s",
		wuname, logicalFileName);
	g_free(wuname);
	return NULL;
}
