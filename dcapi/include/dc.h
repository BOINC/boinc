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
	union
	{
		DC_Result	*result;
		DC_PhysicalFile	*subresult;
		char		*message;
	};
};

/* Prototype of the result handling callback function.
 *
 * @wu: The WU that generated this result.
 * @result: The resut descriptor. It is %NULL if the work unit terminated
 * abnormally without generating a result.
 */
typedef void (*DC_ResultCallback)(DC_Workunit *wu, DC_Result *result);

/* Prototype of the sub-result handling callback.
 *
 * NOTE: Unlike real results, sub-results are _NOT_ validated, so the
 * application must be prepared to process sub-results even from clients
 * that will be marked as 'invalid' in the end.
 *
 * @wu: The WU this sub-result belongs to.
 * @logicalFileName: The logical name of the sub-result as set by the client
 * 	(usually the file name on the client side).
 * @path: The local filename of the sub-result.
 */
typedef void (*DC_SubresultCallback)(DC_Workunit *wu,
	const char *logicalFileName, const char *path);

/* Prototype of the message-handling callback.
 *
 * NOTE: The master application may receive messages from clients that the
 * validator will later reject as 'invalid', so be prepared to handle this.
 *
 * @wu: The WU sending the message.
 * @message: The received message.
 */
typedef void (*DC_MessageCallback)(DC_Workunit *wu, const char *message);


/********************************************************************
 * Function prototypes: Library utilities
 */

/** Initializes the DC-API.
 *
 * @configFile: Name of the config file to use. If %NULL, DC_init will look
 * 	for a file named 'dc_api.conf' in the current directory.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_init(const char *configFile);

/** Prints a message to the log file.
 *
 * @priority: Log level (see syslog(3) for the possible values)
 * @fmt: Format string (like printf)
 */
void DC_log(int level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

/** Prints a message to the log file.
 *
 * @priority: Log level (see syslog(3) for the possible values)
 * @fmt: Format string (like printf)
 * @args: Argument list matching the format string
 */
void DC_vlog(int level, const char *fmt, va_list args)
	__attribute__((format(printf, 2, 0)));

/** Sets the callback functions that will be called when a particular event
 * occurs.
 *
 * @resultcb: Called when a WU finishes or aborts and a result is available.
 * @subresultcb: Called when a WU sends back a sub-result.
 * @msgcb: Called when a WU sends a message.
 */
void DC_setcb(DC_ResultCallback resultcb, DC_SubresultCallback subresultcb,
	DC_MessageCallback msgcb);

/** Waits for events and processes them.
 *
 * @timeout: max. time to wait for an event to arrive.
 *
 * @Returns: 0 if at least one event was processed, an error code otherwise.
 */
int DC_processEvents(int timeout);

/** Checks for events and return them.
 *
 * @wuFilter: if not %NULL, only work units that have a tag matching the
 * 	filter will be checked for outstanding events.
 * @timeout: seconds to wait for an event to occur (result, subresult or
 * 	message arrival). A value of 0 means waiting forever, a negative
 * 	value causes just a single check with no blocking.
 *
 * @Returns: an event if one is available or %NULL on timeout. The returned
 * event must be destroyed using DC_destroyEvent() when it is no longer needed.
 */
DC_Event *DC_waitEvent(const char *wuFilter, int timeout);

/** Checks for events for a particular WU.
 *
 * @wu: specifies the work unit to watch events for.
 * @timeout: seconds to wait for an event to occur (result, subresult or
 * 	message arrival). A value of 0 means waiting forever, a negative
 * 	value causes just a single check with no blocking.
 *
 * @Returns: an event if one is available or %NULL on timeout. The returned
 * event must be destroyed using DC_destroyEvent() when it is no longer needed.
 */
DC_Event *DC_waitWUEvent(DC_Workunit *wu, int timeout);

/** Destroys an event.
 * @event: the event to destroy.
 */
void DC_destroyEvent(DC_Event *event);

/** Queries the number of WUs known to the API in the given state.
 * 
 * @state: The state the work unit must be in.
 *
 * @Returns: The number of work units in the given state or -1 if the state
 * is invalid.
 */
int DC_getWUNumber(DC_WUState state);

/********************************************************************
 * Function prototypes: Work unit management
 */

/** Creates one work unit.
 *
 * @clientName: name of the client application. Mapping this name to an actual
 * 	binary executable depends on the DC-API backend in use.
 * @arguments: NULL-terminated list of command line arguments.
 * @subresults: Number of sub-results the client may allowed to generate.
 * 	Specify 0 if the client will not send back sub-results.
 * @tag: arbitrary string to attach to this work unit. The value of this
 * 	string is not used by the DC-API, it is free for the application writer
 * 	to make use of it.
 *
 * @Returns: a handle for the work unit or %NULL if there was not enough
 * memory.
 */
DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag);

/** Sets an input file for the work unit.
 *
 * @wu: the work unit to operate on. Input files can be set only when the
 * 	workunit is not submitted yet.
 * @logicalFileName: the file name the client will use to refer to this
 * 	input file. The client should call the DC_resolveFilenName() function
 *	with the DC_FILE_IN file type and this name to get the real name of
 *	the file on the local system.
 * @URL: URL of the input file. May also be a local path on the master's
 * 	file system.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_addWUInput(DC_Workunit *wu, const char *logicalFileName, const char *URL,
	DC_FileMode fileMode);

/** Defines an output file for the work unit.
 *
 * @wu: the work unit to operate on. Output files can be defined only when the
 * 	workunit is not submitted yet.
 * @logicalFileName: the logical file name the client will use to refer to this
 * 	output file. The client should call the DC_resolveFilenName() function
 *	with the DC_FILE_OUT file type and this name to get the real name of
 *	the file on the local system.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName);

/** Sets the priority for the work unit.
 *  The priority should be less or equal than the priority of the application.
 *  XXX: What is the priority of the application?!?
 *
 * @wu: The work unit to operate on. The priority can be set only before
 * 	the work unit is submitted.
 * @priority: The priority of the work unit (XXX: What is the allowed range?!?)
 * 
 * @Returns: 0 if successful or an error code.
 */
int DC_setWUPriority(DC_Workunit *wu, int priority);

/** Serializes a work unit description.
 * This function creates an external representation of the work unit that may
 * be stored on persistent storage and can be used to re-create the WU
 * description.
 *
 * @wu: The work unit to serialize.
 * 
 * @Returns: An external representation of the WU. The caller should not assume
 * anything about the format of this representation. The returned value should
 * be de-allocated using free() when it is no longer needed. %NULL is returned
 * if the serialization fails.
 */
char *DC_serializeWU(DC_Workunit *wu);

/** Restores a serialized work unit.
 * This function restores a work unit from an external representation.
 *
 * @buf: The external representation as generated by DC_serialize().
 *
 * @Returns: A WU descriptor or %NULL if the de-serialization have failed.
 */
DC_Workunit *DC_deserializeWU(const char *buf);

/** Queries the state of a work unit.
 *
 * @wu: The work unit to examine.
 *
 * @Returns: The state of the work unit.
 */
DC_WUState DC_getWUState(DC_Workunit *wu);

/** Submits a work unit.
 *
 * @wu: The work unit to submit.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_submitWU(DC_Workunit *wu);

/** Queries the low-level ID of the work unit.
 *
 * @wu: A work unit. It must be in the DC_WU_RUNNING state when calling this
 * 	function.
 *
 * @Returns: An identifier used by the underlying grid infrastructure for this
 * work unit. This ID can be used to look up what happened to the WU in system
 * logs etc. If the WU is not in the DC_WU_RUNNING state, %NULL is returned.
 * The returned value must be deallocated using free() when it is no longer
 * needed.
 */
char *DC_getWUId(DC_Workunit *wu);

/** Queries the tag of a work unit.
 * @wu: a work unit.
 *
 * @Returns: the tag specified in the DC_createWU() call. The returned value
 * must be deallocated using free() when it is no longer needed.
 */
char *DC_getWUTag(DC_Workunit *wu);

/** Cancels all computations for a given work unit.
 *
 * @wu: The work unit to cancel.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_cancelWU(DC_Workunit *wu);

/** Temporarily suspends the execution of a work unit.
 * This works only if the underlying grid infrastructure supports
 * checkpointing.
 *
 * @wu: The work unit to suspend.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_suspendWU(DC_Workunit *wu);

/** Resumes computation of a previously suspended work unit.
 *
 * @wu: A work unit suspended by calling DC_suspendWU().
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_resumeWU(DC_Workunit *wu);

/** Releases internal resources allocated to a work unit.
 * Note that it does not stop the execution of the work unit if it is still
 * running; you should call DC_cancelWU() before DC_destroyWU() for that.
 *
 * @wu: The work unit descriptor to destroy.
 */
void DC_destroyWU(DC_Workunit *wu);

/** Sends a message to a running work unit.
 * Note that the message is sent asynchronously and this function does not
 * wait for the message to be delivered. Delivering the message may only
 * happen when the client checks for it which can be an arbitrary long time.
 *
 *
 * @wu: The work unit to send to. It must be in the DC_WU_RUNNING state.
 * @message: The message to send. The message may not be longer than
 * 	%DC_MAX_MESSAGE_LENGTH.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_sendWUMessage(DC_Workunit *wu, const char *message);

/********************************************************************
 * Function prototypes: Result handling
 */

/** Query what optional fields are present in the result.
 * 
 * @result: The result descriptor to examine.
 *
 * @Returns: zero or more of DC_GC_EXITCODE, DC_GC_STDOUT, DC_GC_STDERR and
 * DC_GC_LOG OR'ed together.
 */
unsigned DC_getResultCapabilities(DC_Result *result);

/** Returns the WU that generated this result.
 *
 * @result: A result descriptor.
 *
 * @Returns: The WU that generated this result, or %NULL if it cannot be
 * determined.
 */
DC_Workunit *DC_getResultWU(DC_Result *result);

/** Returns the exit code of the client application.
 *
 * @result: A result descriptor.
 *
 * @Returns: The client's exit code. If the DC_GC_EXITCODE capability is
 * not set for this result, then the returned value will always be 0.
 */
int DC_getResultExit(DC_Result *result);

/** Returns the local name of an output file.
 *
 * @result: A result descriptor.
 * @logicalFileName: File name on the client side, as it was given to
 * 	DC_addWUOutput() when the WU was created. The special constants
 * 	%DC_RESULT_STDOUT, %DC_RESULT_STDERR and %DC_RESULT_LOG can be used
 * 	to resolve the names of the client's stdout, stderr and log file,
 * 	respectively.
 *
 * @Returns: the resolved name of the file or %NULL if the clientfile
 * argument is invalid. The returned value should be deallocated using
 * free() when it is no longer needed.
 */
char *DC_getResultOutput(DC_Result *result, const char *logicalFileName);

#ifdef __cplusplus
}
#endif

#endif /* __DC_H_ */
