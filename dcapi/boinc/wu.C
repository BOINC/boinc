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

#include <openssl/sha.h>

#include <sched_util.h>
#include <sched_config.h>
#include <backend_lib.h>
#include <md5_file.h>

#include "dc_boinc.h"

/********************************************************************
 * Constants
 */

typedef enum
{
	WU_NOTAG = -1,
	WU_WUDESC,
	WU_INPUT_LABEL,
	WU_REMOTE_IN_LABEL,
	WU_OUTPUT_LABEL,
	WU_TAG,
	WU_CLIENT_NAME,
	WU_CLIENT_ARG,
	WU_SUBRESULTS,
	WU_SUBMITTED,
	WU_SERIALIZED,
	WU_SUSPENDED,
	WU_NOSUSPEND,
	WU_NATIVECLIENT,
	WU_PRIORITY,
	WU_BATCH
} wu_tag;

#define WU_DESC_FILE		"wu_desc.xml"


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
	char			*curr_url;
	char			*curr_md5;
	size_t			curr_size;
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

int DC_addWUOutputExt(DC_Workunit *wu, const char *logicalFileName, int flags);

/* XML parser callback functions */
static void wudesc_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error);
static void wudesc_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error);
static void wudesc_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error);


/********************************************************************
 * Global variables
 */

extern SCHED_CONFIG dc_boinc_config G_GNUC_INTERNAL;

static GHashTable *wu_table;
static int num_wus;

static const struct tag_desc tags[] =
{
	{ WU_WUDESC,		"wudesc" },
	{ WU_INPUT_LABEL,	"input_label" },
	{ WU_REMOTE_IN_LABEL,	"remote_input_label" },
	{ WU_OUTPUT_LABEL,	"output_label" },
	{ WU_TAG,		"tag" },
	{ WU_CLIENT_NAME,	"client_name" },
	{ WU_CLIENT_ARG,	"client_arg" },
	{ WU_SUBRESULTS,	"subresults" },
	{ WU_SUBMITTED,		"submitted" },
	{ WU_SUSPENDED,		"suspended" },
	{ WU_SERIALIZED,	"serialized" },
	{ WU_NOSUSPEND,		"nosuspend" },
	{ WU_NATIVECLIENT,	"nativeclient" },
	{ WU_PRIORITY,		"priority" },
	{ WU_BATCH,		"batch" }
};

static const GMarkupParser wudesc_parser =
{
	wudesc_start,
	wudesc_end,
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
char *_DC_workDirPath(const DC_Workunit *wu, const char *label, WorkdirFile type)
{
	static const char *const pfx[] = { "in_", "out_", "checkpoint", "dc_" };

	if (type == FILE_CKPT)
		return g_strdup_printf("%s%ccheckpoint", wu->workdir,
			G_DIR_SEPARATOR);

	return g_strdup_printf("%s%c%s%s", wu->workdir, G_DIR_SEPARATOR,
		pfx[type], label);
}

/* Open a file in the WU's working directory */
static FILE *open_workdir_file(const DC_Workunit *wu, const char *label,
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

/* Returns the unique per-wu file name in the BOINC download hierarchy,
 * without path components */
static char *get_input_download_name(DC_Workunit *wu, const char *label, const char *physicalfilename)
{
	char uuid_str[37];

	if (physicalfilename)
		return g_strdup(physicalfilename);
	else
	{
		uuid_unparse_lower(wu->uuid, uuid_str);
		return g_strdup_printf("%s_%s", label, uuid_str);
	}
}

/* Returns the full path of the input file in the BOINC download hierarchy */
static char *get_input_download_path(DC_Workunit *wu, const char *label, const char *physicalfilename)
{
	char *filename, *path;

	filename = get_input_download_name(wu, label, physicalfilename);
	path = _DC_hierPath(filename, FALSE);
	g_free(filename);
	return path;
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

static DC_Workunit *alloc_wu(void)
{
	DC_Workunit *wu;

	wu = g_new0(DC_Workunit, 1);
	wu->is_optional = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	wu->refcnt = 1;
	return wu;
}

/**
 * Perform regular expression replacement for a given URL.
 * @param url the URL to perform the replace on
 * @return NULL-terminated string array of replacement result(s)
 */
static gchar **replace_regex(const char *url)
{
	if (!url)
		return NULL;

	char *regex = DC_getCfgStr(CFG_REGEXPMATCH);
	if (!regex)
		return NULL;

	char *replace = DC_getCfgStr(CFG_REGEXPREPLACE);
	if (!replace)
	{
		g_free(regex);
		return NULL;
	}

	GRegex *reg = g_regex_new(regex, (GRegexCompileFlags)0, (GRegexMatchFlags)0, NULL);
	if (!reg)
	{
		g_free(regex);
		g_free(replace);
		return NULL;
	}

	gchar *res = g_regex_replace(reg, url, -1, 0, replace, (GRegexMatchFlags)0, NULL);
	if (!res)
	{
		g_free(regex);
		g_free(replace);
		return NULL;
	}

	return g_strsplit(res, "\n", 0);
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
	if (i >= (int)(sizeof(tags) / sizeof(tags[0])))
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
	else if (tags[i].id == WU_REMOTE_IN_LABEL)
	{
		int j = 0;
		for (; j < 3; j++) {
			if (attr_names && attr_names[j] && !strcmp(attr_names[j], "url"))
				pctx->curr_url = g_strdup(attr_values[j]);
			if (attr_names && attr_names[j] && !strcmp(attr_names[j], "md5"))
				pctx->curr_md5 = g_strdup(attr_values[j]);
			if (attr_names && attr_names[j] && !strcmp(attr_names[j], "size"))
				pctx->curr_size = atoll(attr_values[j]);
		}
	}
	else if (attr_names && attr_names[0])
	{
		*error = g_error_new(G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
			"Element %s does not take attributes", element_name);
		return;
	}

	switch (tags[i].id)
	{
		case WU_SERIALIZED:
			pctx->wu->serialized = TRUE;
			break;
		case WU_SUBMITTED:
			pctx->wu->submitted = TRUE;
			break;
		case WU_SUSPENDED:
			pctx->wu->suspended = TRUE;
			break;
		case WU_NOSUSPEND:
			pctx->wu->nosuspend = TRUE;
			break;
		case WU_NATIVECLIENT:
			pctx->wu->nativeclient = TRUE;
			break;
		default:
			break;
	}

	pctx->curr_tag = tags[i].id;
}

static void wudesc_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error)
{
	struct parser_state *pctx = (struct parser_state *)ptr;

	pctx->curr_tag = WU_NOTAG;
}

static void wudesc_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error)
{
	struct parser_state *pctx = (struct parser_state *)ptr;
	DC_PhysicalFile *file;
	DC_RemoteFile *rfile;
	char *tmp, *label;

	switch (pctx->curr_tag)
	{
		case WU_INPUT_LABEL:
			label = g_strndup(text, text_len);
			tmp = _DC_workDirPath(pctx->wu, label, FILE_IN);
			file = _DC_createPhysicalFile(label, tmp);
			g_free(tmp);
			g_free(label);
			file->mode = pctx->curr_mode;
			pctx->wu->input_files =
				g_list_append(pctx->wu->input_files, file);
			pctx->wu->num_inputs++;
			break;
		case WU_REMOTE_IN_LABEL:
			label = g_strndup(text, text_len);
			rfile = _DC_createRemoteFile(label, pctx->curr_url,
				pctx->curr_md5, pctx->curr_size);
			pctx->wu->remote_input_files =
				g_list_append(pctx->wu->remote_input_files, rfile);
			pctx->wu->num_remote_inputs++;
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
				pctx->wu->argc + 2);
			pctx->wu->argv[pctx->wu->argc++] = g_strndup(text, text_len);
			pctx->wu->argv[pctx->wu->argc] = NULL;
			break;
		case WU_SUBRESULTS:
			tmp = g_strndup(text, text_len);
			pctx->wu->subresults = atoi(tmp);
			g_free(tmp);
			break;
		case WU_PRIORITY:
			tmp = g_strndup(text, text_len);
			pctx->wu->priority = atoi(tmp);
			g_free(tmp);
			break;
		case WU_BATCH:
			tmp = g_strndup(text, text_len);
			pctx->wu->batch = atoi(tmp);
			g_free(tmp);
			break;
		case WU_WUDESC:
			break;
		case WU_SUSPENDED:
		case WU_SUBMITTED:
		case WU_SERIALIZED:
		case WU_NOSUSPEND:
		case WU_NATIVECLIENT:
			/* XXX Emit error? */
			break;
		default:
			break;
	}
}

static int write_wudesc(const DC_Workunit *wu)
{
	DC_PhysicalFile *file;
	DC_RemoteFile *rfile;
	GList *l;
	FILE *f;
	int i;

	f = open_workdir_file(wu, WU_DESC_FILE, FILE_DCAPI, "w");
	if (!f)
		return DC_ERR_SYSTEM;

	fprintf(f, "<wudesc>\n");
	if (wu->serialized)
		fprintf(f, "\t<serialized/>\n");
	if (wu->submitted)
		fprintf(f, "\t<submitted/>\n");
	if (wu->suspended)
		fprintf(f, "\t<suspended/>\n");
	if (wu->nosuspend)
		fprintf(f, "\t<nosuspend/>\n");
	if (wu->nativeclient)
		fprintf(f, "\t<nativeclient/>\n");
	fprintf(f, "\t<tag>%s</tag>\n", wu->tag);

	fprintf(f, "\t<client_name>%s</client_name>\n", wu->client_name);
	for (i = 0; i < wu->argc; i++)
		fprintf(f, "\t<client_arg>%s</client_arg>\n", wu->argv[i]);

	for (l = wu->input_files; l; l = l->next)
	{
		file = (DC_PhysicalFile *)l->data;
		fprintf(f, "\t<input_label type=\"%d\">%s</input_label>\n",
			file->mode, file->label);
	}

	for (l = wu->remote_input_files; l; l = l->next)
	{
		rfile = (DC_RemoteFile *)l->data;
		fprintf(f, "\t<remote_input_label url=\"%s\" md5=\"%s\" size=\"%zu\">%s</remote_input_label>\n",
			rfile->url, rfile->remotefilehash, rfile->remotefilesize, rfile->label);
	}

	for (l = wu->output_files; l; l = l->next)
		fprintf(f, "\t<output_label>%s</output_label>\n", (char *)l->data);

	fprintf(f, "\t<subresults>%d</subresults>\n", wu->subresults);
	fprintf(f, "\t<priority>%d</priority>\n", wu->priority);
	fprintf(f, "\t<batch>%d</batch>\n", wu->batch);

	fprintf(f, "</wudesc>\n");
	fclose(f);
	return 0;
}

/********************************************************************
 * API functions
 */

static int generate_client_config(DC_Workunit *wu)
{
	DC_PhysicalFile *file;
	char *workpath;
	FILE *f;

	workpath = _DC_workDirPath(wu, DC_CONFIG_FILE, FILE_IN);
	file = _DC_createPhysicalFile(DC_CONFIG_FILE, workpath);
	g_free(workpath);
	if (!file)
		return DC_ERR_INTERNAL;
	file->mode = DC_FILE_VOLATILE;

	f = fopen(file->path, "w");
	if (!f)
	{
		DC_log(LOG_ERR, "Failed to create file %s: %s", file->path,
			strerror(errno));
		return DC_ERR_SYSTEM;
	}
	fprintf(f, "# This file is generated by the master-side DC-API\n");
	fprintf(f, "%s = %s\n", CFG_LOGFILE, DC_LABEL_CLIENTLOG);

	_DC_initClientConfig(wu->client_name, f);
	fclose(f);

	wu->input_files = g_list_append(wu->input_files, file);
	wu->num_inputs++;
	return 0;
}

DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag)
{
	DC_Workunit *wu;
	int ret;

	if (!wu_table)
	{
		DC_log(LOG_ERR, "%s: Library is not initialized", __func__);
		return NULL;
	}
	if (!clientName)
	{
		DC_log(LOG_ERR, "%s: Missing client name", __func__);
		return NULL;
	}

	wu = alloc_wu();
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

	/* Set the default priority */
	wu->priority = DC_getClientCfgInt(clientName, CFG_DEFAULTPRIO, 0, TRUE);

	/* Set the default batch value */
	wu->batch = 0;

	/* Add the internal output files */
	if (DC_getClientCfgBool(clientName, CFG_NATIVECLIENT, FALSE, TRUE))
	{
		wu->nativeclient = TRUE;
		wu->nosuspend = TRUE;
	}
	else
	{
		/* Add the client config as an internal input file */
		ret = generate_client_config(wu);
		if (ret)
		{
			DC_destroyWU(wu);
			return NULL;
		}

		DC_addWUOutputExt(wu, DC_LABEL_STDOUT, DC_OUTFILE_OPTIONAL);
		DC_addWUOutputExt(wu, DC_LABEL_STDERR, DC_OUTFILE_OPTIONAL);
		DC_addWUOutputExt(wu, DC_LABEL_CLIENTLOG, DC_OUTFILE_OPTIONAL);
		if (DC_getClientCfgBool(clientName, CFG_ENABLESUSPEND, FALSE, TRUE))
			DC_addWUOutput(wu, CKPT_LABEL_OUT);
		else
			wu->nosuspend = TRUE;
	}

	g_hash_table_insert(wu_table, wu->uuid, wu);
	++num_wus;

	return wu;
}

void DC_destroyWU(DC_Workunit *wu)
{
	if (!wu || --wu->refcnt > 0)
		return;

	g_hash_table_remove(wu_table, wu->uuid);
	--num_wus;

	switch (wu->state)
	{
		case DC_WU_RUNNING:
			DC_cancelWU(wu);
			break;
		default:
			break;
	}

	g_hash_table_destroy(wu->is_optional);

	while (wu->input_files)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)wu->input_files->data;

		unlink(file->path);
		wu->input_files = g_list_delete_link(wu->input_files,
			wu->input_files);
		_DC_destroyPhysicalFile(file);
	}

	while (wu->remote_input_files)
	{
		DC_RemoteFile *file = (DC_RemoteFile *)wu->remote_input_files->data;

		wu->remote_input_files = g_list_delete_link(wu->remote_input_files,
			wu->remote_input_files);
		_DC_destroyRemoteFile(file);
	}

	while (wu->output_files)
	{
		g_free(wu->output_files->data);
		wu->output_files = g_list_delete_link(wu->output_files,
			wu->output_files);
	}

	if (wu->ckpt_name)
	{
		char *path = _DC_workDirPath(wu, wu->ckpt_name, FILE_CKPT);
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
		while (dir && (name = g_dir_read_name(dir)))
		{
			GString *str = g_string_new(wu->workdir);
			g_string_append_c(str, G_DIR_SEPARATOR);
			g_string_append(str, name);
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
	if (wu->argv)
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
	for (l = wu->remote_input_files; l; l = l->next)
	{
		DC_RemoteFile *file = (DC_RemoteFile *)l->data;
		if (!strcmp(file->label, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as a "
				"remote input file", logicalFileName);
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
        DC_FileMode fileMode, ...)
{
	int ret;
	va_list ap;
	char *workpath;
	DC_PhysicalFile *file = NULL;
	DC_RemoteFile *rfile = NULL;
    int persistent_on_client = 0;

    if (fileMode & DC_FILE_PERSISTENT_CLIENT) 
    {
        persistent_on_client = 1;
        fileMode ^= DC_FILE_PERSISTENT_CLIENT;
    }
	/* Sanity checks */
	if (!wu || !logicalFileName)
	{
		DC_log(LOG_ERR, "%s: Missing arguments", __func__);
		return DC_ERR_BADPARAM;
	}
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	/* Handle remote http:// files */
	if (fileMode == DC_FILE_REMOTE && !strncmp("http://", URL, 7))
	{
		va_start(ap, fileMode);
		char *md5 = va_arg(ap, char *);
		size_t size = va_arg(ap, size_t);
		va_end(ap);

		rfile = _DC_createRemoteFile(logicalFileName, URL, md5, size);
		if (!rfile)
			return DC_ERR_INTERNAL;

        if (persistent_on_client) 
        {
            rfile->persistentonclient = 1;
        }

		wu->remote_input_files = g_list_append(wu->remote_input_files, rfile);
		wu->num_remote_inputs++;

		if (wu->serialized)
		write_wudesc(wu);

		return 0;
	}

	/* Now handle local and attic files */
	workpath = _DC_workDirPath(wu, logicalFileName, FILE_IN);
	file = _DC_createPhysicalFile(logicalFileName, workpath);
	g_free(workpath);
	if (!file)
		return DC_ERR_INTERNAL;

	/* Handle remote attic:// files */
	if (fileMode == DC_FILE_REMOTE && !strncmp("attic://", URL, 8))
	{
		va_start(ap, fileMode);
		char *md5 = va_arg(ap, char *);
		size_t size = va_arg(ap, size_t);
		va_end(ap);

		// If regular expression for the Attic file gives valid substitutions,
		// use substitued URLs and keep the file remote
		gchar **repl = replace_regex(URL);
		if (repl)
		{
			g_strfreev(repl);
			_DC_destroyPhysicalFile(file);

			rfile = _DC_createRemoteFile(logicalFileName, URL, md5, size);
			if (!rfile)
				return DC_ERR_INTERNAL;

            if (persistent_on_client) 
            {
                rfile->persistentonclient = 1;
            }

			wu->remote_input_files = g_list_append(wu->remote_input_files, rfile);
			wu->num_remote_inputs++;

			if (wu->serialized)
			write_wudesc(wu);

			return 0;
		}

		char *physicalFileName = g_strrstr(URL,"/");
		if (!physicalFileName)
		{
			DC_log(LOG_ERR, "URL of attic file has wrong format! URL: '%s'", URL);
			return DC_ERR_BADPARAM;
		}
		physicalFileName++;
		physicalFileName = g_strdup(physicalFileName);

		char *physicalFileHashString = g_strdup_printf("%s %i\n", md5, (int)size);

		DC_log(LOG_DEBUG, "Attic file details: logic: %s, url:%s, md5:%s, size:%i, file:%s, filecont:%s",
			logicalFileName, URL, md5, (int)size, physicalFileName, physicalFileHashString);

		file->physicalfilename = strdup(physicalFileName);
		g_free(physicalFileName);

		if (!physicalFileHashString)
		{
			DC_log(LOG_ERR, "%s: Hash of attic file "
				"'%s' not found", __func__, logicalFileName);
			return DC_ERR_BADPARAM;
		}
		file->physicalfilehash = strdup(physicalFileHashString);
		g_free(physicalFileHashString);
	}
	else if (fileMode == DC_FILE_REMOTE)
	{
		DC_log(LOG_ERR, "%s: Unsupported URL received: '%s'",
			__func__, URL);
		_DC_destroyPhysicalFile(file);
		return DC_ERR_BADPARAM;
	}

    if (persistent_on_client) 
    {
        file->persistentonclient = 1;
    }

	switch (fileMode)
	{
		case DC_FILE_PERSISTENT:
			ret = link(URL, file->path);
			if (!ret)
			{
				/* Remember the file mode */
				file->mode = DC_FILE_PERSISTENT;
				break;
			}

			DC_log(LOG_DEBUG, "Hardlinking failed for input file %s: %s; "
				"falling back to copy", URL, strerror(errno));
			/* Fall through */
		case DC_FILE_REGULAR:
			ret = _DC_copyFile(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to copy the input file %s to %s",
					URL, file->path);
				_DC_destroyPhysicalFile(file);
				return ret;
			}
			break;
		case DC_FILE_VOLATILE:
			ret = rename(URL, file->path);
			if (!ret)
				break;
			DC_log(LOG_DEBUG, "Renaming failed for input file %s: %s; "
				"falling back to copy/delete", URL, strerror(errno));
			ret = _DC_copyFile(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to copy the input file %s to %s",
					URL, file->path);
				_DC_destroyPhysicalFile(file);
				return ret;
			}
			unlink(URL);
			break;
		case DC_FILE_REMOTE:
			break;
		default:
			DC_log(LOG_ERR, "Invalid file mode %d", fileMode);
			return DC_ERR_BADPARAM;
	}

	file->mode = fileMode;
	wu->input_files = g_list_append(wu->input_files, file);
	wu->num_inputs++;

	if (wu->serialized)
		write_wudesc(wu);

	return 0;
}

int DC_addWUOutputExt(DC_Workunit *wu, const char *logicalFileName, int flags)
{
	int ret;

	/* Sanity checks */
	if (!wu || !logicalFileName)
	{
		DC_log(LOG_ERR, "%s: Missing arguments", __func__);
		return DC_ERR_BADPARAM;
	}
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	if (flags & DC_OUTFILE_OPTIONAL)
		g_hash_table_replace(wu->is_optional,
				     g_strdup(logicalFileName),
				     (void*)1);

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	wu->output_files = g_list_append(wu->output_files,
		g_strdup(logicalFileName));
	wu->num_outputs++;

	if (wu->serialized)
		write_wudesc(wu);

	return 0;
}
int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName)
{
	return DC_addWUOutputExt(wu, logicalFileName, 0);
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
		dest = get_input_download_path(wu, file->label, file->physicalfilename);
		/* By creating the hard link ourselves we prevent BOINC from
		 * copying the file later */
		ret = link(file->path, dest);
		if (ret)
		{
			if (file->physicalfilename)
			{
				if (errno == EEXIST)
				{
					DC_log(LOG_NOTICE, "File %s already exists under Boinc at %s. Skipping...",file->physicalfilename,dest);
				}
				else if (errno == ENOENT)
				{
					FILE *f;
					f = fopen(dest, "w");
					if (!f)
					{
						DC_log(LOG_ERR, "Cannot create file %s: %s",dest,strerror(errno));
						g_free(dest);
						return DC_ERR_INTERNAL;
					}
					fprintf(f, "This file has been generated by DC-API.\nOriginal location of this file:%s", dest);
					fclose(f);
					DC_log(LOG_DEBUG, "File %s has been created by DC-API under Boinc.",dest);
				}
				else
				{
					DC_log(LOG_ERR, "---Failed to install file %s to the "
						"Boinc download directory at %s: %s",
						file->path, dest, strerror(errno));
					g_free(dest);
					return DC_ERR_INTERNAL;
				}
			}
			else
			{
				DC_log(LOG_ERR, "Failed to install file %s to the "
					"Boinc download directory at %s: %s",
					file->path, dest, strerror(errno));
				g_free(dest);
				return DC_ERR_INTERNAL;
			}
		}
		if (file->physicalfilehash)
		{
			const char *hashFileExt=".md5";
			GString *hashFile;
			FILE *f;

			hashFile = g_string_new(dest);
			g_string_append(hashFile, hashFileExt);

			f = fopen(hashFile->str, "w");
			if (!f)
			{
				if (errno == EEXIST && file->physicalfilename)
				{
					DC_log(LOG_NOTICE, "File %s already exists under Boinc. Skipping...",hashFile->str);
				}
				else
				{
					DC_log(LOG_ERR, "Failed to create hash file %s: %s", hashFile->str,strerror(errno));
					g_string_free(hashFile, TRUE);
					g_free(dest);
					return DC_ERR_INTERNAL;
				}
			}
			else
			{
				fprintf(f, "%s", file->physicalfilehash);
				fclose(f);
				DC_log(LOG_DEBUG, "Hash file \"%s\" has been created with content \"%s\".", hashFile->str, file->physicalfilehash);
			}

			g_string_free(hashFile, TRUE);
		}
		g_free(dest);
	}

	/* During resume, we have to install the checkpoint file as well */
	if (wu->ckpt_name)
	{
		char *src;

		src = _DC_workDirPath(wu, wu->ckpt_name, FILE_CKPT);
		dest = get_input_download_path(wu, wu->ckpt_name, NULL);
		ret = link(src, dest);
		if (ret)
		{
			DC_log(LOG_ERR, "Failed to install file %s to the "
				"Boinc download directory at %s: %s",
				src, dest, strerror(errno));
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
    int min_quorum = 0;
	memset(params, 0, sizeof(*params));

	params->rsc_fpops_est = DC_getClientCfgDouble(wu->client_name,
		CFG_FPOPS_EST, 1e13, TRUE);
	params->rsc_fpops_bound = DC_getClientCfgDouble(wu->client_name,
		CFG_MAXFPOPS, 1e15, TRUE);
	params->rsc_memory_bound = DC_getClientCfgDouble(wu->client_name,
		CFG_MAXMEMUSAGE, 128 << 20, TRUE);
	params->rsc_disk_bound = DC_getClientCfgDouble(wu->client_name,
		CFG_MAXDISKUSAGE, 64 << 20, TRUE);
	params->delay_bound = DC_getClientCfgInt(wu->client_name,
		CFG_DELAYBOUND, 4 * 7 * 24 * 60 * 60, TRUE);

	min_quorum = DC_getClientCfgInt(wu->client_name,
		CFG_MIN_QUORUM, -1, FALSE);
		
    if (min_quorum != -1) 
    {
    	params->min_quorum = min_quorum;
    	if (params->min_quorum < 1)
    		params->min_quorum = 1;
    		
    	params->target_nresults = DC_getClientCfgInt(wu->client_name,
    		CFG_TARGET_NRESULTS, 1, FALSE);
    	if (params->target_nresults < params->min_quorum)
            params->target_nresults = params->min_quorum;

    	params->max_error_results = DC_getClientCfgInt(wu->client_name,
    		CFG_MAX_ERROR_RESULTS, 0, FALSE);
    	if (params->max_error_results < 0)
    		params->max_error_results = 0;

    	params->max_total_results = DC_getClientCfgInt(wu->client_name,
    		CFG_MAX_TOTAL_RESULTS, 1, FALSE);
    	if (params->max_total_results < params->min_quorum)
            params->max_total_results = params->min_quorum;
            
    	params->max_success_results = DC_getClientCfgInt(wu->client_name,
    		CFG_MAX_SUCCESS_RESULTS, 1, FALSE);
    	if (params->max_success_results < params->min_quorum)
            params->max_success_results = params->min_quorum;
    } else {
    	params->min_quorum = DC_getClientCfgInt(wu->client_name,
    		CFG_REDUNDANCY, 1, TRUE);
    	if (params->min_quorum < 1)
    		params->min_quorum = 1;

    	params->target_nresults = params->min_quorum;

    	/* Calculate with logarithmic error */
    	params->max_error_results = params->target_nresults +
    		(int)log(params->min_quorum + 2) + 1;
    	params->max_total_results = (params->target_nresults +
    		(int)log(params->min_quorum + 2)) * 2;
    	params->max_success_results = params->target_nresults +
    		(int)log(params->min_quorum + 2) + 1;
    }
}

static void append_wu_file_info(GString *tmpl, int idx, DC_PhysicalFile *file)
{
	g_string_append(tmpl, "<file_info>\n");
	g_string_append_printf(tmpl, "\t<number>%d</number>\n", idx);
    if (file != NULL && file->persistentonclient == 1) 
    {
        g_string_append(tmpl, "\t<sticky/>\n");
        g_string_append(tmpl, "\t<no_delete/>\n");
    }
	g_string_append(tmpl, "</file_info>\n");
}

static void append_wu_remote_file_info(GString *tmpl, int idx, DC_RemoteFile *file)
{
	g_string_append(tmpl, "<file_info>\n");
	g_string_append_printf(tmpl, "\t<number>%d</number>\n", idx);
    if (file != NULL && file->persistentonclient == 1) 
    {
        g_string_append(tmpl, "\t<sticky/>\n");
        g_string_append(tmpl, "\t<no_delete/>\n");
    }
	gchar **alts = replace_regex(file->url);
	if (!alts)
	{
		g_string_append_printf(tmpl, "\t<url>%s</url>\n", file->url);
	}
	else
	{
		int i;
		for (i = 0; alts[i]; i++)
			g_string_append_printf(tmpl, "\t<url>%s</url>\n", alts[i]);
		g_strfreev(alts);
	}
	g_string_append_printf(tmpl, "\t<md5_cksum>%s</md5_cksum>\n", file->remotefilehash);
	g_string_append_printf(tmpl, "\t<nbytes>%lu</nbytes>\n", file->remotefilesize);
	g_string_append(tmpl, "</file_info>\n");
}

static void append_wu_file_ref(GString *tmpl, int idx, const char *label)
{
	g_string_append(tmpl, "\t<file_ref>\n");
	g_string_append_printf(tmpl, "\t\t<file_number>%d</file_number>\n", idx);
	g_string_append_printf(tmpl, "\t\t<open_name>%s</open_name>\n", label);
	g_string_append(tmpl, "\t</file_ref>\n");
}

static char *generate_wu_template(DC_Workunit *wu)
{
	struct wu_params params;
	GString *tmpl, *cmd;
	int i, num_inputs, num_remote_inputs;
	GList *l;
	char *p;

	fill_wu_params(wu, &params);

	/* Generate the file info block */
	num_inputs = wu->num_inputs;
	num_remote_inputs = wu->num_remote_inputs;
	if (wu->ckpt_name)
		num_inputs++;

	tmpl = g_string_new("");
	for (i = 0, l = wu->input_files; l && i < num_inputs; l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;
        append_wu_file_info(tmpl, i, file);
    }
    
	for (l = wu->remote_input_files; l && i < num_inputs + num_remote_inputs;
			l = l->next, i++)
	{
		DC_RemoteFile *file = (DC_RemoteFile *)l->data;
		append_wu_remote_file_info(tmpl, i, file);
	}
	/* Checkpoint file, if exists */
	if (wu->ckpt_name)
		append_wu_file_info(tmpl, i++, NULL);

	/* Generate the workunit description */
	g_string_append(tmpl, "<workunit>\n");
	for (i = 0, l = wu->input_files; l && i < num_inputs;
			l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;
		append_wu_file_ref(tmpl, i, file->label);
	}

	for (l = wu->remote_input_files; l && i < num_inputs + num_remote_inputs;
			l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;
		append_wu_file_ref(tmpl, i, file->label);
	}

	/* Checkpoint file */
	if (wu->ckpt_name)
		append_wu_file_ref(tmpl, i++, CKPT_LABEL_IN);

	/* Concatenate the shell-quoted argv elements into a single string */
	cmd = g_string_new("");
	for (i = 0; i < wu->argc; i++)
	{
		if (i)
			g_string_append_c(cmd, ' ');
		g_string_append(cmd, wu->argv[i]);
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

	g_string_append(tmpl, "</workunit>\n");

	p = tmpl->str;
	g_string_free(tmpl, FALSE);
	return p;
}

static void append_result_file_info(GString *tmpl, int idx, int auto_upload,
	double max_output)
{
	g_string_append(tmpl, "<file_info>\n");
	g_string_append_printf(tmpl, "\t<name><OUTFILE_%d/></name>\n", idx);
	g_string_append(tmpl, "\t<generated_locally/>\n");
	if (auto_upload)
		g_string_append(tmpl, "\t<upload_when_present/>\n");
	g_string_append_printf(tmpl, "\t<max_nbytes>%g</max_nbytes>\n", max_output);

	gchar *uploadURL = DC_getCfgStr(CFG_UPLOADURL);
	if (!uploadURL)
		g_string_append(tmpl, "\t<url><UPLOAD_URL/></url>\n");
	else
	{
		g_string_append_printf(tmpl, "\t<url>%s</url>\n", uploadURL);
		free(uploadURL);
	}

	g_string_append(tmpl, "</file_info>\n");
}

static void append_result_file_ref(GString *tmpl, int idx, int optional, const char *fmt, ...)
	G_GNUC_PRINTF(4, 5);
static void append_result_file_ref(GString *tmpl, int idx, int optional, const char *fmt, ...)
{
	char buf[1024];
	va_list ap;

	g_string_append(tmpl, "\t<file_ref>\n");
	g_string_append_printf(tmpl, "\t\t<file_name><OUTFILE_%d/></file_name>\n", idx);
	g_string_append(tmpl, "\t\t<open_name>");

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	g_string_append(tmpl, buf);

	g_string_append(tmpl, "</open_name>\n");
	if (optional)
		g_string_append(tmpl, "<optional>1</optional>");
	g_string_append(tmpl, "\t</file_ref>\n");
}

static char *generate_result_template(DC_Workunit *wu)
{
	unsigned char digest[SHA_DIGEST_LENGTH];
	int i, file_cnt;
    // support output files larger than 2GB
    double max_output_size;
	GString *path, *tmpl;
	char *file, *cfgval;
	SHA_CTX sha;
	GList *l;

	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (!cfgval)
	{
		/* Should never happen here */
		DC_log(LOG_ERR, "Internal error: project root is not set");
		return NULL;
	}

	max_output_size = DC_getClientCfgDouble(wu->client_name, CFG_MAXOUTPUT,
		256 * 1024, TRUE);
	file_cnt = 0;

	tmpl = g_string_new("");

	/* Slots for output files */
	for (i = 0; i < wu->num_outputs; i++)
		append_result_file_info(tmpl, file_cnt++, TRUE, max_output_size);
	/* Slots for subresults - no automatic uploading */
	for (i = 0; i < wu->subresults; i++)
		append_result_file_info(tmpl, file_cnt++, FALSE, max_output_size);

	g_string_append(tmpl, "<result>\n");
	file_cnt = 0;
	/* The output templates */
	for (l = wu->output_files; l; l = l->next)
	{
		char *lname = (char*)l->data;
	        int optional = 0 != g_hash_table_lookup(wu->is_optional, lname);							
		append_result_file_ref(tmpl, file_cnt++, optional, "%s", lname);
	}

	/* The subresult templates */
	for (i = 0; i < wu->subresults; i++)
		append_result_file_ref(tmpl, file_cnt++, 0, "%s%d", SUBRESULT_PFX, i);

	g_string_append(tmpl, "</result>\n");

	/* Use the hash of the template's contents as the file name,
	 * so we can avoid generating lots of files with the same
	 * content */
	SHA1_Init(&sha);
	SHA1_Update(&sha, tmpl->str, strlen(tmpl->str));
	SHA1_Final(digest, &sha);

	path = g_string_new(cfgval);
	g_string_append_printf(path, "%ctemplates%cdcapi_",
		G_DIR_SEPARATOR, G_DIR_SEPARATOR);
	for (i = 0; i < SHA_DIGEST_LENGTH; i += 2)
		g_string_append_printf(path, "%02x%02x", digest[i], digest[i + 1]);
	g_string_append(path, ".xml");

	if (access(path->str, R_OK))
	{
		FILE *f = fopen(path->str, "w");
		if (!f)
		{
			DC_log(LOG_ERR, "Failed to create result template %s: %s",
				path->str, strerror(errno));
			g_string_free(tmpl, TRUE);
			g_string_free(path, TRUE);
			free(cfgval);
			return NULL;
		}

		fprintf(f, "%s", tmpl->str);
		fclose(f);
	}

	file = g_strdup(path->str + strlen(cfgval) + 1);
	free(cfgval);
	g_string_free(path, TRUE);
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

char *_DC_getWUName(const DC_Workunit *wu)
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

	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}

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
	dbwu.rsc_fpops_est = params.rsc_fpops_est;
	dbwu.rsc_fpops_bound = params.rsc_fpops_bound;
	dbwu.rsc_memory_bound = params.rsc_memory_bound;
	dbwu.rsc_disk_bound = params.rsc_disk_bound;
	dbwu.delay_bound = params.delay_bound;

	dbwu.priority = wu->priority;
	dbwu.batch = wu->batch;

	wu_template = generate_wu_template(wu);
	result_template_file = generate_result_template(wu);
	if (!result_template_file)
	{
		g_free(wu_template);
		return DC_ERR_INTERNAL;
	}
	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (!cfgval)
	{
		g_free(wu_template);
		g_free(result_template_file);
		errno = ENOMEM;
		return DC_ERR_SYSTEM;
	}
	result_path = g_strdup_printf("%s%c%s", cfgval, G_DIR_SEPARATOR,
		result_template_file);
	free(cfgval);

	/* Create the input file name array as required by create_work() */
	ninputs = wu->num_inputs + wu->num_remote_inputs;
	if (wu->ckpt_name)
		ninputs++;

	infiles = g_new0(char *, ninputs + 1);
	for (l = wu->input_files, i = 0; l && i < wu->num_inputs;
			l = l->next, i++)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;
		infiles[i] = get_input_download_name(wu, file->label, file->physicalfilename);
	}
	for (l = wu->remote_input_files; l && i < wu->num_remote_inputs+wu->num_inputs;
			l = l->next, i++)
	{
		DC_RemoteFile *file = (DC_RemoteFile *)l->data;
		infiles[i] = g_strdup_printf("%s_%s", file->label, md5_string(file->url).c_str());
	}
	if (wu->ckpt_name)
		infiles[i++] = get_input_download_name(wu, wu->ckpt_name, NULL);
	/* Terminator so we can use g_strfreev() later */
	infiles[i] = NULL;

	ret = create_work(dbwu, wu_template, result_template_file, result_path,
		const_cast<const char **>(infiles), ninputs, dc_boinc_config);
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

	wu->submitted = TRUE;

	_DC_getDBid(wu);
	
	write_wudesc(wu);

	return 0;
}

char *DC_serializeWU(DC_Workunit *wu)
{
	char id[2 * 36 + 2];

	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return NULL;
	}

	if (write_wudesc(wu))
		return NULL;

	wu->serialized = TRUE;

	strncpy(id, project_uuid_str, sizeof(id));
	id[36] = '_';
	uuid_unparse_lower(wu->uuid, id + 37);
	return strdup(id);
}

DC_Workunit *DC_deserializeWU(const char *buf)
{
	if (!wu_table)
	{
		DC_log(LOG_ERR, "%s: Library is not initialized", __func__);
		return NULL;
	}
	if (!buf)
	{
		DC_log(LOG_ERR, "%s: No serialized data", __func__);
		return NULL;
	}

	return _DC_getWUByName(buf);
}

static DC_Workunit *load_from_boinc_db(const uuid_t uuid)
{
	char uuid_str[37], *query, *result_template, *cfgval, *filename;
	GList *output_files;
	DB_WORKUNIT db_wu;
	DC_Workunit *wu;
	DB_APP app;
	int ret;

	uuid_unparse_lower(uuid, uuid_str);
	query = g_strdup_printf("WHERE name LIKE '%s\\_%s%%'", project_uuid_str,
		uuid_str);
	ret = db_wu.lookup(query);
	g_free(query);
	if (ret)
	{
		DC_log(LOG_WARNING, "Could not load WU %s_%s from the BOINC "
			"database", project_uuid_str, uuid_str);
		return NULL;
	}

	ret = app.lookup_id(db_wu.appid);
	if (ret)
	{
		DC_log(LOG_WARNING, "Could not find application ID %d",
			db_wu.appid);
		return NULL;
	}

	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (!cfgval)
	{
		/* Should never happen here */
		DC_log(LOG_ERR, "Internal error: project root is not set");
		return NULL;
	}

	filename = g_strdup_printf("%s%c%s", cfgval, G_DIR_SEPARATOR,
		db_wu.result_template_file);
	free(cfgval);
	ret = g_file_get_contents(filename, &result_template, NULL, NULL);
	if (!ret)
	{
		DC_log(LOG_ERR, "Result template file %s is missing", filename);
		g_free(filename);
		return NULL;
	}
	g_free(filename);

	wu = alloc_wu();
	wu->client_name = g_strdup(app.name);
	memcpy(wu->uuid, uuid, sizeof(wu->uuid));

	/* If the WU is in the database then it was submitted... */
	wu->submitted = TRUE;

	if (strlen(db_wu.name) > 74)
		wu->tag = g_strdup(db_wu.name + 74);

	/* Always create the WU's work dir */
	wu->workdir = get_workdir(wu->uuid, TRUE);
	if (!wu->workdir)
	{
		DC_destroyWU(wu);
		g_free(result_template);
		return NULL;
	}

	output_files = _DC_parseFileRefs(result_template, NULL);
	g_free(result_template);
	if (!output_files)
	{
		DC_log(LOG_ERR, "Failed to parse the output files for WU %s_%s",
			project_uuid_str, uuid_str);
		DC_destroyWU(wu);
		return NULL;
	}

	while (output_files)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)output_files->data;

		DC_addWUOutput(wu, file->label);
		_DC_destroyPhysicalFile(file);
		output_files = g_list_delete_link(output_files, output_files);
	}

	/* We may no longer have the input files, this WU can not be suspended */
	wu->nosuspend = TRUE;

	g_hash_table_insert(wu_table, wu->uuid, wu);
	++num_wus;

	_DC_updateWUState(wu);

	return wu;
}

static DC_Workunit *load_from_disk(const uuid_t uuid)
{
	GMarkupParseContext *ctx;
	struct parser_state pctx;
	char *workdir, buf[256];
	DC_Workunit *wu;
	GError *error = NULL;
	FILE *f;

	workdir = get_workdir(uuid, FALSE);
	if (!g_file_test(workdir, G_FILE_TEST_IS_DIR))
	{
		g_free(workdir);
		return NULL;
	}

	wu = alloc_wu();
	wu->workdir = workdir;
	memcpy(wu->uuid, uuid, sizeof(wu->uuid));

	f = open_workdir_file(wu, WU_DESC_FILE, FILE_DCAPI, "r");
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

	g_hash_table_insert(wu_table, wu->uuid, wu);
	++num_wus;

	_DC_updateWUState(wu);

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
		DC_log(LOG_ERR, "WU name '%s' contains illegal UUID", name);
		return NULL;
	}

	if (uuid_compare(uuid, project_uuid))
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

	wu = (DC_Workunit *)g_hash_table_lookup(wu_table, uuid);
	if (wu)
		return wu;
	wu = load_from_disk(uuid);
	if (wu)
		return wu;
	return load_from_boinc_db(uuid);
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
			g_hash_table_foreach(wu_table, count_ready, &val);
			return val;
		case DC_WU_RUNNING:
			query = g_strdup_printf("SELECT COUNT(DISTINCT wu.id) "
				"FROM result res FORCE INDEX(ind_res_st), workunit wu "
				"WHERE res.workunitid = wu.id AND "
					"wu.name LIKE '%s\\_%%' AND "
					"(res.server_state = %d OR res.server_state = %d)",
				project_uuid_str, RESULT_SERVER_STATE_UNSENT,
				RESULT_SERVER_STATE_IN_PROGRESS);
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
		case DC_WU_ABORTED:
			query = g_strdup_printf("SELECT COUNT(*) "
				"FROM workunit wu WHERE "
				"wu.name LIKE '%s\\_%%' AND "
				"wu.error_mask != 0",
				project_uuid_str);
			ret = db.get_integer(query, val);
			g_free(query);
			if (ret)
				return -1;
			return val;
		case DC_WU_SUSPENDED:
			/* XXX */
		case DC_WU_UNKNOWN:
		default:
			return -1;
	}
}

char *DC_getWUTag(const DC_Workunit *wu)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return NULL;
	}

	return strdup(wu->tag);
}

char *DC_getWUId(const DC_Workunit *wu)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return NULL;
	}
	
	if (wu->db_id)
		return g_strdup_printf("%i",wu->db_id);
	else
		return NULL;
}

int DC_setWUPriority(DC_Workunit *wu, int priority)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}

	wu->priority = priority;
	if (wu->serialized)
		write_wudesc(wu);
	return 0;
}

int DC_setWUBatch(DC_Workunit *wu, int batch)
{
        if (!wu)
        {
                DC_log(LOG_ERR, "%s: Missing WU", __func__);
                return DC_ERR_BADPARAM;
        }

        wu->batch = batch;
        if (wu->serialized)
                write_wudesc(wu);
        return 0;
}


DC_WUState DC_getWUState(DC_Workunit *wu)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_WU_UNKNOWN;
	}

	return wu->state;
}

int DC_suspendWU(DC_Workunit *wu)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}

	if (wu->nosuspend)
	{
		char *name = _DC_getWUName(wu);
		DC_log(LOG_ERR, "Work unit %s can not be suspended", name);
		g_free(name);
		return DC_ERR_BADPARAM;
	}

	return DC_ERR_NOTIMPL;
}

int DC_resumeWU(DC_Workunit *wu)
{
	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}
	if (!wu->suspended)
	{
		char *name = _DC_getWUName(wu);
		DC_log(LOG_ERR, "Work unit %s is not suspended, cannot resume", name);
		g_free(name);
		return DC_ERR_BADPARAM;
	}

	return DC_ERR_NOTIMPL;
}

int _DC_initWUs(void)
{
	wu_table = g_hash_table_new_full(wu_uuid_hash, wu_uuid_equal, NULL, NULL);
	return 0;
}
