/*
 * client.c - Client side of the BOINC DC-API backend
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
#include <dc_internal.h>

#include "common_defs.h"

/********************************************************************
 * Constants
 */

typedef enum {
	STATE_NORMAL,
	STATE_SUSPEND,
	STATE_FINISH
} client_event_state;

#define LAST_CKPT_FILE		"dc_lastckpt"


/********************************************************************
 * Global variables
 */

/* Checkpoint file generation counter */
static int ckpt_generation;
/* Name of the last completed checkpoint file */
static char *last_complete_ckpt;
/* Name of the active (not yet completed) checkpoint file */
static char active_ckpt[PATH_MAX];
/* Number of subresults already sent */
static int subresult_cnt;
/* Maximum number of allowed subresults */
static int max_subresults;
/* Name of the current work unit */
static char wu_name[256];
/* State machine for suspend */
static client_event_state client_state;


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
	DIR *dir;

	if (max_subresults)
		return max_subresults - subresult_cnt;

	dir = opendir(".");
	if (!dir)
		return 0;
	while ((d = readdir(dir)))
		if (!strncasecmp(d->d_name, SUBRESULT_PFX, strlen(SUBRESULT_PFX)))
			max_subresults++;
	closedir(dir);
	return max_subresults;
}

DC_GridCapabilities DC_getGridCapabilities(void)
{
	char path[PATH_MAX];
	unsigned caps;

	caps = DC_GC_EXITCODE | DC_GC_MESSAGING;
	if (DC_getMaxSubresults())
		caps |= DC_GC_SUBRESULT;

	if (!boinc_resolve_filename(DC_LABEL_STDOUT, path, sizeof(path)))
		caps |= DC_GC_STDOUT;
	if (!boinc_resolve_filename(DC_LABEL_STDERR, path, sizeof(path)))
		caps |= DC_GC_STDERR;
	if (!boinc_resolve_filename(DC_LABEL_CLIENTLOG, path, sizeof(path)))
		caps |= DC_GC_LOG;

	return (DC_GridCapabilities)caps;
}

/********************************************************************
 * Client API functions
 */

int DC_init(void)
{
	char path[PATH_MAX], *buf, label[32];
	struct stat st;
	int i, ret;
	FILE *f;

	/* We leave the redirection to BOINC and only copy stdout.txt/stderr.txt
	 * to the final output location in DC_finish(). This means BOINC can
	 * rotate the files so they won't grow too big. */
	if (boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR |
			BOINC_DIAG_REDIRECTSTDOUT))
		return DC_ERR_INTERNAL;

	if (boinc_init())
		return DC_ERR_INTERNAL;

	/* Check if we are starting from a checkpoint file */
	if ((f = boinc_fopen(LAST_CKPT_FILE, "r")))
	{
		fgets(path, sizeof(path), f);
		fclose(f);
		if (strlen(path))
			last_complete_ckpt = strdup(path);
	}
	/* If the application did not generate a checkpoint file before, check
	 * if the master sent one */
	else
		last_complete_ckpt = DC_resolveFileName(DC_FILE_IN, CKPT_LABEL_IN);

	/* Extract the WU name from init_data.xml */
	ret = stat(INIT_DATA_FILE, &st);
	if (ret)
		return DC_ERR_INTERNAL;

	f = boinc_fopen(INIT_DATA_FILE, "r");
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

	/* Parse the config file if the master sent one */
	buf = DC_resolveFileName(DC_FILE_IN, DC_CONFIG_FILE);
	if (buf && access(buf, R_OK))
	{
		ret = _DC_parseCfg(buf);
		if (ret)
			return ret;
	}
	free(buf);

	/* Initialize all optional output files as empty to prevent
	 * <file_xfer_error>s */
	for (i = 0; i < DC_getMaxSubresults(); i++)
	{
		snprintf(label, sizeof(label), "%s%d", SUBRESULT_PFX, i);
		if (boinc_resolve_filename(label, path, sizeof(path)))
			continue;
		f = boinc_fopen(path, "w");
		if (f)
			fclose(f);
	}
	ret = boinc_resolve_filename(CKPT_LABEL_OUT, path, sizeof(path));
	if (!ret)
	{
		f = boinc_fopen(path, "w");
		if (f)
			fclose(f);
	}

	DC_log(LOG_DEBUG, "DC-API initializef for work unit %s", wu_name);

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
			/* Remove the previous checkpoint if it was not
			 * finished */
			if (strlen(active_ckpt))
				unlink(active_ckpt);

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
	DC_FileMode mode)
{
	char label[32], new_path[PATH_MAX], msg[PATH_MAX], *p;
	int ret;

	if (!DC_getMaxSubresults())
	{
		DC_log(LOG_ERR, "No more subresults are allowed to be sent");
		return DC_ERR_NOTIMPL;
	}

	/* We have to use the subresult labels that were defined by the
	 * master for doing the upload */
	snprintf(label, sizeof(label), "%s%d", SUBRESULT_PFX, ++subresult_cnt);
	if (boinc_resolve_filename(label, new_path, sizeof(new_path)))
		return DC_ERR_INTERNAL;

	/* We need the file name that will appear on the server */
	p = strrchr(new_path, PATH_SEPARATOR[0]);
	if (!p)
	{
		DC_log(LOG_ERR, "Failed to determine the on-server file name "
			"of the subresult");
		return DC_ERR_INTERNAL;
	}
	p++;

	switch (mode)
	{
		case DC_FILE_REGULAR:
			ret = _DC_copyFile(path, new_path);
			if (ret)
				return DC_ERR_SYSTEM;
			break;
		case DC_FILE_PERSISTENT:
			ret = link(path, new_path);
			if (!ret)
				break;
			/* The client system may not support hard links so
			 * fall back to copying silently */
			ret = _DC_copyFile(path, new_path);
			if (ret)
				return DC_ERR_SYSTEM;
			break;
		case DC_FILE_VOLATILE:
			ret = rename(path, new_path);
			if (!ret)
				break;
			/* If renaming fails, fall back to copying, but in this
			 * case we are also responsible for deleting the
			 * original */
			ret = _DC_copyFile(path, new_path);
			if (ret)
				return DC_ERR_SYSTEM;
			break;
	}

	/* Stupid C++ */
	std::string str = label;
	ret = boinc_upload_file(str);
	if (ret)
	{
		unlink(new_path);
		return DC_ERR_INTERNAL;
	}

	snprintf(msg, sizeof(msg), "%s:%s:%s:%s", DCAPI_MSG_PFX, DC_MSG_UPLOAD,
		p, logicalFileName);
	DC_sendMessage(msg);

	/* If we had to copy volatile files, delete the original only when
	 * we are sure the upload will happen */
	if (mode == DC_FILE_VOLATILE)
		unlink(path);
	return 0;
}

int DC_sendMessage(const char *message)
{
	std::string xml, msg;
	int ret;

	msg = message;
	xml_escape(msg, xml);
	xml = "<message>" + xml + "</message>";
	/* We use the WU's name as the variety */
	ret = boinc_send_trickle_up(wu_name, (char *)xml.c_str());
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to send trickle-up message");
		return DC_ERR_INTERNAL;
	}
	return 0;
}

static DC_Event *new_event(DC_EventType type)
{
	DC_Event *event;

	event = (DC_Event *)calloc(1, sizeof(DC_Event));
	event->type = type;
	return event;
}

static DC_Event *handle_special_msg(const char *message)
{
	if (!strncmp(message, DC_MSG_CANCEL, strlen(DC_MSG_CANCEL)) &&
			message[strlen(DC_MSG_CANCEL)] == ':')
	{
		if (strcmp(message + strlen(DC_MSG_CANCEL) + 1, wu_name))
		{
			DC_log(LOG_WARNING, "Received CANCEL message for "
				"unknown WU %s",
				message + strlen(DC_MSG_CANCEL) + 1);
			return NULL;
		}
		client_state = STATE_FINISH;
		return new_event(DC_EVENT_FINISH);
	}
	if (!strcmp(message, DC_MSG_SUSPEND))
	{
		client_state = STATE_SUSPEND;
		return new_event(DC_EVENT_DO_CHECKPOINT);
	}

	DC_log(LOG_WARNING, "Received unknown control message %s", message);

	return NULL;
}

DC_Event *DC_checkEvent(void)
{
	DC_Event *event;
	char *buf, *msg;
	int ret;

	/* Check for checkpoint requests */
	if (boinc_time_to_checkpoint())
		return new_event(DC_EVENT_DO_CHECKPOINT);

	/* Check for messages */
	buf = (char *)malloc(MAX_MESSAGE_SIZE);
	if (!buf)
		/* Let's hope the error is transient and we can deliver the
		 * message when DC_checkEvent() is called for the next time */
		return NULL;

	/* Check for trickle-down messages and also handle internal
	 * communication */
	ret = boinc_receive_trickle_down(buf, MAX_MESSAGE_SIZE);
	if (ret)
	{
		msg = (char *)malloc(MAX_MESSAGE_SIZE);
		if (!msg)
		{
			free(buf);
			return NULL;
		}
		ret = parse_str(buf, "<message>", msg, MAX_MESSAGE_SIZE);
		if (!ret)
		{
			DC_log(LOG_WARNING, "Failed to parse message %s", buf);
			free(buf);
			free(msg);
			return NULL;
		}

		if (!strncmp(msg, DCAPI_MSG_PFX, strlen(DCAPI_MSG_PFX)) &&
				msg[strlen(DCAPI_MSG_PFX)] == ':')
		{
			event = handle_special_msg(msg +
				strlen(DCAPI_MSG_PFX) + 1);
			free(msg);
			return event;
		}

		event = new_event(DC_EVENT_MESSAGE);
		event->message = buf;
		return event;
	}
	free(buf);

	if (client_state == STATE_FINISH)
		return new_event(DC_EVENT_FINISH);

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
	FILE *f;

	if (strcmp(filename, active_ckpt))
	{
		DC_log(LOG_ERR, "DC_checkpointMade: bad checkpoint file %s "
			"(expected %s)", filename, active_ckpt);
		return;
	}

	/* Remember which was the last completed checkpoint */
	f = boinc_fopen(LAST_CKPT_FILE, "w");
	if (f)
	{
		fprintf(f, "%s", filename);
		fclose(f);
	}

	unlink(last_complete_ckpt);
	free(last_complete_ckpt);
	last_complete_ckpt = strdup(filename);
	/* Prevent it to be deleted by DC_resolveFileName() */
	active_ckpt[0] = '\0';

	boinc_checkpoint_completed();
	
	if (client_state == STATE_SUSPEND)
		client_state = STATE_FINISH;
}

void DC_fractionDone(double fraction)
{
	boinc_fraction_done(fraction);
}

void DC_finish(int exitcode)
{
	char path[PATH_MAX];
	int ret;

	/* Rename/copy the checkpoint file to the label CKPT_LABEL_OUT
	 * so it will be uploaded together with the result(s) */
	if (last_complete_ckpt &&
		!boinc_resolve_filename(CKPT_LABEL_OUT, path, sizeof(path)))
	{
		ret = link(last_complete_ckpt, path);
		if (ret)
			_DC_copyFile(last_complete_ckpt, path);
	}

	/* Delete files that we have created */
	if (last_complete_ckpt)
		unlink(last_complete_ckpt);
	if (strlen(active_ckpt))
		unlink(active_ckpt);
	unlink(LAST_CKPT_FILE);

	if (!boinc_resolve_filename(DC_LABEL_STDOUT, path, sizeof(path)))
		_DC_copyFile("stdout.txt", path);

	if (!boinc_resolve_filename(DC_LABEL_STDERR, path, sizeof(path)))
		_DC_copyFile("stderr.txt", path);

	boinc_finish(exitcode);
	/* We should never get here, but boinc_finish() is not marked
	 * with "noreturn" so this avoids a GCC warning */
	_exit(exitcode);
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
