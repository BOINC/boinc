#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* BOINC includes */
#ifdef _WIN32
#include <boinc_win.h>
#include <util.h>
#endif
#include <boinc_api.h>
#include <filesys.h>
#include <diagnostics.h>
#include <graphics_api.h>
#include <parse.h>

#include <dc_client.h>
#include "common_defs.h"

/********************************************************************
 * Global variables
 */

static int ckpt_generation;
static char *last_complete_ckpt;
static char active_ckpt[PATH_MAX];
static char wu_name[256];

/********************************************************************
 * Common API functions
 */

int DC_getMaxMessageSize(void)
{
	return MAX_MESSAGE_SIZE;
}

/* Determine the number of allowed subresults by counting all output files
 * that have a name starting with SUBRESULT_PFX */
int DC_getMaxSubresults(void)
{
	struct dirent *d;
	int nsubs;
	DIR *dir;

	dir = opendir(".");
	if (!dir)
		return 0;
	nsubs = 0;
	while ((d = readdir(dir)))
		if (!strncasecmp(d->d_name, SUBRESULT_PFX, strlen(SUBRESULT_PFX)))
			nsubs++;
	closedir(dir);
	return nsubs;
}

DC_GridCapabilities DC_getGridCapabilities(void)
{
	return (DC_GridCapabilities)(DC_GC_EXITCODE | DC_GC_STDERR |
		DC_GC_SUBRESULT | DC_GC_MESSAGING);
}

/********************************************************************
 * Client API functions
 */

int DC_init(void)
{
	char path[PATH_MAX], *buf;
	struct stat st;
	FILE *f;
	int ret;

	if (boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR))
		return DC_ERR_INTERNAL;
	if (boinc_init())
		return DC_ERR_INTERNAL;

	if (!boinc_resolve_filename(CKPT_LABEL_IN, path, sizeof(path)))
		last_complete_ckpt = strdup(path);

	ret = stat("init_data.xml", &st);
	if (ret)
		return DC_ERR_INTERNAL;

	f = boinc_fopen("init_data.xml", "r");
	if (!f)
		return DC_ERR_INTERNAL;

	buf = (char *)malloc(st.st_size + 1);
	fread(buf, st.st_size, 1, f);
	fclose(f);
	buf[st.st_size] = '\0';

	ret = parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name));
	free(buf);
	if (!ret)
		return DC_ERR_INTERNAL;

	return 0;
}

char *DC_resolveFileName(DC_FileType type, const char *logicalFileName)
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

int DC_sendMessage(const char *message)
{
	return boinc_send_trickle_up(wu_name, (char *)message);
}

static void handle_special_msg(const char *message)
{
	/* XXX Implement it */
}

static DC_Event *new_event(DC_EventType type)
{
	DC_Event *event;

	event = (DC_Event *)calloc(1, sizeof(DC_Event));
	event->type = type;
	return event;
}

DC_Event *DC_checkEvent(void)
{
	DC_Event *event;
	char *buf;

	/* Check for checkpoint requests */
	if (boinc_time_to_checkpoint())
		return new_event(DC_EVENT_DO_CHECKPOINT);

	/* Check for messages */
	buf = (char *)malloc(MAX_MESSAGE_SIZE);
	if (buf)
	{
		int ret;

		ret = boinc_receive_trickle_down(buf, MAX_MESSAGE_SIZE);
		if (ret)
		{
			if (!strncmp(buf, DCAPI_MSG_PFX, strlen(DCAPI_MSG_PFX)))
			{
				handle_special_msg(buf);
				free(buf);
				return NULL;
			}

			event = new_event(DC_EVENT_MESSAGE);
			event->message = buf;
			return event;
		}
		free(buf);
	}

	return NULL;
}

void DC_destroyEvent(DC_Event *event)
{
	if (!event)
		return;

	switch (event->type)
	{
		case DC_EVENT_MESSAGE:
			free(event->message);
			break;
		default:
			break;
	}
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
