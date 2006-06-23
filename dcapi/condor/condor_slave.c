/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "dc_common.h"
#include "dc_client.h"
#include "dc_internal.h"

#include "condor_common.h"
#include "condor_slave.h"
#include "condor_utils.h"


/* Initializes the client API. */
int DC_initClient(void)
{
	int ret= _DC_parseCfg(CLIENT_CONFIG_NAME);
	if (ret)
		fprintf(stderr, "Error parsing configfile %s",
			CLIENT_CONFIG_NAME);
	DC_log(LOG_DEBUG, "Slave dcapi initialized");
	return(ret);
}


/* Resolves the local name of input/output files. */
char *DC_resolveFileName(DC_FileType type,
			 const char *logicalFileName)
{
	/* init_log calls this fn, so it is not possible to call DC_log
	   from here */
	/*DC_log(LOG_DEBUG, "DC_resolveFileName(%d,%s)",
	  type, logicalFileName);*/
	if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
		return(strdup(CKPT_LABEL));
	return(strdup((char*)logicalFileName));
}


/* Sends a sub-result back to the master. */
int DC_sendResult(const char *logicalFileName,
		  const char *path,
		  DC_FileMode fileMode)
{
	char *fn;
	int ret;

	DC_log(LOG_DEBUG, "DC_sendResult(%s,%s,%d)",
	       logicalFileName,
	       path,
	       fileMode);
	fn= malloc(strlen(logicalFileName)+100);
	strcpy(fn, "client_subresults");
	strcat(fn, "/real_files");
	if ((ret= _DC_mkdir_with_parents(fn, S_IRWXU|
					 S_IRGRP|S_IXGRP|
					 S_IROTH|S_IXOTH)) != DC_OK)
	{
		DC_log(LOG_ERR, "Failed to create dir for subresult (%s): %s",
		       fn, strerror(errno));
		free(fn);
		return(ret);
	}
	strcat(fn, "/");
	strcat(fn, logicalFileName);
	if ((ret= _DC_copyFile(path, fn)) != DC_OK)
	{
		DC_log(LOG_ERR, "Failed to copy subresult file %s to "
		       "%s: %s", path, fn, strerror(errno));
		free(fn);
		return(ret);
	}
	ret= _DC_create_message("client_subresults", "logical_name",
				logicalFileName, NULL);
	free(fn);
	return(ret);
}


/* Sends a message to the master. */
int DC_sendMessage(const char *message)
{
	DC_log(LOG_DEBUG, "DC_sendMessage(%s)", message);
	return _DC_create_message("client_messages", "message", message, NULL);
}


/* Checks for application control events. */
DC_ClientEvent *DC_checkClientEvent(void)
{
	char *message;
	DC_ClientEvent *e= NULL;

	message= _DC_read_message("master_messages", "message.", 1);
	if (message)
	{
		if ((e= calloc(1, sizeof(DC_ClientEvent))))
		{
			DC_log(LOG_DEBUG, "API event created: %p", e);
			e->type= DC_CLIENT_MESSAGE;
			e->message= message;
			DC_log(LOG_DEBUG, "Message of the event: %s",
			       e->message);
		}
		else
		{
			free(message);
			DC_log(LOG_ERR, "Failed to create "
			       "API event, memory allocation "
			       "error");
		}
	}
	return(e);
}


/* Destroys the event-specific data returned by DC_checkClientEvent(). */
void DC_destroyClientEvent(DC_ClientEvent *event)
{
	DC_log(LOG_DEBUG, "DC_destroyClientEvent(%p)", event);
	if (event)
	{
		if (event->message)
			free(event->message);
		free(event);
	}
}


/* Indicates that an application-level checkpoint has completed. */
void DC_checkpointMade(const char *fileName)
{
	DC_log(LOG_DEBUG, "DC_checkpointMade(%s)", fileName);
}


/* Informs the user interface about the fraction of work already done. */
void DC_fractionDone(double fraction)
{
	DC_log(LOG_DEBUG, "DC_fractionDone(%g)", fraction);
}


/* Finishes computation. */
void DC_finishClient(int exitcode)
{
	DC_log(LOG_DEBUG, "DC_finishClient(%d)", exitcode);
	exit(exitcode);
}


/* End of condor/condor_slave.c */
