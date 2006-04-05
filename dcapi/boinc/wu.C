#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>

#include <sched_util.h>
#include <sched_config.h>
#include <backend_lib.h>

#include "dc_boinc.h"

/********************************************************************
 * Constants
 */

typedef enum
{
	WU_INPUT_LABEL,
	WU_OUTPUT_LABEL,
	WU_TAG,
	WU_CLIENT_NAME,
	WU_CLIENT_ARG,
	WU_SUBRESULTS,
} wu_tag;

/* Buffer size used when copying a file */
#define COPY_BUFSIZE		65536


/********************************************************************
 * Data type definitions
 */

struct tag_desc
{
	wu_tag			id;
	const char		*name;
};

struct parser_state
{
	wu_tag			curr_tag;
	DC_FileMode		curr_mode;
	DC_Workunit		*wu;
};


/********************************************************************
 * Prototypes
 */

/* XML parser callback functions */
static void wudesc_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error);
static void wudesc_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error);


/********************************************************************
 * Global variables
 */

extern SCHED_CONFIG dc_boinc_config;

static GHashTable *wu_table;
static int num_wus;

static const struct tag_desc tags[] =
{
	{ WU_INPUT_LABEL,	"input_label" },
	{ WU_OUTPUT_LABEL,	"output_label" },
	{ WU_TAG,		"tag" },
	{ WU_CLIENT_NAME,	"client_name" },
	{ WU_CLIENT_ARG,	"client_arg" },
	{ WU_SUBRESULTS,	"subresults" }
};

static const GMarkupParser wudesc_parser =
{
	wudesc_start,
	NULL,
	wudesc_text,
	NULL,
	NULL
};


/********************************************************************
 * Helper functions
 */

/* Calculate & create the working directory. The working directory has the
 * form: <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid> Where <hash>
 * is the first 2 hex digits of the uuid
 */
static char *get_workdir(const uuid_t uuid, const char *uuid_str)
{
	GString *str;
	char *tmp;

	str = g_string_new(_DC_getCfgStr(CFG_WORKDIR));
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, ".dcapi-");
	g_string_append(str, project_uuid_str);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append_printf(str, "%02x", uuid[0]);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, uuid_str);

	tmp = str->str;
	g_string_free(str, FALSE);
	return tmp;
}

/* Get the full path of a file in the WU's working directory */
static char *get_workdir_path(DC_Workunit *wu, const char *label,
	WorkdirFile type)
{
	static const char *const pfx[] = { "in_", "out_", "checkpoint", "dc_" };

	if (type == FILE_CKPT)
		return g_strdup_printf("%s%ccheckpoint", wu->workdir,
			G_DIR_SEPARATOR);

	return g_strdup_printf("%s%c%s%s", wu->workdir, G_DIR_SEPARATOR,
		pfx[type], label);
}

/* Open a file in the WU's working directory */
static FILE *open_workdir_file(DC_Workunit *wu, const char *label,
	WorkdirFile type, const char *mode)
{
	char *name;
	FILE *f;

	name = get_workdir_path(wu, label, type);
	f = fopen(name, mode);
	if (!f)
		DC_log(LOG_ERR, "Failed to open %s (mode %s): %s",
			name, mode, strerror(errno));
	g_free(name);
	return f;
}

/* Returns the unique per-wu file name in the BOINC download hierarchy,
 * without path components */
static char *get_input_download_name(DC_Workunit *wu, const char *label)
{
	return g_strdup_printf("%s_%s", label, wu->uuid_str);
}

/* Returns the full path of the input file in the BOINC download hierarchy */
static char *get_input_download_path(DC_Workunit *wu, const char *label)
{
	char path[PATH_MAX], *filename;

	filename = get_input_download_name(wu, label);
	dir_hier_path(filename, _DC_getDownloadDir(), _DC_getUldlDirFanout(),
		path, TRUE);
	g_free(filename);
	return g_strdup(path);
}

/* Calculate the WU name from the uuid and tag */
static char *build_wu_name(const char *uuid_str, const char *tag)
{
	if (tag)
		return g_strdup_printf("%s_%s_%s", project_uuid_str,
			uuid_str, tag);
	else
		return g_strdup_printf("%s_%s", project_uuid_str, uuid_str);
}

static int copy_file(const char *src, const char *dst)
{
	int sfd, dfd;
	ssize_t ret;
	char *buf;

	buf = (char *)g_malloc(COPY_BUFSIZE);
	sfd = open(src, O_RDONLY);
	if (sfd == -1)
	{
		DC_log(LOG_ERR, "Failed to open %s for copying: %s", src,
			strerror(errno));
		g_free(buf);
		return -1;
	}
	dfd = open(dst, O_WRONLY | O_CREAT | O_TRUNC);
	if (dfd == -1)
	{
		DC_log(LOG_ERR, "Failed to create %s: %s", dst, strerror(errno));
		g_free(buf);
		close(sfd);
		return -1;
	}

	while ((ret = read(sfd, buf, COPY_BUFSIZE)) > 0)
	{
		char *ptr = buf;
		while (ret)
		{
			ssize_t ret2 = write(dfd, ptr, ret);
			if (ret2 < 0)
			{
				DC_log(LOG_ERR, "Error writing to %s: %s", dst,
					strerror(errno));
				close(sfd);
				close(dfd);
				unlink(dst);
				g_free(buf);
				return -1;
			}
			ret -= ret2;
			ptr += ret2;
		}
	}

	if (ret < 0)
	{
		DC_log(LOG_ERR, "Error reading from %s: %s", src, strerror(errno));
		close(sfd);
		close(dfd);
		g_free(buf);
		unlink(dst);
		return -1;
	}

	g_free(buf);
	close(sfd);
	if (close(dfd))
	{
		DC_log(LOG_ERR, "Error writing to %s: %s", dst, strerror(errno));
		unlink(dst);
		return -1;
	}
	return 0;
}

/********************************************************************
 * The WU description parser
 */

static void wudesc_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error)
{
	struct parser_state *pctx = (struct parser_state *)ptr;
	int i;

	for (i = 0; i < (int)(sizeof(tags) / sizeof(tags[0])); i++)
		if (!strcmp(tags[i].name, element_name))
			break;
	if (i > (int)(sizeof(tags) / sizeof(tags[0])))
	{
		*error = g_error_new(G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ELEMENT,
			"Unknown element %s", element_name);
		return;
	}

	if (tags[i].id == WU_INPUT_LABEL)
	{
		if (attr_names && attr_names[0] && !strcmp(attr_names[0], "type"))
			pctx->curr_mode = (DC_FileMode)atoi(attr_values[0]);
		else
			pctx->curr_mode = DC_FILE_REGULAR;
	}
	else if (attr_names)
	{
		*error = g_error_new(G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
			"Element %s does not take attributes", element_name);
		return;
	}

	pctx->curr_tag = tags[i].id;
}

static void wudesc_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error)
{
	struct parser_state *pctx = (struct parser_state *)ptr;
	DC_PhysicalFile *file;
	char *tmp, *label;

	switch (pctx->curr_tag)
	{
		case WU_INPUT_LABEL:
			label = g_strndup(text, text_len);
			tmp = get_workdir_path(pctx->wu, label, FILE_IN);
			file = _DC_createPhysicalFile(label, tmp);
			g_free(tmp);
			g_free(label);
			file->mode = pctx->curr_mode;
			pctx->wu->input_files =
				g_list_append(pctx->wu->input_files, file);
			pctx->wu->num_inputs++;
			break;
		case WU_OUTPUT_LABEL:
			label = g_strndup(text, text_len);
			pctx->wu->output_files =
				g_list_append(pctx->wu->output_files, label);
			pctx->wu->num_outputs++;
			break;
		case WU_TAG:
			pctx->wu->tag = g_strndup(text, text_len);
			break;
		case WU_CLIENT_NAME:
			pctx->wu->client_name = g_strndup(text, text_len);
			break;
		case WU_CLIENT_ARG:
			pctx->wu->argv = g_renew(char *, pctx->wu->argv,
				pctx->wu->argc + 1);
			pctx->wu->argv[pctx->wu->argc] = g_strndup(text, text_len);
			pctx->wu->argc++;
			break;
		case WU_SUBRESULTS:
			tmp = g_strndup(text, text_len);
			pctx->wu->subresults = atoi(tmp);
			g_free(tmp);
			break;
	}
}

/********************************************************************
 * API functions
 */

DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag)
{
	DC_Workunit *wu;
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
	wu->uuid_str = (char *)malloc(37);
	uuid_unparse_lower(wu->uuid, wu->uuid_str);

	/* We only need to store wu->name because it is the hash key */
	/* XXX Use wu->uuid as hash key and get rid of wu->name */
	wu->name = build_wu_name(wu->uuid_str, wu->tag);
	wu->workdir = get_workdir(wu->uuid, wu->uuid_str);
	ret = g_mkdir_with_parents(wu->workdir, 0700);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to create WU working directory %s: %s",
			wu->workdir, strerror(errno));
		DC_destroyWU(wu);
		return NULL;
	}

	if (!wu_table)
		wu_table = g_hash_table_new_full(g_str_hash, g_str_equal,
			NULL, NULL);
	g_hash_table_insert(wu_table, wu->name, wu);
	++num_wus;

	return wu;
}

void DC_destroyWU(DC_Workunit *wu)
{
	if (!wu)
		return;

	if (wu_table)
		g_hash_table_remove(wu_table, wu->name);
	--num_wus;

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
		DC_PhysicalFile *file = (DC_PhysicalFile *)wu->input_files->data;

		unlink(file->path);
		wu->input_files = g_list_delete_link(wu->input_files,
			wu->input_files);
		_DC_destroyPhysicalFile(file);
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
	g_free(wu->name);

	g_free(wu);
}

/* Check if the logical name is not already registered */
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
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;

		if (!strcmp(file->label, logicalFileName))
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
	DC_PhysicalFile *file;
	char *workpath;
	int ret;

	/* Sanity checks */
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	workpath = get_workdir_path(wu, logicalFileName, FILE_IN);
	file = _DC_createPhysicalFile(logicalFileName, workpath);
	g_free(workpath);
	if (!file)
		return DC_ERR_INTERNAL;

	switch (fileMode)
	{
		case DC_FILE_REGULAR:
			ret = copy_file(URL, file->path);
			if (ret)
			{
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			break;
		case DC_FILE_PERSISTENT:
			ret = link(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to link %s to %s: %s",
					URL, file->path, strerror(errno));
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			/* Remember the file mode */
			file->mode = DC_FILE_PERSISTENT;
			break;
		case DC_FILE_VOLATILE:
			ret = rename(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to rename %s to %s: %s",
					URL, file->path, strerror(errno));
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			break;
		default:
			DC_log(LOG_ERR, "Invalid file mode %d", fileMode);
			return DC_ERR_BADPARAM;
	}

	file->mode = fileMode;
	wu->input_files = g_list_append(wu->input_files, file);
	wu->num_inputs++;
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

/* This function installs input files to the Boinc download directory */
static int install_input_files(DC_Workunit *wu)
{
	char *dest;
	GList *l;
	int ret;

	for (l = wu->input_files; l; l = l->next)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;

		/* This also creates the directory if needed */
		dest = get_input_download_path(wu, file->label);
		/* By creating the hard link ourselves we prevent BOINC from
		 * copying the file later */
		ret = link(file->path, dest);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to install file %s to the "
				"Boinc download directory at %s", file->path,
				dest);
			g_free(dest);
			return DC_ERR_INTERNAL;
		}
		g_free(dest);
	}

	/* During resume, we have to install the checkpoint file as well */
	if (wu->ckpt_name)
	{
		char *src;

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
	int i, num_inputs;
	GString *tmpl;
	GList *l;
	char *p;

	/* Generate the file info block */
	num_inputs = wu->num_inputs;
	if (wu->ckpt_name)
		num_inputs++;

	tmpl = g_string_new("<file_info>\n");
	for (i = 0; i < num_inputs; i++)
		g_string_append_printf(tmpl, "\t<number>%d</number>\n", i);
	if (wu->ckpt_name)
		g_string_append_printf(tmpl, "\t<number>%d</number>\n", i);
	g_string_append(tmpl, "</file_info>\n");

	/* Generate the workunit description */
	g_string_append(tmpl, "<workunit>\n");
	for (i = 0, l = wu->input_files; l && i < wu->num_inputs;
			l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;

		g_string_append(tmpl, "\t<file_ref>\n");
		g_string_append_printf(tmpl,
			"\t\t<file_number>%d</file_number>\n", i);
		g_string_append_printf(tmpl,
			"\t\t<open_name>%s</open_name>\n", file->label);
		g_string_append(tmpl, "\t</file_ref>\n");
	}
	if (wu->ckpt_name)
	{
		g_string_append(tmpl, "\t<file_ref>\n");
		g_string_append_printf(tmpl,
			"\t\t<file_number>%d</file_number>\n", i++);
		g_string_append_printf(tmpl,
			"\t\t<open_name>%s</open_name>\n", CKPT_LABEL_IN);
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
	abspath = g_strdup_printf("%s%c%s", _DC_getCfgStr(CFG_PROJECTROOT),
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
		fprintf(tmpl, "\t\t<open_name>%s%d</open_name>\n",
			SUBRESULT_PFX, i - wu->num_outputs);
		fprintf(tmpl, "\t</file_ref>\n");
		i++;
	}

	/* The checkpoint template */
	fprintf(tmpl, "\t<file_ref>\n");
	fprintf(tmpl, "\t\t<file_name><OUTFILE_%d/></name>\n", i++);
	fprintf(tmpl, "\t\t<open_name>%s</open_name>\n", CKPT_LABEL_OUT);
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
	char **infiles, *result_path;
	char *wu_template, *result_template_file;
	int i, ret, ninputs;
	DB_WORKUNIT bwu;
	GList *l;

	ret = install_input_files(wu);
	if (ret)
		return ret;

	strncpy(bwu.name, wu->name, sizeof(bwu.name));

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
	result_path = g_strdup_printf("%s%c%s", _DC_getCfgStr(CFG_PROJECTROOT),
		G_DIR_SEPARATOR, result_template_file);

	/* Create the input file name array as required by create_work() */
	ninputs = wu->num_inputs;
	if (wu->ckpt_name)
		ninputs++;

	infiles = g_new0(char *, ninputs + 1);
	for (l = wu->input_files, i = 0; l && i < wu->num_inputs;
			l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;
		infiles[i] = get_input_download_name(wu, file->label);
	}
	if (wu->ckpt_name)
		infiles[i++] = get_input_download_name(wu, wu->ckpt_name);
	/* Terminator so we can use g_strfreev() later */
	infiles[i] = NULL;

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

	wu->state = DC_WU_RUNNING;

	return 0;
}

char *DC_serialize(DC_Workunit *wu)
{
	DC_PhysicalFile *file;
	GList *l;
	FILE *f;
	int i;

	if (!wu)
		return NULL;

	f = open_workdir_file(wu, "wu_desc.xml", FILE_DCAPI, "w");
	if (!f)
		return NULL;
	for (l = wu->input_files; l; l = l->next)
	{
		file = (DC_PhysicalFile *)l->data;
		fprintf(f, "<input_label type=%d>%s</input_label>\n", file->mode, file->label);
	}
	for (l = wu->output_files; l; l = l->next)
		fprintf(f, "<output_label%s</output_label>\n", (char *)l->data);
	fprintf(f, "<tag>%s</tag>\n", wu->tag);
	fprintf(f, "<client_name>%s</client_name>\n", wu->client_name);
	for (i = 0; i < wu->argc; i++)
		fprintf(f, "<client_arg>%s</client_arg>\n", wu->argv[i]);
	fprintf(f, "<subresults>%d</subresults>\n", wu->subresults);
	fclose(f);

	return g_strdup(wu->name);
}

#if 0
static DC_Workunit *load_from_boinc_db(const char *name)
{
	DB_WORKUNIT db_wu;
	DC_Workunit *wu;
	char *query;
	DB_APP app;
	int ret;

	query = g_strdup_printf("WHERE name = '%s'", name);
	ret = db_wu.lookup(query);
	g_free(query);
	if (!ret)
		return NULL;

	app.lookup_id(db_wu.appid);
}
#endif

static DC_Workunit *load_from_disk(const uuid_t uuid)
{
	char uuid_str[37], *workdir, buf[256];
	GMarkupParseContext *ctx;
	struct parser_state pctx;
	DC_Workunit *wu;
	GError *error;
	FILE *f;

	uuid_unparse_lower(uuid, uuid_str);
	workdir = get_workdir(uuid, uuid_str);
	if (!g_file_test(workdir, G_FILE_TEST_IS_DIR))
	{
		g_free(workdir);
		return NULL;
	}

	wu = g_new0(DC_Workunit, 1);
	wu->workdir = workdir;
	memcpy(wu->uuid, uuid, sizeof(wu->uuid));
	wu->uuid_str = g_strdup(uuid_str);

	f = open_workdir_file(wu, "wu_desc", FILE_DCAPI, "r");
	if (!f)
	{
		DC_destroyWU(wu);
		return NULL;
	}

	memset(&pctx, 0, sizeof(pctx));
	pctx.wu = wu;
	ctx = g_markup_parse_context_new(&wudesc_parser, (GMarkupParseFlags)0,
		&pctx, NULL);
	while (fgets(buf, sizeof(buf), f))
	{
		if (!g_markup_parse_context_parse(ctx, buf, strlen(buf),
				&error))
			goto error;
	}
	if (!g_markup_parse_context_end_parse(ctx, &error))
		goto error;

	g_markup_parse_context_free(ctx);
	fclose(f);

	wu->name = build_wu_name(wu->uuid_str, wu->tag);

	if (!wu_table)
		wu_table = g_hash_table_new_full(g_str_hash, g_str_equal,
			NULL, NULL);
	g_hash_table_insert(wu_table, wu->name, wu);
	++num_wus;

	return wu;

error:
	g_markup_parse_context_free(ctx);
	fclose(f);
	DC_destroyWU(wu);
	DC_log(LOG_ERR, "Failed to parse WU description: %s",
		error->message);
	g_error_free(error);
	return NULL;
}

DC_Workunit *_DC_getWUByName(const char *name)
{
	DC_Workunit *wu;
	char *uuid_str;
	uuid_t uuid;
	int ret;

	if (wu_table)
	{
		wu = (DC_Workunit *)g_hash_table_lookup(wu_table, name);
		if (wu)
			return wu;
	}

	/* Check if the WU belongs to this application */
	uuid_str = g_strndup(name, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name contains illegal UUID");
		return NULL;
	}

	if (uuid_compare(uuid, project_uuid))
	{
		DC_log(LOG_WARNING, "WU does not belong to this application");
		return NULL;
	}

	if (name[36] != '_')
	{
		DC_log(LOG_ERR, "Illegal WU name syntax");
		return NULL;
	}

	/* Check the WU's UUID */
	uuid_str = g_strndup(name + 37, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name contains illegal UUID");
		return NULL;
	}

#if 0
	wu = load_from_boinc_db(name);
	if (wu)
		return wu;
#endif

	return load_from_disk(uuid);
}

int DC_getWUNumber(DC_WUState state)
{
	DB_BASE db("", &boinc_db);
	int val, ret;
	char *query;

	switch (state)
	{
		case DC_WU_READY:
			/* XXX Should walk the wu_table */
			return -1;
		case DC_WU_RUNNING:
			query = g_strdup_printf("SELECT COUNT(*) "
				"FROM workunit wu, result res WHERE "
				"wu.name LIKE '%s\\_%%' AND "
				"(res.server_state = 2 OR res.server_state = 4)",
				project_uuid_str);
			ret = db.get_integer(query, val);
			g_free(query);
			if (ret)
				return -1;
			return val;
		case DC_WU_FINISHED:
			query = g_strdup_printf("SELECT COUNT(*) "
				"FROM workunit wu WHERE "
				"wu.name LIKE '%s\\_%%' AND "
				"wu.assimilate_state = %d",
				project_uuid_str, ASSIMILATE_READY);
			ret = db.get_integer(query, val);
			g_free(query);
			if (ret)
				return -1;
			return val;
		case DC_WU_SUSPENDED:
		case DC_WU_ABORTED:
		case DC_WU_UNKNOWN:
		default:
			return -1;
	}

}
