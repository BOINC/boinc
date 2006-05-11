#include "dc_client.h"


/* Initializes the client API. */
int DC_init(void)
{
	return(0);
}


/* Resolves the local name of input/output files. */
char *DC_resolveFileName(DC_FileType type,
			 const char *logicalFileName)
{
	return(0);
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
DC_Event *DC_checkEvent(void)
{
	return(0);
}


/* Destroys the event-specific data returned by DC_checkEvent(). */
void DC_destroyEvent(DC_Event *event)
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
void DC_finish(int exitcode)
{
	for (;;) ;
}


/* End of condor/condor_slave.c */
