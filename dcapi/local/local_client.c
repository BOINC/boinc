/*
 * local/local_client.c
 *
 * DC-API functions of client side
 *
 * (c) Gabor Vida 2005-2006, Daniel Drotos, 2007
 */

/* $Id$ */
/* $Date$ */
/* $Revision$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include <dc_client.h>

#include "local_common.h"


static int _DC_checkpoint_file_requested= 0;
static int _DC_checkpoint_made= 0;

static char *
_DC_cfg(enum _DC_e_param what)
{
	if (what >= cfg_nuof)
		return(NULL);
	if (_DC_params[what].lvalue)
		return(_DC_params[what].lvalue);

	_DC_params[what].lvalue= DC_getCfgStr(_DC_params[what].name);
	if (_DC_params[what].lvalue)
		return(_DC_params[what].lvalue);
	return(_DC_params[what].def);
}

/********************************************************************
 * Common API functions
 */

int DC_getMaxMessageSize(void)
{
        return MAX_MESSAGE_SIZE;
}

int DC_getMaxSubresults(void)
{
        return MAX_SUBRESULTS;
}

unsigned DC_getGridCapabilities(void)
{
        return (DC_GridCapabilities)(DC_GC_STDERR | DC_GC_STDOUT);
}

/********************************************************************
 * Client API functions
 */

int DC_initClient(void)
{
	_DC_init_common();
	return 0;
}

char *DC_resolveFileName(DC_FileType type, const char *logicalFileName)
{
	/* init_log calls this fn, so it is not possible to call DC_log
	   from here */
	/*DC_log(LOG_DEBUG, "DC_resolveFileName(%d,%s)",
	  type, logicalFileName);*/
	char *cn= _DC_cfg(cfg_checkpoint_file);
	if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
	{
		switch (type)
		{
		case DC_FILE_IN:
		{
			/* - param of last DC_checkpointMade()
			   - filename created by previous run
			   - NULL otherwise */
			if (_DC_checkpoint_made)
			{
				char *fn= (char*)malloc(strlen(cn)+100);
				sprintf(fn, "%s_finished.txt", cn);
				return(fn);
			}
			else
			{
				FILE *f;
				char *fn= (char*)malloc(strlen(cn)+100);
				sprintf(fn, "%s_finished.txt", cn);
				if ((f= fopen(fn, "r")) != NULL)
				{
					fclose(f);
					return(fn);
				}
				else
				{
					free(fn);
					return(NULL);
				}
			}
			break;
		}
		case DC_FILE_OUT:
		{
			char *s;
			/* new non-existant name */
			if (_DC_checkpoint_file_requested)
			{
				DC_log(LOG_ERR, "Checkpoint file creation "
				       "can not be restarted "
					"(DC_resolveFileName("
					DC_CHECKPOINT_FILE
					") called twice without calling of "
					"DC_checkpointMade())");
				return(NULL);
			}
			_DC_checkpoint_file_requested= 1;
			s= (char*)malloc(strlen(cn)+100);
			sprintf(s, "%s_creating.txt", cn);
			return(s);
			break;
		}
		default:
			return(NULL);
		}
	}
	return(strdup((char*)logicalFileName));
/*
        if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
        {
		return strdup(CKPT_LABEL);
        }
	return strdup(logicalFileName);
*/
}

int DC_sendResult(const char *logicalFileName, const char *path, DC_FileMode fileMode)
{
	// not implemented yet!

	return DC_ERR_NOTIMPL;
}

int DC_sendMessage(const char *message)
{
	// not implemented yet!

	return DC_ERR_NOTIMPL;
}

DC_ClientEvent *DC_checkClientEvent(void)
{
	// not implemented yet!

	return NULL;
}

void DC_destroyClientEvent(DC_ClientEvent *event)
{
	// not implemented yet!
}

void DC_checkpointMade(const char *fileName)
{
	DC_log(LOG_DEBUG, "DC_checkpointMade(%s)", fileName);
	_DC_checkpoint_file_requested= 0;
	if (fileName)
	{
		char *cn= _DC_cfg(cfg_checkpoint_file);
		char *fn;
		fn= (char*)malloc(strlen(cn)+100);
		sprintf(fn, "%s_finished.txt", cn);
		if (rename(fileName, fn) != 0)
		{
			DC_log(LOG_ERR, "Renaming %s to %s failed: %s",
			       fileName, fn, strerror(errno));
		}
		else
			_DC_checkpoint_made++;
		free(fn);
	}
}

void DC_fractionDone(double fraction)
{
	// Nothing to do with it.
}

void DC_finishClient(int exitcode)
{
	// XXX Need to send a message to the master side of the DC_API!

	exit(exitcode);
}


/* End of local/local_client.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
