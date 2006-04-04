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
} DC_Filetype;

/* Special handle for the checkpoint file */
#define DC_CHECKPOINT_FILE	"__DC_CHECKPOINT_"

/* Control events */
typedef enum {
	DC_EVENT_NONE,
	DC_EVENT_DO_CHECKPOINT,
	DC_EVENT_FINISH,
	DC_EVENT_MESSAGE
} DC_Event;


/********************************************************************
 * Function prototypes
 */

/** Initializes the client API.
 *
 * @Returns: 0 if succesful or an error code.
 */
int DC_init(void);

/** Resolves the local name of input/output files.
 * The real name (and path) of an input/output file may be different from what
 * the application expects. It should ask the API about the actual path.
 * It may want to open input file 'inp.txt' in the actual directory but
 * the actual file may be e.g. '../project/szdg_lpds_sztaki_hu_szdg/inp.txt09247814354'
 *
 * @type: The type of the file (input/output etc.)
 * @logicalFileName: the logical name of the file. It must be the same as the
 * 	master used for the DC_addWUInput() and DC_addWUOutput() functions.
 *
 * @Returns: the local path of the file. The value should be deallocated using
 * free(3) when it is no longer needed. %NULL is returned if @type is invalid
 * or @logicalFileName is not known to the infrastructure.
 */
char *DC_resolveFileName(DC_Filetype type, const char *logicalFileName);

/** Sends a sub-result back to the master.
 * Note: sending back the result happens asyncronously, and this function does
 * not wait for the trasfer to finish. Therefore it is not allowed to change
 * the local file after this function has been called. The local file will be
 * deleted when the transfer is complete.
 *
 * @logicalFileName: A logical identifier of the file that will be received
 * 	by the master. You may specify %NULL if you do not want to send one.
 * @path: The local path of the file to send.
 * @copyMode: how to handle the file. Possible values are %DC_COPY_DEFAULT for
 * 	making a copy while keeping the original, and %DC_COPY_RENAME to
 * 	remove the original.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_sendResult(const char *logicalFileName, const char *path,
	DC_FileMode fileMode);

/** Sends a message to the master.
 * Note: sending of the message happens asynchronously and there may be
 * a long delay before it actually takes place.
 *
 * @message: The message to send. It may not be larger than
 * 	%DC_MAX_MESSAGE_LENGTH.
 *
 * @Returns: 0 if successful or an error code.
 */
int DC_sendMessage(const char *message);

/** Checks for application-control events.
 * This function should be called in a loop until it returns %DC_EVENT_NONE.
 *
 * @data: reserved for future use.
 *
 * @Returns: the event code.
 */
DC_Event DC_checkEvent(void **data);

/** Destroys the event-specific data returned by DC_checkEvent()
 *
 * @event: the received event
 * @data: the data returned by DC_checkEvent()
 */
void DC_destroyEvent(DC_Event event, void *data);

/** Indicates that an application-level checkpoint has completed.
 *
 * @fileName: the name of the checkpoint file. This should be the value
 * 	returned by the DC_resolveFileName(%DC_FILE_OUT, %DC_CHECKPOINT_FILE)
 * 	function call. The checkpoint file must not be modified after this
 * 	function returns.
 */
void DC_checkpointMade(const char *fileName);

/** Informs the user interface about the fraction of work already done.
 *
 * @fraction: The fraction of the work completed.
 */
void DC_fractionDone(double fraction);

/** Finishes computation.
 * Tells the DC-API to finish this work unit and start a new one.
 * The client should called it as the last function and then exit.
 *
 * Note: it is strongly recommended to pass the same exitcode to this
 * function as will be passed to the real exit(3) because there is no
 * guarantee which of the two will really be reported to the master.
 *
 * @exitcode: The exit code to return back to the master.
 */
void DC_finish(int exitcode);

#ifdef __cplusplus
}
#endif

#endif
