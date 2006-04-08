/*
 * DC-API: Distributed Computing Platform for Master-Worker Applications
 *
 * Client side
 *
 * Authors:
 * 	Norbert Podhorszki <pnorbert@sztaki.hu>
 * 	Gabor Vida <vida@sztaki.hu>
 * 	Gabor Gombas <gombasg@sztaki.hu>
 *
 * Copyright MTA SZTAKI, 2006
 */
#ifndef __DCCLIENT_H_
#define __DCCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dc_common.h>

/********************************************************************
 * Constant definitions
 */

/* File types for the DC_ResolveFileName function */
typedef enum {
	DC_FILE_IN,
	DC_FILE_OUT,
	DC_FILE_TMP
} DC_FileType;

/* Special handle for the checkpoint file */
#define DC_CHECKPOINT_FILE	"__DC_CHECKPOINT_"

/* Control events */
typedef enum {
	DC_EVENT_DO_CHECKPOINT,	/* Checkpointing is requested */
	DC_EVENT_FINISH,	/* Computation should be aborted */
	DC_EVENT_MESSAGE	/* A message has arrived */
} DC_EventType;


/********************************************************************
 * Data types
 */

/* Description of a DC-API event */
typedef struct _DC_Event	DC_Event;
struct _DC_Event
{
	DC_EventType		type;
	union
	{
		char		*message;
	};
};


/********************************************************************
 * Function prototypes
 */

/* Initializes the client API. */
int DC_init(void);

/* Resolves the local name of input/output files. */
char *DC_resolveFileName(DC_FileType type, const char *logicalFileName);

/* Sends a sub-result back to the master. */
int DC_sendResult(const char *logicalFileName, const char *path,
	DC_FileMode fileMode);

/* Sends a message to the master. */
int DC_sendMessage(const char *message);

/* Checks for application control events. */
DC_Event *DC_checkEvent(void);

/* Destroys the event-specific data returned by DC_checkEvent(). */
void DC_destroyEvent(DC_Event *event);

/* Indicates that an application-level checkpoint has completed. */
void DC_checkpointMade(const char *fileName);

/* Informs the user interface about the fraction of work already done. */
void DC_fractionDone(double fraction);

/* Finishes computation. */
void DC_finish(int exitcode);

#ifdef __cplusplus
}
#endif

#endif
