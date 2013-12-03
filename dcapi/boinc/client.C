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
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#ifndef _WIN32
#include <dirent.h>
#endif

/* BOINC includes */
#ifdef _WIN32
#include <boinc_win.h>
#include <util.h>
#endif

#include <boinc_api.h>
#include <filesys.h>
#include <diagnostics.h>
#include <graphics2.h>
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

#ifdef _WIN32
#define PATHSEP			'\\'
#define __func__		"DC-API"
#else
#define PATHSEP			'/'
#endif


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

#ifdef _WIN32
static void count_subresults(void)
{
	WIN32_FIND_DATA fdata;
	HANDLE hfind;

	hfind = FindFirstFile("*", &fdata);
	if (hfind == INVALID_HANDLE_VALUE)
		return;
	do {
		DC_log(LOG_DEBUG, "\t%s", fdata.cFileName);
		if (!strncmp(fdata.cFileName, SUBRESULT_PFX,
				strlen(SUBRESULT_PFX)))
			max_subresults++;
	} while (FindNextFile(hfind, &fdata));
	FindClose(hfind);
}
#else
static void count_subresults(void)
{
	struct dirent *d;
	DIR *dir;

	dir = opendir(".");
	if (!dir)
		return;

	DC_log(LOG_DEBUG, "Workdir contents:");
	while ((d = readdir(dir)))
	{
		DC_log(LOG_DEBUG, "\t%s", d->d_name);
		if (!strncmp(d->d_name, SUBRESULT_PFX, strlen(SUBRESULT_PFX)))
			max_subresults++;
	}
	closedir(dir);
}
#endif

/* Determine the number of allowed subresults by counting all output files
 * that have a name starting with SUBRESULT_PFX */
int DC_getMaxSubresults(void)
{
	if (max_subresults)
		return max_subresults - subresult_cnt;

	count_subresults();
	return max_subresults;
}

unsigned DC_getGridCapabilities(void)
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

	return caps;
}

/********************************************************************
 * Client API functions
 */

int DC_initClient(void)
{
	char path[PATH_MAX], *buf, label[32];
	int i, ret;
	FILE *f;

	/* We leave the redirection to BOINC and only copy stdout.txt/stderr.txt
	 * to the final output location in DC_finishClient(). This means BOINC
	 * can rotate the files so they won't grow too big. */
	if (boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR |
			BOINC_DIAG_REDIRECTSTDOUT))
		return DC_ERR_INTERNAL;

	if (boinc_init())
		return DC_ERR_INTERNAL;

	/* Parse the config file if the master sent one */
	buf = DC_resolveFileName(DC_FILE_IN, DC_CONFIG_FILE);
	if (buf)
	{
		ret = _DC_parseCfg(buf);
		if (ret && ret != DC_ERR_SYSTEM)
			return ret;
	}
	free(buf);

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

	if (last_complete_ckpt)
		DC_log(LOG_INFO, "Found initial checkpoint file %s",
			last_complete_ckpt);

	/* Extract the WU name from init_data.xml */
	if (boinc_is_standalone())
	{
		DC_log(LOG_NOTICE, "Running in stand-alone mode, some "
			"functions are not available");
		wu_name[0] = '\0';
	}
	else
	{
		APP_INIT_DATA init_data;

		boinc_get_init_data(init_data);
		strncpy(wu_name, init_data.wu_name, sizeof(wu_name));
	}

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

	DC_log(LOG_INFO, "DC-API initialized for work unit %s", wu_name);

	return 0;
}

char *DC_resolveFileName(DC_FileType type, const char *logicalFileName)
{
	char buf[PATH_MAX];
	int ret;

	if (!logicalFileName)
	{
		DC_log(LOG_ERR, "%s: Missing logical file name", __func__);
		return NULL;
	}

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
			DC_log(LOG_DEBUG, "Opened new checkpoint file %s",
				active_ckpt);
			return strdup(active_ckpt);
		}
		else
			return NULL;
	}

	/* Temporary files can be created in the current directory */
	if (type == DC_FILE_TMP)
		return strdup(logicalFileName);

	ret = boinc_resolve_filename(logicalFileName, buf, sizeof(buf));
	if (!ret) {
        if (type == DC_FILE_IN) {
    	    if (FILE * file = fopen(buf, "r")) 
    	    {
                fclose(file);
        		return strdup(buf);	    
    	    }
            return NULL;
    	}
		return strdup(buf);	    
	}

	/* Do not fail for missing output files in stand-alone mode */
	if (!wu_name[0] && type == DC_FILE_OUT)
		return strdup(logicalFileName);
	return NULL;
}

int DC_sendResult(const char *logicalFileName, const char *path,
	DC_FileMode mode)
{
	char label[32], new_path[PATH_MAX], msg[PATH_MAX], *p;
	int ret;

	if (!logicalFileName)
	{
		DC_log(LOG_ERR, "%s: Missing logical file name", __func__);
		return DC_ERR_BADPARAM;
	}
	if (!path)
	{
		DC_log(LOG_ERR, "%s: Missing path", __func__);
		return DC_ERR_BADPARAM;
	}

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
	p = strrchr(new_path, PATHSEP);
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
#ifndef _WIN32
			ret = link(path, new_path);
			if (!ret)
				break;
#endif
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
		case DC_FILE_REMOTE:
			/* Ignore this case, entirely handled by BOINC */
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

	DC_log(LOG_INFO, "File %s (%s) has been scheduled for upload",
		logicalFileName, label);

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
	char *xml, *msg;
	int ret, len;

	if (!message)
	{
		DC_log(LOG_ERR, "%s: Missing message", __func__);
		return DC_ERR_BADPARAM;
	}

	if (!wu_name[0])
	{
		DC_log(LOG_ERR, "Messaging is not available in stand-alone mode");
		return DC_ERR_NOTIMPL;
	}

	int buflen = 6 * strlen(message) + 1;
	xml = (char *)malloc(buflen);
	if (!xml)
	{
		DC_log(LOG_ERR, "Sending message: Out of memory");
		return DC_ERR_INTERNAL;
	}

	xml_escape(message, xml, buflen);
	len = strlen(xml) + 24;
	msg = (char *)malloc(len);
	if (!msg)
	{
		DC_log(LOG_ERR, "Sending message: Out of memory");
		free(xml);
		return DC_ERR_INTERNAL;
	}
	snprintf(msg, len, "<message>%s</message>", xml);

	/* We use the WU's name as the variety */
	ret = boinc_send_trickle_up(wu_name, msg);
	free(msg);
	free(xml);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to send trickle-up message");
		return DC_ERR_INTERNAL;
	}
	return 0;
}

static DC_ClientEvent *new_event(DC_ClientEventType type)
{
	DC_ClientEvent *event;

	event = (DC_ClientEvent *)calloc(1, sizeof(DC_ClientEvent));
	event->type = type;
	return event;
}

static DC_ClientEvent *handle_special_msg(const char *message)
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
		DC_log(LOG_NOTICE, "Received cancel request from the master");
		client_state = STATE_FINISH;
		return new_event(DC_CLIENT_FINISH);
	}
	if (!strcmp(message, DC_MSG_SUSPEND))
	{
		DC_log(LOG_NOTICE, "Received suspend request from the master");
		client_state = STATE_SUSPEND;
		return new_event(DC_CLIENT_CHECKPOINT);
	}

	DC_log(LOG_WARNING, "Received unknown control message %s", message);

	return NULL;
}

DC_ClientEvent *DC_checkClientEvent(void)
{
	DC_ClientEvent *event;
	char *buf, *msg;
	int ret;

	/* Check for checkpoint requests */
	if (boinc_time_to_checkpoint())
	{
		DC_log(LOG_DEBUG, "Core client requested checkpoint");
		return new_event(DC_CLIENT_CHECKPOINT);
	}

	/* Check for messages */
	buf = (char *)malloc(MAX_MESSAGE_SIZE);
	if (!buf)
		/* Let's hope the error is transient and we can deliver the
		 * message when DC_checkClientEvent() is called for the next time */
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

		DC_log(LOG_DEBUG, "Received trickle message");

		event = new_event(DC_CLIENT_MESSAGE);
		event->message = buf;
		return event;
	}
	free(buf);

	if (client_state == STATE_FINISH)
		return new_event(DC_CLIENT_FINISH);

	return NULL;
}

void DC_destroyClientEvent(DC_ClientEvent *event)
{
	if (!event)
		return;

	switch (event->type)
	{
		case DC_CLIENT_MESSAGE:
			free(event->message);
			break;
		default:
			break;
	}
}

void DC_checkpointMade(const char *filename)
{
	FILE *f;

	if (!filename)
	{
		/* No file name - reset the checkpoint logic */
		if (active_ckpt[0])
			unlink(active_ckpt);
		active_ckpt[0] = '\0';

		if (last_complete_ckpt)
		{
			unlink(last_complete_ckpt);
			free(last_complete_ckpt);
			last_complete_ckpt = NULL;
		}

		boinc_checkpoint_completed();
		return;
	}

	if (strcmp(filename, active_ckpt))
	{
		DC_log(LOG_ERR, "DC_checkpointMade: bad checkpoint file %s "
			"(expected %s)", filename, active_ckpt);
		boinc_checkpoint_completed();
		return;
	}

	DC_log(LOG_INFO, "Completed checkpoint %s", filename);

	/* Remember which was the last completed checkpoint */
	f = boinc_fopen(LAST_CKPT_FILE, "w");
	if (f)
	{
		fprintf(f, "%s", filename);
		fclose(f);
	}

	if (last_complete_ckpt)
	{
		unlink(last_complete_ckpt);
		free(last_complete_ckpt);
	}
	last_complete_ckpt = strdup(filename);

	/* Reset the active checkpoint */
	active_ckpt[0] = '\0';

	boinc_checkpoint_completed();
	
	if (client_state == STATE_SUSPEND)
		client_state = STATE_FINISH;
}

void DC_fractionDone(double fraction)
{
	boinc_fraction_done(fraction);
}

void DC_finishClient(int exitcode)
{
	char path[PATH_MAX];
	int ret;

	/* Rename/copy the checkpoint file to the label CKPT_LABEL_OUT
	 * so it will be uploaded together with the result(s) */
	if (last_complete_ckpt &&
		!boinc_resolve_filename(CKPT_LABEL_OUT, path, sizeof(path)))
	{
#ifndef _WIN32
		ret = link(last_complete_ckpt, path);
		if (ret)
#endif
		ret = _DC_copyFile(last_complete_ckpt, path);

		DC_log(LOG_DEBUG, "Uploading last complete checkpoint %s",
			last_complete_ckpt);
	}

	/* Delete files that we have created */
	if (last_complete_ckpt)
		unlink(last_complete_ckpt);
	if (strlen(active_ckpt))
		unlink(active_ckpt);
	unlink(LAST_CKPT_FILE);

	/* Make sure the output is on disk */
	DC_log(LOG_DEBUG, "Flushing stdout/stderr");
	fflush(stdout);
	fflush(stderr);

	if (!boinc_resolve_filename(DC_LABEL_STDOUT, path, sizeof(path)))
	{
		DC_log(LOG_DEBUG, "Uploading stdout.txt");
		_DC_copyFile("stdout.txt", path);
	}

	if (!boinc_resolve_filename(DC_LABEL_STDERR, path, sizeof(path)))
	{
		DC_log(LOG_DEBUG, "Uploading stderr.txt");
		_DC_copyFile("stderr.txt", path);
	}

	DC_log(LOG_INFO, "DC-API: Application finished with exit code %d",
		exitcode);
	boinc_finish(exitcode);
	/* We should never get here, but boinc_finish() is not marked
	 * with "noreturn" so this avoids a GCC warning */
	exit(exitcode);
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
