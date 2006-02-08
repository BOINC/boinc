/* API for Master-worker applications on Distributed Computing Platforms
   API for the Master program
   v0.2 Norbert Podhorszki
   MTA SZTAKI, 2004
*/

#ifndef __DC_H_
#define __DC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/syslog.h>
#include <stdarg.h>
#include <dc_common.h>

typedef enum {
	DC_RESULT_ACCEPT,
	DC_RESULT_INVALID,
	DC_RESULT_FINAL,
	DC_RESULT_SUB
} DC_ResultStatus;

/** The first function to be invoked by the main application before using DC 
 *  
 *  Return:  DC_OK on success
 *              >1 on error   (MORE DETAILS NEEDED!)
 *
 *  Configuration file is not specified yet but it should contain some information
 *  about the DC platform to be used, the download and upload data servers,
 *  authentication staff etc.
 */
int DC_init(
   const char *project_name,         // name of the project, if NULL application_name is used
   const char *application_name,     // name of the application
   const char *configfile            // path+name of DC configuration file to be processed
);

/* Workunit.
 * It describes one work unit to be computed on a computing client.
 */
typedef int DC_Workunit;


/* Result
   It describes one result for one work unit.
   In redundant computing there are more than one results for one work unit.
*/

#define MAX_OUTFILES 4
typedef struct {
    char *name;          // unique string ID of the Result
    DC_Workunit wu;           // workunit index in the WUtable
    char *outfiles_dir;  // directory of output files
    char *outfiles[MAX_OUTFILES];    // output files
    int  noutfiles;      // number of output files

    /* clgr-specific fields */
    char *std_out;
    char *std_err;
    char *sys_log;
    int exitcode;
    DC_ResultStatus status;
} dc_result;

/* Result.
 * It describes one result for one work unit.
 */
typedef dc_result* DC_Result;


/** Create one work unit
 *  Lets DC create a workunit. 
 *  Several functions should be called to set the parameters:
 *     - DC_setInput      to assign input data
 *     - DC_setPriority   to set priority
 *  It should be submitted after that.
 *
 *  Parameters:
 *      application Application client name
 *      arguments   Command line argument for the client
 *
 *  Return: DC_Workunit type.  This is the number of the WU.
 * 	    The functions bellow will need this DC_Workunit.
 *	    If the value of this DC_Workunit is minus, it means ERROR!
 * 	    If it is 0 or higher, its OK!
 */
DC_Workunit DC_createWU (const char *application_client,
		 const char *arguments);

/** Specify an input file for the work unit.
 *  Parameters:
 *  URL is either
 *     - a path to a file in master's file system or
 *     - an URL to a downloadable file 
 *  The input file will be renamed to 'localFileName' on the client node.
 *  I.e. client should use 'localFileName' to open the file 
 *  (or it should get the name as command line argument
 *
 *  Return: DC_OK
 */

int DC_setInput (DC_Workunit wu, char * URL, char * localFileName);


/** Set the priority for the work unit.
 *  The priority should be less or equal than the priority of the application.
 *
 *  Return: DC_OK
 */
int DC_setPriority (DC_Workunit wu, int priority);


/** Destroy allocated memory for a given work unit.
 *  Return:  DC_OK on success
 *              >1 on error
 */
int DC_destroyWU(DC_Workunit wu);


/** Submit work unit
 *  Lets DC to submit a workunit to a client machine.
 *  Return:  DC_OK on submission success 
 *              >1 on error
 */
int DC_submitWU (DC_Workunit wu);


/** Cancel all computations for a given work unit.
 *
 *  Return: DC_OK 
 */
int DC_cancelWU (DC_Workunit wu);


/** DC_suspend.
 *  It cancels the running of the given work unit.
 *  It is not deleted from the hdd, or from the memmory,
 *  only from the execution system.
 *
 *  return: DC_OK
 *
 *  Comment: This has no meaning on BOINC, so nothing will happen with the
 *           running client, but you will not get back the results even if
 *           the wu is finished.
 *           Even this feature is not implemented yet.
 */
int DC_suspendWU(DC_Workunit wu);


/** DC_resubmit.
 *  It restarts the given, suspended work unit.
 *
 *  return: DC_OK;
 *
 *  Comment: In BOINC based implementation, nothing happens to the client
 *           but from now on you can get back the results.
 *           Even this feature is not implemented yet.
 */
int DC_resubmitWU(DC_Workunit wu);

/** Check for results.
 *  Waits for available results and returns the first.
 *  Return:  DC_OK on success
 *              >1 on error
 *  Parameters:
 *     timeout    function blocks and waits until a result becomes available 
 *                but returns after the specified timeout (seconds).
 *                timeout = 0 means: waiting forever.
 *                timeout < 0 means: single check only, no blocking 
 *     func1      callback function for checking (accepting) a result
 *     func2      callback function for assimilating result within the application
 */
int DC_checkForResult(int  timeout,
		      //int  (*cb_check_result)(DC_Result result), 
		      void (*cb_assimilate_result)(DC_Result result)
		      );


/* Callback functions
   These functions should be provided by the application.

   Parameters:
     result     Result, memory allocated by the function using malloc.
                It will be deallocated automatically after the return 
                from assimilation.

   int cb_check_result( DC_Result result)

       Checking the result when it is available.
       Return DC_RESULT_ACCEPT  if result is accepted
              DC_RESULT_INVALID if result is invalid


   void cb_assimilate_result( DC_Result result)

       Assimilate the result within the application.
       On return, the result will be deleted within DC.

*/

void DC_log(int level, const char *fmt, ...);
void DC_vlog(int level, const char *fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* __DC_H_ */

