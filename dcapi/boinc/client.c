#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

/* BOINC includes */
#ifdef _WIN32
#include <boinc_win.h>
#include <util.h>
#endif
#include <boinc_api.h>
#include <filesys.h>
#include <diagnostics.h>
#include <graphics_api.h>

#include <dc_client.h>
#include "common_defs.h"

static int ckpt_generation;
static char *last_complete_ckpt;
static char active_ckpt[PATH_MAX];

int DC_init(void)
{
	char buf[PATH_MAX];

	if (boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR))
		return DC_ERR_INTERNAL;
	if (boinc_init())
		return DC_ERR_INTERNAL;

	if (!boinc_resolve_filename(CKPT_LABEL, buf, sizeof(buf)))
		last_complete_ckpt = strdup(buf);

	return 0;
}

char *DC_resolveFileName(DC_Filetype type, const char *logicalFileName)
{
	char buf[PATH_MAX];

	if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
	{
		if (type == DC_FILE_IN)
		{
			if (last_complete_ckpt)
				return strdup(last_complete_ckpt);
			return NULL;
		}
		else if (type == DC_FILE_OUT)
		{
			snprintf(active_ckpt, sizeof(active_ckpt), "dc_ckpt_%d",
				++ckpt_generation);
			return strdup(active_ckpt);
		}
		else
			return NULL;
	}

	/* Temporary files can be created in the current directory */
	if (type == DC_FILE_TMP)
		return strdup(logicalFileName);

	if (boinc_resolve_filename(logicalFileName, buf, sizeof(buf)))
		return NULL;
	return strdup(buf);
}

int DC_sendResult(const char *logicalFileName, const char *path,
	DC_FileMode fileMode)
{
	return DC_ERR_NOTIMPL;
}

DC_Event DC_checkEvent(void **data)
{
	if (boinc_time_to_checkpoint())
		return DC_EVENT_DO_CHECKPOINT;
	return DC_EVENT_NONE;
}

void DC_checkpointMade(const char *filename)
{
	boinc_checkpoint_completed();
	free(last_complete_ckpt);
	last_complete_ckpt = strdup(filename);
}

void DC_fractionDone(double fraction)
{
	boinc_fraction_done(fraction);
}

void DC_finish(int exitcode)
{
	boinc_finish(exitcode);
}

/********************************************************************
 * Provide empty implementation for some functions required by the
 * BOINC libraries
 */

void app_graphics_init(void)
{
}

void app_graphics_resize(int width, int height)
{
}

void app_graphics_render(int xs, int ys, double time_of_day)
{
}

void app_graphics_reread_prefs(void)
{
}

void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right)
{
}

void boinc_app_mouse_button(int x, int y, int which, bool is_down)
{
}

void boinc_app_key_press(int x, int y)
{
}

void boinc_app_key_release(int x, int y)
{
}
