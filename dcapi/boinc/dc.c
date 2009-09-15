/*
 * dc.c - Master side of the BOINC DC-API backend
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
	NAME,
	FILE_NAME,
	OPEN_NAME
} element_type;

typedef enum
{
	INSIDE_FILE_INFO	= (1 << 0),
	GENERATED_LOCALLY	= (1 << 1),
	UPLOAD_WHEN_PRESENT	= (1 << 2)
} element_flags;

/* The private state of the XML parser */
typedef struct _fileref_ctx	fileref_ctx;
struct _fileref_ctx
{
	element_type		element;
	element_flags		flags;
	char			*label;
	char			*path;
	GList			*file_list;
	int			num_files;
	GHashTable		*file_info;
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

int DC_initMaster(const char *config_file)
{
	char *cfgval;
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

	cfgval = DC_getCfgStr(CFG_PROJECTROOT);
	if (cfgval)
		setenv("BOINC_PROJECT_DIR", cfgval, 1);
	free(cfgval);

	/* Check & switch to the working directory */
	cfgval = DC_getCfgStr(CFG_WORKDIR);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not specified in the config file",
			CFG_WORKDIR);
		return DC_ERR_CONFIG;
	}

	if (*cfgval != G_DIR_SEPARATOR)
	{
		DC_log(LOG_ERR, "The working directory must be an "
			"absolute path");
		return DC_ERR_CONFIG;
	}

	/* If we are started as a BOINC daemon, then the current dir is
	 * <projectroot>/bin, so we must change to the working directory */
	if (chdir(cfgval))
	{
		DC_log(LOG_ERR, "Failed to set the current directory to %s: %s",
			cfgval, strerror(errno));
		free(cfgval);
		return DC_ERR_CONFIG;
	}
	free(cfgval);

	/* Check the project UUID */
	cfgval = DC_getCfgStr(CFG_INSTANCEUUID);
	if (!cfgval)
	{
		DC_log(LOG_ERR, "%s is not set in the config file",
			CFG_INSTANCEUUID);
		return DC_ERR_CONFIG;
	}

	ret = uuid_parse((char *)cfgval, project_uuid);
	if (ret)
	{
		DC_log(LOG_ERR, "Invalid project UUID %s", cfgval);
		free(cfgval);
		return DC_ERR_CONFIG;
	}
	free(cfgval);

	/* Enforce a canonical string representation of the UUID */
	uuid_unparse_lower(project_uuid, project_uuid_str);

	ret = _DC_parseConfigXML();
	if (ret)
		return ret;

	/* Now connect to the Boinc database */
	ret = _DC_initDB();
	if (ret)
		return ret;
	ret = _DC_initWUs();
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

unsigned DC_getGridCapabilities(void)
{
	return DC_GC_EXITCODE | DC_GC_SUBRESULT | DC_GC_MESSAGING |
		DC_GC_STDOUT | DC_GC_STDERR | DC_GC_LOG;
}

void DC_setMasterCb(DC_ResultCallback resultcb,
	DC_SubresultCallback subresultcb, DC_MessageCallback msgcb)
{
	_dc_resultcb = resultcb;
	_dc_subresultcb = subresultcb;
	_dc_messagecb = msgcb;
}

void DC_setResultCb(DC_ResultCallback cb)
{
	_dc_resultcb = cb;
}

void DC_setSubresultCb(DC_SubresultCallback cb)
{
	_dc_subresultcb = cb;
}

void DC_setMessageCb(DC_MessageCallback cb)
{
	_dc_messagecb = cb;
}


/********************************************************************
 * XML file definition parser
 */

static void fileref_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error)
{
	fileref_ctx *fctx = ptr;

	if (attr_names && attr_names[0])
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
		fctx->element = FILE_NAME;
	else if (!strcmp(element_name, "open_name"))
		fctx->element = OPEN_NAME;
	else if (!strcmp(element_name, "file_info"))
		fctx->flags |= INSIDE_FILE_INFO;
	else if (fctx->flags & INSIDE_FILE_INFO)
	{
		if (!strcmp(element_name, "name"))
			fctx->element = NAME;
		else if (!strcmp(element_name, "generated_locally"))
			fctx->flags |= GENERATED_LOCALLY;
		else if (!strcmp(element_name, "upload_when_present"))
			fctx->flags |= UPLOAD_WHEN_PRESENT;
	}
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
	else if (!strcmp(element_name, "file_info"))
	{
		fctx->flags = 0;

		g_hash_table_insert(fctx->file_info, fctx->path,
			GINT_TO_POINTER(fctx->flags));
		fctx->path = NULL;
	}

	fctx->element = UNKNOWN;
}

static void fileref_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error)
{
	fileref_ctx *fctx = ptr;

	switch (fctx->element)
	{
		case NAME:
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
	GError *error = NULL;

	memset(&fctx, 0, sizeof(fctx));
	fctx.file_info = g_hash_table_new_full(g_str_hash, g_str_equal,
		g_free, NULL);

	ctx = g_markup_parse_context_new(&fileref_parser, 0, &fctx, NULL);
	if (!g_markup_parse_context_parse(ctx, xml_doc, strlen(xml_doc),
			&error))
		goto error;
	if (!g_markup_parse_context_end_parse(ctx, &error))
		goto error;
	if (num_files)
		*num_files = fctx.num_files;

out:
	g_markup_parse_context_free(ctx);
	g_hash_table_destroy(fctx.file_info);
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
	goto out;
}
