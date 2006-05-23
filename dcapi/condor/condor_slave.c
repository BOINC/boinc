/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <string.h>
#include <stdlib.h>

#include "dc_common.h"
#include "dc_client.h"


/* Initializes the client API. */
int DC_initClient(void)
{
	return(0);
}


/* Resolves the local name of input/output files. */
char *DC_resolveFileName(DC_FileType type,
			 const char *logicalFileName)
{
	if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
		return("dc_checkpoint.txt");
	return((char*)logicalFileName);
}


/* Sends a sub-result back to the master. */
int DC_sendResult(const char *logicalFileName,
		  const char *path,
		  DC_FileMode fileMode)
{
	return(0);
}


/* Sends a message to the master. */
int DC_sendMessage(const char *message)
{
	return(0);
}


/* Checks for application control events. */
DC_ClientEvent *DC_checkClientEvent(void)
{
	return(0);
}


/* Destroys the event-specific data returned by DC_checkClientEvent(). */
void DC_destroyClientEvent(DC_ClientEvent *event)
{
}


/* Indicates that an application-level checkpoint has completed. */
void DC_checkpointMade(const char *fileName)
{
}


/* Informs the user interface about the fraction of work already done. */
void DC_fractionDone(double fraction)
{
}


/* Finishes computation. */
void DC_finishClient(int exitcode)
{
	exit(exitcode);
}


/* End of condor/condor_slave.c */
