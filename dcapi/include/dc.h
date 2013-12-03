/*
 * DC-API: Distributed Computing Platform for Master-Worker Applications
 *
 * Master side
 *
 * Authors:
 * 	Norbert Podhorszki <pnorbert@sztaki.hu>
 * 	Gabor Vida <vida@sztaki.hu>
 * 	Gabor Gombas <gombasg@sztaki.hu>
 *
 * Copyright MTA SZTAKI, 2006
 */
#ifndef _DC_H_
#define _DC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dc_common.h>

/********************************************************************
 * Constant definitions
 */

/* Possible states of a work unit */
typedef enum {
	DC_WU_READY,	/* Created, but not yet submitted */
	DC_WU_RUNNING,	/* Submitted and running */
	DC_WU_FINISHED,	/* The WU finished normally according to the grid
			   infrastructure. Note that the application client
			   returning an error code is considered a 'normal'
			   shutdown as far as the infrastructure is concerned,
			   so you should check the exit status and/or other
			   output. */
	DC_WU_SUSPENDED,/* The work unit is suspended */
	DC_WU_ABORTED,	/* The WU was aborted for some reason (infrastructure
			   failure, no canonical result, or by calling
			   DC_cancel()) */
	DC_WU_UNKNOWN	/* The WU's state is not known. This may happen for
			   example when a WU is serialized, then it finishes,
			   and upon deserialization the underlying grid
			   infrastructure no longer knows about it */
} DC_WUState;

/* Possible event types */
typedef enum {
	DC_MASTER_RESULT,	/* A DC_Result is available */
	DC_MASTER_SUBRESULT,	/* A sub-result is available */
	DC_MASTER_MESSAGE	/* A message has arrived */
} DC_MasterEventType;


/********************************************************************
 * Data types
 */

/* Opaque type representing a workunit */
typedef struct _DC_Workunit	DC_Workunit;

/* Opaque type representing a result */
typedef struct _DC_Result	DC_Result;

/* Description of a DC-API event */
typedef struct _DC_MasterEvent	DC_MasterEvent;
struct _DC_MasterEvent
{
	DC_MasterEventType	type;
	DC_Workunit		*wu;
	union
	{
		DC_Result	*result;
		DC_PhysicalFile	*subresult;
		char		*message;
	};
};

/* Prototype of the result handling callback function. */
typedef void (*DC_ResultCallback)(DC_Workunit *wu, DC_Result *result);

/* Prototype of the sub-result handling callback. */
typedef void (*DC_SubresultCallback)(DC_Workunit *wu,
	const char *logicalFileName, const char *path);

/* Prototype of the message-handling callback. */
typedef void (*DC_MessageCallback)(DC_Workunit *wu, const char *message);


/********************************************************************
 * Function prototypes: Library utilities
 */

/* Initializes the DC-API. */
int DC_initMaster(const char *configFile);

/* Sets the callback functions that will be called when a particular event. */
void DC_setMasterCb(DC_ResultCallback resultcb,
	DC_SubresultCallback subresultcb, DC_MessageCallback msgcb);

/* Set the callback functions for particular events */
void DC_setResultCb(DC_ResultCallback cb);
void DC_setSubresultCb(DC_SubresultCallback cb);
void DC_setMessageCb(DC_MessageCallback cb);

/* Queries the number of WUs known to the API in the given state. */
int DC_getWUNumber(DC_WUState state);

/* Queries per-client configuration variables */
char *DC_getClientCfgStr(const char *clientName, const char *key,
	int fallbackGlobal);

/* Queries per-client configuration variables */
int DC_getClientCfgInt(const char *clientName, const char *key,
	int defaultValue, int fallbackGlobal);

/* Queries per-client configuration variables */
double DC_getClientCfgDouble(const char *clientName, const char *key,
	double defaultValue, int fallbackGlobal);

/* Queries per-client configuration variables */
int DC_getClientCfgBool(const char *clientName, const char *key,
	int defaultValue, int fallbackGlobal);

/********************************************************************
 * Function prototypes: Event processing
 */

/* Waits for events and processes them. */
int DC_processMasterEvents(int timeout);

/* Checks for events and return them. */
DC_MasterEvent *DC_waitMasterEvent(const char *wuFilter, int timeout);

/* Checks for events for a particular WU. */
DC_MasterEvent *DC_waitWUEvent(DC_Workunit *wu, int timeout);

/* Destroys an event. */
void DC_destroyMasterEvent(DC_MasterEvent *event);

/********************************************************************
 * Function prototypes: Work unit management
 */

/* Creates one work unit. */
DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag);

/* Sets an input file for the work unit. */
int DC_addWUInput(DC_Workunit *wu, const char *logicalFileName, const char *URL,
	DC_FileMode fileMode, ...);

/* Defines an output file for the work unit. */
int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName);

/* Sets the priority for the work unit. */
int DC_setWUPriority(DC_Workunit *wu, int priority);

/* Sets the batch id for the work unit. */
int DC_setWUBatch(DC_Workunit *wu, int batch);

/* Serializes a work unit description. */
char *DC_serializeWU(DC_Workunit *wu);

/* Restores a serialized work unit. */
DC_Workunit *DC_deserializeWU(const char *buf);

/* Queries the state of a work unit. */
DC_WUState DC_getWUState(DC_Workunit *wu);

/* Submits a work unit. */
int DC_submitWU(DC_Workunit *wu);

/* Queries the low-level ID of the work unit. */
char *DC_getWUId(const DC_Workunit *wu);

/* Queries the tag of a work unit. */
char *DC_getWUTag(const DC_Workunit *wu);

/* Cancels all computations for a given work unit. */
int DC_cancelWU(DC_Workunit *wu);

/* Temporarily suspends the execution of a work unit. */
int DC_suspendWU(DC_Workunit *wu);

/* Resumes computation of a previously suspended work unit. */
int DC_resumeWU(DC_Workunit *wu);

/* Releases internal resources allocated to a work unit. */
void DC_destroyWU(DC_Workunit *wu);

/* Sends a message to a running work unit. */
int DC_sendWUMessage(DC_Workunit *wu, const char *message);

/********************************************************************
 * Function prototypes: Result handling
 */

/* Queries what optional fields are present in the result. */
unsigned DC_getResultCapabilities(const DC_Result *result);

/* Returns the WU that generated this result. */
DC_Workunit *DC_getResultWU(DC_Result *result);

/* Returns the exit code of the client application. */
int DC_getResultExit(const DC_Result *result);

/* Returns the local name of an output file. */
char *DC_getResultOutput(const DC_Result *result, const char *logicalFileName);

/* Returns the CPU time used by the result. */
double DC_getResultCPUTime(const DC_Result *result);

#ifdef __cplusplus
}
#endif

#endif /* _DC_H_ */
