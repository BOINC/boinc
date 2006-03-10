#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <glib.h>

#include <sched_util.h>
#include <sched_config.h>
#include <backend_lib.h>

#include "dc_boinc.h"

extern SCHED_CONFIG dc_boinc_config;

DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag)
{
	char uuid_str[37];
	DC_Workunit *wu;
	GString *str;
	int ret;

	if (!clientName)
	{
		DC_log(LOG_ERR, "DC_createWU: clientName is not supplied");
		return NULL;
	}

	wu = g_new0(DC_Workunit, 1);
	wu->client_name = g_strdup(clientName);
	wu->argv = g_strdupv((char **)arguments);

	for (wu->argc = 0; arguments && arguments[wu->argc]; wu->argc++)
		/* Nothing */;

	wu->subresults = subresults;
	wu->tag = g_strdup(tag);

	uuid_generate(wu->uuid);
	uuid_unparse_lower(wu->uuid, uuid_str);
	wu->uuid_str = g_strdup(uuid_str);

	/* Calculate & create the working directory. The working directory
	 * has the form:
	 * <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid>
	 * Where <hash> is the first 2 hex digits of the uuid
	 */
	str = g_string_new(_DC_getCfgStr(CFG_WORKDIR));
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, ".dcapi-");
	g_string_append(str, project_uuid_str);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append_printf(str, "%02x", wu->uuid[0]);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, uuid_str);

	ret = g_mkdir_with_parents(str->str, 0700);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to create WU working directory %s: %s",
			str->str, strerror(errno));
		DC_destroyWU(wu);
		return NULL;
	}

	wu->workdir = str->str;
	g_string_free(str, FALSE);

	return wu;
}

static char *get_workdir_path(DC_Workunit *wu, const char *label,
	WorkdirFile type)
{

	return g_strdup_printf("%s%cin_%s", wu->workdir, G_DIR_SEPARATOR, label);
}

void DC_destroyWU(DC_Workunit *wu)
{
	if (!wu)
		return;

	switch (wu->state)
	{
		case DC_WU_RUNNING:
			/* XXX Abort the work unit */
			break;
		default:
			break;
	}

	while (wu->input_files)
	{
		char *path = get_workdir_path(wu,
			(const char *)wu->input_files->data, FILE_IN);
		unlink(path);
		g_free(path);
		g_free(wu->input_files->data);
		wu->input_files = g_list_delete_link(wu->input_files,
			wu->input_files);
	}

	while (wu->output_files)
	{
		g_free(wu->output_files->data);
		wu->output_files = g_list_delete_link(wu->output_files,
			wu->output_files);
	}

	if (wu->ckpt_name)
	{
		char *path = get_workdir_path(wu, wu->ckpt_name, FILE_CKPT);
		unlink(path);
		g_free(path);
		g_free(wu->ckpt_name);
	}

	if (wu->workdir)
	{
		const char *name;
		GDir *dir;
		int ret;

		dir = g_dir_open(wu->workdir, 0, NULL);
		/* The work directory should not contain any extra files, but
		 * just in case */
		while (dir && (name = g_dir_read_name(dir)))
		{
			GString *str = g_string_new(wu->workdir);
			g_string_append_c(str, G_DIR_SEPARATOR);
			g_string_append(str, name);
			DC_log(LOG_INFO, "Removing unknown file %s",
				str->str);
			unlink(str->str);
			g_string_free(str, TRUE);
		}
		if (dir)
			g_dir_close(dir);

		ret = rmdir(wu->workdir);
		if (ret)
			DC_log(LOG_WARNING, "Failed to remove WU working "
				"directory %s: %s", wu->workdir,
				strerror(errno));
		g_free(wu->workdir);
	}

	g_free(wu->uuid_str);
	g_free(wu->client_name);
	g_strfreev(wu->argv);
	g_free(wu->tag);

	g_free(wu);
}

static int check_logical_name(DC_Workunit *wu, const char *logicalFileName)
{
	GList *l;

	if (strchr(logicalFileName, '/') || strchr(logicalFileName, '\\'))
	{
		DC_log(LOG_ERR, "Illegal characters in logical file name %s",
			logicalFileName);
		return DC_ERR_BADPARAM;
	}
	for (l = wu->input_files; l; l = l->next)
	{
		if (!strcmp((char *)l->data, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
				"input file", logicalFileName);
			return DC_ERR_BADPARAM;
		}
	}
	for (l = wu->output_files; l; l = l->next)
	{
		if (!strcmp((char *)l->data, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
				"output file", logicalFileName);
			return DC_ERR_BADPARAM;
		}
	}
	return 0;
}

int DC_addWUInput(DC_Workunit *wu, const char *logicalFileName, const char *URL,
	DC_FileMode fileMode)
{
	char *workpath;
	int ret;

	/* Sanity checks */
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	workpath = get_workdir_path(wu, logicalFileName, FILE_IN);
	switch (fileMode)
	{
		case DC_FILE_REGULAR:
		{
			GString *cmd;
			char *quoted;

			cmd = g_string_new("/bin/cp -a ");
			quoted = g_shell_quote(URL);
			g_string_append(cmd, quoted);
			g_free(quoted);
			g_string_append_c(cmd, ' ');
			quoted = g_shell_quote(workpath);
			g_string_append(cmd, quoted);
			g_free(quoted);

			/* XXX Re-implement this in C to save the overhead
			 * of shell invocation */
			ret = system(cmd->str);
			if (ret)
			{
				DC_log(LOG_ERR, "Executing \"%s\" failed: %s",
					cmd->str, strerror(errno));
				g_string_free(cmd, TRUE);
				g_free(workpath);
				return DC_ERR_BADPARAM; /* XXX */
			}
			g_string_free(cmd, TRUE);
			break;
		}
		case DC_FILE_PERSISTENT:
			ret = link(URL, workpath);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to link %s to %s: %s",
					URL, workpath, strerror(errno));
				g_free(workpath);
				return DC_ERR_BADPARAM; /* XXX */
			}
			/* XXX Further optimization: remember that the file was
			 * persistent and add <sticky/> and <report_on_rpc/>
			 * tags to the <file_info> description in the WU
			 * template */
			break;
		case DC_FILE_VOLATILE:
			ret = rename(URL, workpath);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to rename %s to %s: %s",
					URL, workpath, strerror(errno));
				g_free(workpath);
				return DC_ERR_BADPARAM; /* XXX */
			}
			break;
	}

	wu->input_files = g_list_append(wu->input_files,
		g_strdup(logicalFileName));
	wu->num_inputs++;
	g_free(workpath);
	return 0;
}

int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName)
{
	int ret;

	/* Sanity checks */
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	wu->output_files = g_list_append(wu->output_files,
		g_strdup(logicalFileName));
	wu->num_outputs++;
	return 0;
}

static char *get_input_download_path(DC_Workunit *wu, const char *label)
{
	char path[PATH_MAX], *filename;
	filename = g_strdup_printf("%s_%s", label, wu->uuid_str);
	dir_hier_path(filename, _DC_getDownloadDir(), _DC_getUldlDirFanout(),
		path, TRUE);
	g_free(filename);
	return g_strdup(path);
}

/* This function installs input files to the Boinc download directory */
static int install_input_files(DC_Workunit *wu)
{
	char *src, *dest;
	GList *l;
	int ret;

	for (l = wu->input_files; l; l = l->next)
	{
		src = get_workdir_path(wu, (char *)l->data, FILE_IN);
		dest = get_input_download_path(wu, (char *)l->data);
		ret = link(src, dest);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to install file %s to the "
				"Boinc download directory at %s", src, dest);
			g_free(src);
			g_free(dest);
			return DC_ERR_INTERNAL;
		}
		g_free(src);
		g_free(dest);
	}

	/* During resume, we have to install the checkpoint file as well */
	if (wu->ckpt_name)
	{
		src = get_workdir_path(wu, wu->ckpt_name, FILE_CKPT);
		dest = get_input_download_path(wu, wu->ckpt_name);
		ret = link(src, dest);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to install file %s to the "
				"Boinc download directory at %s", src, dest);
			g_free(src);
			g_free(dest);
			return DC_ERR_INTERNAL;
		}
		g_free(src);
		g_free(dest);
	}

	return 0;
}

static char *generate_wu_template(DC_Workunit *wu)
{
	GString *tmpl;
	GList *l;
	char *p;
	int i;

	/* Generate the file info block */
	tmpl = g_string_new("<file_info>\n");
	for (i = 0; i < wu->num_inputs; i++)
		g_string_append_printf(tmpl, "\t<number>%d</number>\n", i);
	g_string_append(tmpl, "</file_info>\n");

	/* Generate the workunit description */
	g_string_append(tmpl, "<workunit>\n");
	for (i = 0, l = wu->input_files; l && i < wu->num_inputs;
			l = l->next, i++)
	{
		g_string_append(tmpl, "\t<file_ref>\n");
		g_string_append_printf(tmpl,
			"\t\t<file_number>%d</file_number>\n", i);
		g_string_append_printf(tmpl,
			"\t\t<open_name>%s</open_name>\n", (char *)l->data);
		g_string_append(tmpl, "\t</file_ref>\n");
	}

	/* XXX Should qoute the individual arguments */
	p = g_strjoinv(" ", wu->argv);
	g_string_append_printf(tmpl,
		"\t<command_line>%s</command_line>\n", p);
	g_free(p);

	g_string_append(tmpl,
		"\t<rsc_fpops_est>1e13</rsc_fpops_est>\n"); /* XXX Check */
	g_string_append(tmpl,
		"\t<rsc_fpops_bound>1e15</rsc_fpops_bound>\n"); /* XXX Check */
	g_string_append(tmpl,
		"\t<rsc_memory_bound>10000000</rsc_memory_bound>\n");
	g_string_append(tmpl,
		"\t<rsc_disk_bound>1000000</rsc_disk_bound>\n");
	g_string_append(tmpl,
		"\t<delay_bound>360000</delay_bound>\n");
	g_string_append(tmpl,
		"\t<min_quorum>1</min_quorum>\n");
	g_string_append(tmpl,
		"\t<target_nresults>1</target_nresults>\n");
	g_string_append(tmpl,
		"\t<max_error_results>3</max_error_results>\n");
	g_string_append(tmpl,
		"\t<max_success_results>3</max_total_results>\n");
	g_string_append(tmpl,
		"\t<max_total_results>4</max_total_results>\n");

	p = tmpl->str;
	g_string_free(tmpl, FALSE);
	return p;
}

static char *generate_result_template(DC_Workunit *wu)
{
	char *file, *abspath;
	FILE *tmpl;
	GList *l;
	int i;

	file = g_strdup_printf("templates%cresult_%s.xml", G_DIR_SEPARATOR,
		wu->uuid_str);
	abspath = g_strdup_printf("%s%c%s", _DC_getProjectRoot(),
		G_DIR_SEPARATOR, file);

	tmpl = fopen(abspath, "w");
	if (!tmpl)
	{
		DC_log(LOG_ERR, "Failed to create result template %s: %s",
			abspath, strerror(errno));
		g_free(file);
		g_free(abspath);
		return NULL;
	}

	/* We need a slot for
	 * - every output file
	 * - every possible subresult
	 * - the checkpoint file */
	for (i = 0; i < wu->num_outputs + wu->subresults + 1; i++)
	{
		fprintf(tmpl, "<file_info>\n");
		fprintf(tmpl, "\t<name><OUTFILE_%d/></name>\n", i);
		fprintf(tmpl, "\t<generated_locally/>\n");
		fprintf(tmpl, "\t<upload_when_present/>\n");
		fprintf(tmpl, "\t<max_nbytes>262144</max_nbytes>\n"); /* XXX */
		fprintf(tmpl, "\t<url><UPLOAD_URL/></url>\n");
		fprintf(tmpl, "</file_info>\n");
	}

	fprintf(tmpl, "<result>\n");
	/* The output templates */
	for (l = wu->output_files, i = 0; l && i < wu->num_outputs;
			l = l->next, i++)
	{
		fprintf(tmpl, "\t<file_ref>\n");
		fprintf(tmpl, "\t\t<file_name><OUTFILE_%d/></name>\n", i);
		fprintf(tmpl, "\t\t<open_name>%s</open_name>\n",
			(char *)l->data);
		fprintf(tmpl, "\t</file_ref>\n");
	}

	/* The subresult templates */
	while (i < wu->num_outputs + wu->subresults)
	{
		fprintf(tmpl, "\t<file_ref>\n");
		fprintf(tmpl, "\t\t<file_name><OUTFILE_%d/></name>\n", i);
		fprintf(tmpl, "\t\t<open_name>_dc_subresult_%d</open_name>\n", /* XXX check name */
			i - wu->num_outputs);
		fprintf(tmpl, "\t</file_ref>\n");
		i++;
	}

	/* The checkpoint template */
	fprintf(tmpl, "\t<file_ref>\n");
	fprintf(tmpl, "\t\t<file_name><OUTFILE_%d/></name>\n", i);
	fprintf(tmpl, "\t\t<open_name>_dc_checkpoint</open_name>\n"); /* XXX check name */
	fprintf(tmpl, "\t</file_ref>\n");

	fprintf(tmpl, "</result>\n");
	fclose(tmpl);

	g_free(abspath);
	return file;
}

static int lookup_appid(DC_Workunit *wu)
{
	char *query;
	DB_APP app;

	query = g_strdup_printf("WHERE name = '%s'", wu->client_name);
	if (app.lookup(query))
	{
		DC_log(LOG_ERR, "Failed to look up application %s",
			wu->client_name);
		g_free(query);
		return -1;
	}
	g_free(query);
	return app.get_id();
}

int DC_submitWU(DC_Workunit *wu)
{
	char **infiles, *wu_name, *result_path;
	char *wu_template, *result_template_file;
	DB_WORKUNIT bwu;
	int i, ret;
	GList *l;

	ret = install_input_files(wu);
	if (ret)
		return ret;

	if (wu->tag)
		wu_name = g_strdup_printf("%s_%s_%s", project_uuid_str,
			wu->uuid_str, wu->tag);
	else
		wu_name = g_strdup_printf("%s_%s", project_uuid_str,
			wu->uuid_str);
	strncpy(bwu.name, wu_name, sizeof(bwu.name));
	g_free(wu_name);

	bwu.appid = lookup_appid(wu);
	if (bwu.appid == -1)
		return DC_ERR_DATABASE;

	/* XXX Make these compatible with the template */
	bwu.batch = 1;
	bwu.rsc_fpops_est = 200000 ;
	bwu.rsc_fpops_bound = 2000000000 ;
	bwu.rsc_memory_bound = 2000000;
	bwu.rsc_disk_bound = 2000000;
	bwu.delay_bound = 720000;

	wu_template = generate_wu_template(wu);
	result_template_file = generate_result_template(wu);
	result_path = g_strdup_printf("%s%c%s", _DC_getProjectRoot(),
		G_DIR_SEPARATOR, result_template_file);

	infiles = g_new0(char *, wu->num_inputs + 1);
	for (l = wu->input_files, i = 0; l && i < wu->num_inputs;
			l = l->next, i++)
		infiles[i] = g_strdup((char *)l->data);

	ret = create_work(bwu, wu_template, result_template_file, result_path,
		const_cast<const char **>(infiles), wu->num_inputs,
		dc_boinc_config);
	g_free(result_path);
	g_strfreev(infiles);
	g_free(result_template_file);
	g_free(wu_template);

	if (ret)
	{
		DC_log(LOG_ERR, "Failed to create Boinc work unit");
		return DC_ERR_DATABASE;
	}

	return 0;
}
