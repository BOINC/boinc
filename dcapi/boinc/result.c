#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
typedef struct _result_ctx	result_ctx;
struct _result_ctx
{
	element_type		curr_element;
	char			*label;
	char			*path;
	DC_Result		*result;
};

/********************************************************************
 * Prototypes
 */

/* XML parser callback functions */
static void result_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error);
static void result_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error);
static void result_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error);

/********************************************************************
 * Global variables
 */

/* XML parser descriptor */
static const GMarkupParser result_parser =
{
	result_start,
	result_end,
	result_text,
	NULL,
	NULL
};

static void result_start(GMarkupParseContext *ctx, const char *element_name,
	const char **attr_names, const char **attr_values, void *ptr,
	GError **error)
{
	result_ctx *rctx = ptr;

	if (attr_names)
	{
		*error = g_error_new(G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
			"Attributes are not recognized in BOINC-generated XML");
		return;
	}

	if (!strcmp(element_name, "file_ref"))
	{
		g_free(rctx->label);
		g_free(rctx->path);
		rctx->label = NULL;
		rctx->path = NULL;
	}
	else if (!strcmp(element_name, "file_name"))
		rctx->curr_element = FILE_NAME;
	else if (!strcmp(element_name, "open_name"))
		rctx->curr_element = OPEN_NAME;
}

static void result_end(GMarkupParseContext *ctx, const char *element_name,
	void *ptr, GError **error)
{
	result_ctx *rctx = ptr;

	if (!strcmp(element_name, "file_ref"))
	{
		DC_PhysicalFile *file;

		file = _DC_createPhysicalFile(rctx->label, rctx->path);
		g_free(rctx->label);
		g_free(rctx->path);
		rctx->label = NULL;
		rctx->path = NULL;
		rctx->result->output_files =
			g_list_append(rctx->result->output_files, file);
		rctx->result->num_outputs++;
	}
	rctx->curr_element = UNKNOWN;
}

static void result_text(GMarkupParseContext *ctx, const char *text,
	gsize text_len, void *ptr, GError **error)
{
	result_ctx *rctx = ptr;

	switch (rctx->curr_element)
	{
		case FILE_NAME:
			rctx->path = g_strndup(text, text_len);
			break;
		case OPEN_NAME:
			rctx->label = g_strndup(text, text_len);
			break;
		default:
			break;
	}
}

DC_Result *_DC_createResult(const char *wu_name, const char *xml_doc_in)
{
	GMarkupParseContext *ctx;
	DC_Result *result;
	result_ctx rctx;
	GError *error;

	result = g_new0(DC_Result, 1);
	result->wu = _DC_getWUByName(wu_name);
	if (!result->wu)
	{
		DC_log(LOG_ERR, "Received result for unknown WU %s", wu_name);
		g_free(result);
		return NULL;
	}

	result->wu->state = DC_WU_FINISHED;

	memset(&rctx, 0, sizeof(rctx));
	rctx.result = result;
	
	ctx = g_markup_parse_context_new(&result_parser, 0, &rctx, NULL);
	if (!g_markup_parse_context_parse(ctx, xml_doc_in, strlen(xml_doc_in),
			&error))
		goto error;
	if (!g_markup_parse_context_end_parse(ctx, &error))
		goto error;
	g_markup_parse_context_free(ctx);

	return result;

error:
	DC_log(LOG_ERR, "Failed to parse result XML description: %s",
		error->message);
	g_error_free(error);
	g_markup_parse_context_free(ctx);
	g_free(rctx.label);
	g_free(rctx.path);
	_DC_destroyResult(result);
	return NULL;
}

void _DC_destroyResult(DC_Result *result)
{
	while (result->output_files)
	{
		_DC_destroyPhysicalFile(result->output_files->data);
		result->output_files = g_list_delete_link(result->output_files,
			result->output_files);
	}
	g_free(result);
}
