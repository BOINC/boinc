/*
 * wu.C - BOINC work unit management
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <glib.h>
#include <math.h>

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

struct wu_params
{
	/* Resource control */
	double			rsc_fpops_est;
	double			rsc_fpops_bound;
	double			rsc_memory_bound;
	double			rsc_disk_bound;

	/* Redundancy */
	int			delay_bound;
	int			min_quorum;
	int			target_nresults;
	int			max_error_results;
	int			max_total_results;
	int			max_success_results;
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
	g_string_append(str, project_uuid_str);
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
	char uuid_str[37];

	uuid_unparse_lower(wu->uuid, uuid_str);
	return g_strdup_printf("%s_%s", label, uuid_str);
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

static int wu_uuid_equal(const void *a, const void *b)
{
	return uuid_compare((const unsigned char *)a,
		(const unsigned char *)b) == 0;
}

static unsigned wu_uuid_hash(const void *ptr)
{
	guint32 *h = (guint *)ptr;

	return h[0] ^ h[1] ^ h[2] ^ h[3];
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

	wu->workdir = get_workdir(wu->uuid, TRUE);
	if (!wu->workdir)
	{
		DC_destroyWU(wu);
		return NULL;
	}

	if (!wu_table)
		wu_table = g_hash_table_new_full(wu_uuid_hash, wu_uuid_equal,
			NULL, NULL);
	g_hash_table_insert(wu_table, wu->uuid, wu);
	++num_wus;

	return wu;
}

void DC_destroyWU(DC_Workunit *wu)
{
	if (!wu)
		return;

	if (wu_table)
		g_hash_table_remove(wu_table, wu->uuid);
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

	g_free(wu->client_name);
	g_strfreev(wu->argv);
	g_free(wu->tag);

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
			ret = _DC_copyFile(URL, file->path);
			if (ret)
			{
				_DC_destroyPhysicalFile(file);
				return ret;
			}
			break;
		case DC_FILE_PERSISTENT:
			ret = link(URL, file->path);
			if (ret)
			{
				ret = errno;
				DC_log(LOG_ERR, "Failed to link %s to %s: %s",
					URL, file->path, strerror(ret));
				_DC_destroyPhysicalFile(file);
				errno = ret;
				return DC_ERR_SYSTEM;
			}
			/* Remember the file mode */
			file->mode = DC_FILE_PERSISTENT;
			break;
		case DC_FILE_VOLATILE:
			ret = rename(URL, file->path);
			if (ret)
			{
				ret = errno;
				DC_log(LOG_ERR, "Failed to rename %s to %s: %s",
					URL, file->path, strerror(ret));
				_DC_destroyPhysicalFile(file);
				errno = ret;
				return DC_ERR_SYSTEM;
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

static void fill_wu_params(const DC_Workunit *wu, struct wu_params *params)
{
	memset(params, 0, sizeof(*params));

	/* XXX */
	params->rsc_fpops_est = 1e13;
	params->rsc_fpops_bound = 1e15;
	params->rsc_memory_bound = DC_getClientCfgInt(wu->client_name,
		CFG_MAXMEMUSAGE, 128 << 20, TRUE);
	params->rsc_disk_bound = DC_getClientCfgInt(wu->client_name,
		CFG_MAXDISKUSAGE, 64 << 20, TRUE);
	params->delay_bound = 360000;

	params->min_quorum = DC_getClientCfgInt(wu->client_name,
		CFG_REDUNDANCY, 1, TRUE);
	if (params->min_quorum < 1)
		params->min_quorum = 1;
	if (params->min_quorum == 2)
	{
		DC_log(LOG_NOTICE, "Quorum of 2 does not make sense for "
			"application %s, increasing to 3", wu->client_name);
		params->min_quorum++;
	}

	/* Calculate with logarithmic error */
	if (params->min_quorum == 1)
		params->target_nresults = 1;
	else
		params->target_nresults = params->min_quorum +
			(int)log(params->min_quorum);

	params->max_error_results = params->target_nresults +
		(int)log(params->min_quorum + 2) + 1;
	params->max_total_results = (params->target_nresults +
		(int)log(params->min_quorum + 2)) * 2;
	params->max_success_results = params->target_nresults +
		(int)log(params->min_quorum + 2) + 1;
}

static char *generate_wu_template(DC_Workunit *wu)
{
	struct wu_params params;
	GString *tmpl, *cmd;
	int i, num_inputs;
	GList *l;
	char *p;

	fill_wu_params(wu, &params);

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

	/* Concatenate the shell-quoted argv elements into a single string */
	cmd = g_string_new("");
	for (i = 0; i < wu->argc; i++)
	{
		char *quoted;

		if (i)
			g_string_append_c(cmd, ' ');
		quoted = g_shell_quote(wu->argv[i]);
		g_string_append(cmd, quoted);
		g_free(quoted);
	}
	g_string_append_printf(tmpl,
		"\t<command_line>%s</command_line>\n", cmd->str);
	g_string_free(cmd, TRUE);

	g_string_append_printf(tmpl,
		"\t<rsc_fpops_est>%g</rsc_fpops_est>\n", params.rsc_fpops_est);
	g_string_append_printf(tmpl,
		"\t<rsc_fpops_bound>%g</rsc_fpops_bound>\n",
		params.rsc_fpops_bound);
	g_string_append_printf(tmpl,
		"\t<rsc_memory_bound>%g</rsc_memory_bound>\n",
		params.rsc_memory_bound);
	g_string_append_printf(tmpl,
		"\t<rsc_disk_bound>%g</rsc_disk_bound>\n",
		params.rsc_disk_bound);
	g_string_append_printf(tmpl,
		"\t<delay_bound>%d</delay_bound>\n", params.delay_bound);
	g_string_append_printf(tmpl,
		"\t<min_quorum>%d</min_quorum>\n", params.min_quorum);
	g_string_append_printf(tmpl,
		"\t<target_nresults>%d</target_nresults>\n",
		params.target_nresults);
	g_string_append_printf(tmpl,
		"\t<max_error_results>%d</max_error_results>\n",
		params.max_error_results);
	g_string_append_printf(tmpl,
		"\t<max_success_results>%d</max_success_results>\n",
		params.max_success_results);
	g_string_append_printf(tmpl,
		"\t<max_total_results>%d</max_total_results>\n",
		params.max_total_results);

	p = tmpl->str;
	g_string_free(tmpl, FALSE);
	return p;
}

static void emit_file_info(FILE *tmpl, int idx, int auto_upload, int max_output)
{
	fprintf(tmpl, "<file_info>\n");
	fprintf(tmpl, "\t<name><OUTFILE_%d/></name>\n", idx);
	fprintf(tmpl, "\t<generated_locally/>\n");
	if (auto_upload)
		fprintf(tmpl, "\t<upload_when_present/>\n");
	fprintf(tmpl, "\t<max_nbytes>%d</max_nbytes>\n", max_output);
	fprintf(tmpl, "\t<url><UPLOAD_URL/></url>\n");
	fprintf(tmpl, "</file_info>\n");
}

static void emit_file_ref(FILE *tmpl, int idx, char *fmt, ...)
	G_GNUC_PRINTF(3, 4);
static void emit_file_ref(FILE *tmpl, int idx, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(tmpl, "\t<file_ref>\n");
	fprintf(tmpl, "\t\t<file_name><OUTFILE_%d/></name>\n", idx);
	fprintf(tmpl, "\t\t<open_name>");
	vfprintf(tmpl, fmt, ap);
	fprintf(tmpl, "</open_name>\n");
	fprintf(tmpl, "\t</file_ref>\n");
}

static char *generate_result_template(DC_Workunit *wu)
{
	char *file, *abspath, uuid_str[37], *cfgval;
	int i, file_cnt, max_output_size;
	FILE *tmpl;
	GList *l;

	uuid_unparse_lower(wu->uuid, uuid_str);
	file = g_strdup_printf("templates%cresult_%s.xml", G_DIR_SEPARATOR,
		uuid_str);
	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (!cfgval)
	{
		g_free(file);
		return NULL;
	}
	abspath = g_strdup_printf("%s%c%s", cfgval, G_DIR_SEPARATOR, file);
	free(cfgval);

	tmpl = fopen(abspath, "w");
	if (!tmpl)
	{
		DC_log(LOG_ERR, "Failed to create result template %s: %s",
			abspath, strerror(errno));
		g_free(file);
		g_free(abspath);
		return NULL;
	}

	max_output_size = DC_getClientCfgInt(wu->client_name, CFG_MAXOUTPUT,
		256 * 1024, TRUE);
	file_cnt = 0;
	/* Slots for output files */
	for (i = 0; i < wu->num_outputs; i++)
		emit_file_info(tmpl, file_cnt++, TRUE, max_output_size);
	/* Slots for subresults - no automatic uploading */
	for (i = 0; i < wu->subresults; i++)
		emit_file_info(tmpl, file_cnt++, FALSE, max_output_size);
	/* Slot for the checkpoint file */
	emit_file_info(tmpl, file_cnt++, TRUE, max_output_size);

	fprintf(tmpl, "<result>\n");
	file_cnt = 0;
	/* The output templates */
	for (l = wu->output_files; l; l = l->next)
		emit_file_ref(tmpl, file_cnt++, "%s", (char *)l->data);

	/* The subresult templates */
	for (i = 0; i < wu->subresults; i++)
	{
		emit_file_ref(tmpl, file_cnt, "%s%d", SUBRESULT_PFX, file_cnt);
		file_cnt++;
	}

	/* The checkpoint template */
	emit_file_ref(tmpl, file_cnt++, "%s", CKPT_LABEL_OUT);

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

char *_DC_getWUName(DC_Workunit *wu)
{
	char uuid_str[37];

	uuid_unparse_lower(wu->uuid, uuid_str);
	if (wu->tag)
		return g_strdup_printf("%s_%s_%s", project_uuid_str, uuid_str,
			wu->tag);
	else
		return g_strdup_printf("%s_%s", project_uuid_str, uuid_str);
}

int DC_submitWU(DC_Workunit *wu)
{
	char *wu_template, *result_template_file, **infiles, *result_path,
	     *name, *cfgval;
	struct wu_params params;
	int i, ret, ninputs;
	DB_WORKUNIT dbwu;
	GList *l;

	ret = install_input_files(wu);
	if (ret)
		return ret;

	dbwu.clear();

	name = _DC_getWUName(wu);
	snprintf(dbwu.name, sizeof(dbwu.name), "%s", name);
	g_free(name);

	dbwu.appid = lookup_appid(wu);
	if (dbwu.appid == -1)
		return DC_ERR_DATABASE;

	fill_wu_params(wu, &params);
	dbwu.batch = 1;
	dbwu.rsc_fpops_est = params.rsc_fpops_est;
	dbwu.rsc_fpops_bound = params.rsc_fpops_bound;
	dbwu.rsc_memory_bound = params.rsc_memory_bound;
	dbwu.rsc_disk_bound = params.rsc_disk_bound;
	dbwu.delay_bound = params.delay_bound;

	wu_template = generate_wu_template(wu);
	result_template_file = generate_result_template(wu);
	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (!cfgval)
	{
		errno = ENOMEM;
		return DC_ERR_SYSTEM;
	}
	result_path = g_strdup_printf("%s%c%s", cfgval, G_DIR_SEPARATOR,
		result_template_file);
	free(cfgval);

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

	ret = create_work(dbwu, wu_template, result_template_file, result_path,
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
	char id[2 * 36 + 2];
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

	strncpy(id, project_uuid_str, sizeof(id));
	id[36] = '_';
	uuid_unparse_lower(wu->uuid, id + 37);
	return strdup(id);
}

DC_Workunit *DC_unserialize(const char *id)
{
	return _DC_getWUByName(id);
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
	GMarkupParseContext *ctx;
	struct parser_state pctx;
	char *workdir, buf[256];
	DC_Workunit *wu;
	GError *error;
	FILE *f;

	workdir = get_workdir(uuid, FALSE);
	if (!g_file_test(workdir, G_FILE_TEST_IS_DIR))
	{
		g_free(workdir);
		return NULL;
	}

	wu = g_new0(DC_Workunit, 1);
	wu->workdir = workdir;
	memcpy(wu->uuid, uuid, sizeof(wu->uuid));

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

	if (!wu_table)
		wu_table = g_hash_table_new_full(wu_uuid_hash, wu_uuid_equal,
			NULL, NULL);
	g_hash_table_insert(wu_table, wu->uuid, wu);
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

	/* WU name syntax: <uuid> '_' <uuid> [ '_' <tag> ] */
	if (name[36] != '_' || (strlen(name) >= 73 && name[73] != '_'))
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

	if (wu_table)
	{
		wu = (DC_Workunit *)g_hash_table_lookup(wu_table, uuid);
		if (wu)
			return wu;
	}

#if 0
	wu = load_from_boinc_db(name);
	if (wu)
		return wu;
#endif

	return load_from_disk(uuid);
}

static void count_ready(void *key, void *value, void *ptr)
{
	DC_Workunit *wu = (DC_Workunit *)value;
	int *count = (int *)ptr;

	if (wu->state == DC_WU_READY)
		++(*count);
}

int DC_getWUNumber(DC_WUState state)
{
	DB_BASE db("", &boinc_db);
	int val, ret;
	char *query;

	switch (state)
	{
		case DC_WU_READY:
			val = 0;
			if (wu_table)
				g_hash_table_foreach(wu_table, count_ready,
					&val);
			return val;
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
