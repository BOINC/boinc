#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dc_local.h"

/********************************************************************
 * Functions
 */

DC_Result *_DC_createResult(const char *wu_name)
{
	DC_Result *result;

	result = g_new0(DC_Result, 1);
	result->wu = _DC_getWUByName(wu_name);
	if (!result->wu)
	{
		DC_log(LOG_ERR, "Received result for unknown WU %s", wu_name);
		g_free(result);
		return NULL;
	}

	// XXX Finish the result correctly

	return result;
}

void _DC_destroyResult(DC_Result *result)
{
	while (result->output_files)
	{
//		_DC_destroyPhysicalFile(result->output_files->data);
		result->output_files = g_list_delete_link(result->output_files,
			result->output_files);
	}
	g_free(result);
}
