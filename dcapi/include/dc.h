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
#ifndef __DC_H_
#define __DC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/syslog.h>
#include <stdarg.h>

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
	DC_EVENT_RESULT,	/* A DC_Result is available */
	DC_EVENT_SUBRESULT,	/* A sub-result is available */
	DC_EVENT_MESSAGE	/* A message has arrived */
} DC_EventType;

/* Special result files, used by DC_getResultOutput() */
#define DC_RESULT_STDOUT	"__DC_STDOUT_"	/* The client's standard
						   output */
#define DC_RESULT_STDERR	"__DC_STDERR_"	/* The client's standard
						   error */
#define DC_RESULT_LOG		"__DC_LOG_"	/* The system's log */


/********************************************************************
 * Data types
 */

/* Opaque type representing a workunit */
typedef struct _DC_Workunit	DC_Workunit;

/* Opaque type representing a result */
typedef struct _DC_Result	DC_Result;

/* Descriptor of a physical file */
typedef struct _DC_PhysicalFile	DC_PhysicalFile;
struct _DC_PhysicalFile
{
	char			*label;
	char			*path;
	DC_FileMode		mode;
};

/* Description of a DC-API event */
typedef struct _DC_Event	DC_Event;
struct _DC_Event
{
	DC_EventType		type;
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
int DC_init(const char *configFile);

/* Prints a message to the log file. */
void DC_log(int level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
void DC_vlog(int level, const char *fmt, va_list args)
	__attribute__((format(printf, 2, 0)));

/* Sets the callback functions that will be called when a particular event. */
void DC_setcb(DC_ResultCallback resultcb, DC_SubresultCallback subresultcb,
	DC_MessageCallback msgcb);

/* Waits for events and processes them. */
int DC_processEvents(int timeout);

/* Checks for events and return them. */
DC_Event *DC_waitEvent(const char *wuFilter, int timeout);

/* Checks for events for a particular WU. */
DC_Event *DC_waitWUEvent(DC_Workunit *wu, int timeout);

/* Destroys an event. */
void DC_destroyEvent(DC_Event *event);

/* Queries the number of WUs known to the API in the given state. */
int DC_getWUNumber(DC_WUState state);

/********************************************************************
 * Function prototypes: Work unit management
 */

/* Creates one work unit. */
DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag);

/* Sets an input file for the work unit. */
int DC_addWUInput(DC_Workunit *wu, const char *logicalFileName, const char *URL,
	DC_FileMode fileMode);

/* Defines an output file for the work unit. */
int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName);

/* Sets the priority for the work unit. */
int DC_setWUPriority(DC_Workunit *wu, int priority);

/* Serializes a work unit description. */
char *DC_serializeWU(DC_Workunit *wu);

/* Restores a serialized work unit. */
DC_Workunit *DC_deserializeWU(const char *buf);

/* Queries the state of a work unit. */
DC_WUState DC_getWUState(DC_Workunit *wu);

/* Submits a work unit. */
int DC_submitWU(DC_Workunit *wu);

/* Queries the low-level ID of the work unit. */
char *DC_getWUId(DC_Workunit *wu);

/* Queries the tag of a work unit. */
char *DC_getWUTag(DC_Workunit *wu);

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
unsigned DC_getResultCapabilities(DC_Result *result);

/* Returns the WU that generated this result. */
DC_Workunit *DC_getResultWU(DC_Result *result);

/* Returns the exit code of the client application. */
int DC_getResultExit(DC_Result *result);

/* Returns the local name of an output file. */
char *DC_getResultOutput(DC_Result *result, const char *logicalFileName);

#ifdef __cplusplus
}
#endif

#endif /* __DC_H_ */
