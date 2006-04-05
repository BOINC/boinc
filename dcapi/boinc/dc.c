#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "dc_boinc.h"

/********************************************************************
 * Data type definitions
 */

/* Current element being parsed. We are not interested in most of the elements
 * and also assume that the XML document is always valid */
typedef enum
{
	UNKNOWN,
	FILE_NAME,
	OPEN_NAME
} element_type;

/* The private state of the XML parser */
typedef struct _fileref_ctx	fileref_ctx;
struct _fileref_ctx
{
	element_type		curr_element;
	char			*label;
	char			*path;
	GList			*file_list;
	int			num_files;
};


/********************************************************************
 * Prototypes
 */

/* XML parser callback functions */
static void fileref_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error);
static void fileref_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error);
static void fileref_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error);


/********************************************************************
 * Global variables
 */

DC_ResultCallback	_dc_resultcb;
DC_SubresultCallback	_dc_subresultcb;
DC_MessageCallback	_dc_messagecb;

uuid_t project_uuid;
char project_uuid_str[37];

/* XML parser descriptor */
static const GMarkupParser fileref_parser =
{
	fileref_start,
	fileref_end,
	fileref_text,
	NULL,
	NULL
};


/********************************************************************
 * API functions
 */

int DC_init(const char *config_file)
{
	const char *cfgval;
	int ret;

	if (!config_file)
		config_file = DC_CONFIG_FILE;

	ret = _DC_parseCfg(config_file);
	if (ret)
	{
		DC_log(LOG_ERR, "The DC-API cannot be initialized without a "
			"config file");
		return ret;
	}

	if (!_DC_getCfgStr(CFG_PROJECTROOT))
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_PROJECTROOT);
		return DC_ERR_CONFIG;
	}

	/* Check & switch to the working directory */
	cfgval = _DC_getCfgStr(CFG_WORKDIR);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_WORKDIR);
		return DC_ERR_CONFIG;
	}

	/* If we are started as a BOINC daemon, then the current dir is
	 * <projectroot>/bin, so we must change to the working directory */
	if (chdir(cfgval))
	{
		DC_log(LOG_ERR, "Failed to set the current directory to %s: %s",
			cfgval, strerror(errno));
		return DC_ERR_CONFIG;
	}

	/* Check the project UUID */
	cfgval = _DC_getCfgStr(CFG_INSTANCEUUID);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not set in the config file",
			CFG_INSTANCEUUID);
		return DC_ERR_CONFIG;
	}

	ret = uuid_parse((char *)cfgval, project_uuid);
	if (ret)
	{
		DC_log(LOG_ERR, "Invalid project UUID");
		return DC_ERR_CONFIG;
	}

	/* Enforce a canonical string representation of the UUID */
	uuid_unparse_lower(project_uuid, project_uuid_str);

	/* Check the project's configuration */
	cfgval = _DC_getCfgStr(CFG_CONFIGXML);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not set in the config file",
			CFG_CONFIGXML);
		return DC_ERR_CONFIG;
	}

	ret = access(cfgval, R_OK);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to access the project's configuration "
			"at %s: %s", cfgval, strerror(errno));
		return DC_ERR_CONFIG;
	}

	ret = _DC_parseConfigXML(cfgval);
	if (ret)
		return ret;

	/* Now connect to the Boinc database */
	ret = _DC_initDB();
	if (ret)
		return ret;

	return 0;
}

int DC_getMaxMessageSize(void)
{
	return 32768; /* XXX */
}

int DC_getMaxSubresults(void)
{
	return 100; /* XXX */
}

DC_GridCapabilities DC_getGridCapabilities(void)
{
	return DC_GC_EXITCODE | DC_GC_SUBRESULT | DC_GC_MESSAGING;
}

void DC_setcb(DC_ResultCallback resultcb, DC_SubresultCallback subresultcb,
	DC_MessageCallback msgcb)
{
	_dc_resultcb = resultcb;
	_dc_subresultcb = subresultcb;
	_dc_messagecb = msgcb;
}

DC_PhysicalFile *_DC_createPhysicalFile(const char *label,
	const char *path)
{
	DC_PhysicalFile *file;

	file = g_new(DC_PhysicalFile, 1);
	file->label = g_strdup(label);
	file->path = g_strdup(path);
	file->mode = DC_FILE_REGULAR;

	return file;
}

void _DC_destroyPhysicalFile(DC_PhysicalFile *file)
{
	if (!file)
		return;

	g_free(file->label);
	g_free(file->path);
	g_free(file);
}

/********************************************************************
 * XML file definition parser
 */

static void fileref_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error)
{
	fileref_ctx *fctx = ptr;

	if (attr_names)
	{
		*error = g_error_new(G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
			"Attributes are not recognized in BOINC-generated XML");
		return;
	}

	if (!strcmp(element_name, "file_ref"))
	{
		g_free(fctx->label);
		g_free(fctx->path);
		fctx->label = NULL;
		fctx->path = NULL;
	}
	else if (!strcmp(element_name, "file_name"))
		fctx->curr_element = FILE_NAME;
	else if (!strcmp(element_name, "open_name"))
		fctx->curr_element = OPEN_NAME;
}

static void fileref_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error)
{
	fileref_ctx *fctx = ptr;

	if (!strcmp(element_name, "file_ref"))
	{
		DC_PhysicalFile *file;

		file = _DC_createPhysicalFile(fctx->label, fctx->path);
		g_free(fctx->label);
		g_free(fctx->path);
		fctx->label = NULL;
		fctx->path = NULL;
		fctx->file_list = g_list_append(fctx->file_list, file);
		fctx->num_files++;
	}
	fctx->curr_element = UNKNOWN;
}

static void fileref_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error)
{
	fileref_ctx *fctx = ptr;

	switch (fctx->curr_element)
	{
		case FILE_NAME:
			fctx->path = g_strndup(text, text_len);
			break;
		case OPEN_NAME:
			fctx->label = g_strndup(text, text_len);
			break;
		default:
			break;
	}
}

GList *_DC_parseFileRefs(const char *xml_doc, int *num_files)
{
	GMarkupParseContext *ctx;
	fileref_ctx fctx;
	GError *error;

	memset(&fctx, 0, sizeof(fctx));

	ctx = g_markup_parse_context_new(&fileref_parser, 0, &fctx, NULL);
	if (!g_markup_parse_context_parse(ctx, xml_doc, strlen(xml_doc),
			&error))
		goto error;
	if (!g_markup_parse_context_end_parse(ctx, &error))
		goto error;
	g_markup_parse_context_free(ctx);
	if (num_files)
		*num_files = fctx.num_files;
	g_free(fctx.label);
	g_free(fctx.path);
	return fctx.file_list;

error:
	while (fctx.file_list)
	{
		_DC_destroyPhysicalFile(fctx.file_list->data);
		fctx.file_list = g_list_delete_link(fctx.file_list,
			fctx.file_list);
	}

	DC_log(LOG_ERR, "Failed to parse result XML description: %s",
		error->message);
	g_error_free(error);
	g_markup_parse_context_free(ctx);
	g_free(fctx.label);
	g_free(fctx.path);

	return NULL;
}
