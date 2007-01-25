#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dc_client.h>

#include "local_common.h"

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
	return 0;
}

char *DC_resolveFileName(DC_FileType type, const char *logicalFileName)
{
        if (!strcmp(logicalFileName, DC_CHECKPOINT_FILE))
        {
		return strdup(CKPT_LABEL);
        }
	return strdup(logicalFileName);
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
	// Nothing to do with it.
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
