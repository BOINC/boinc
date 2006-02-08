/* API for Master-worker applications on Distributed Computing Platforms
   API for the Client program
   v0.4 Norbert Podhorszki, and Gabor Vida
   MTA SZTAKI, 2006
*/
#ifndef __DCCLIENT_H_
#define __DCCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dc_common.h>

/* For the DC_ResolveFileName function */
typedef enum {
	DC_FILE_IN,
	DC_FILE_OUT,
	DC_FILE_CKPT,
	DC_FILE_TMP
} DC_Filetype;


/** Inicialize the client API.
 *  Some kind of init is usually required in most of the GRIDs!
 *
 *  Return: DC_OK
 */
int DC_Init(void);

/** File name resolution
 * The real name (and path) of an input/output file may be different from what
 * the application expects. It should ask the API about the actual path.
 * It may want to open input file 'inp.txt' in the actual directory but
 * the actual file may be e.g. '../project/szdg_lpds_sztaki_hu_szdg/inp.txt09247814354'
 *
 * The first "type" parameter decides what is the type of the requested file.
 *
 * Usage:
 *   char fname[256];
 *   DC_ResolveFileName(DC_FILE_IN, "inp.txt", fname, sizeof(fname));
 *   f = fopen(fname);
 *
 * Return: DC_OK always
 */
int DC_ResolveFileName(
    DC_Filetype type,
    const char *requestedFileName,
    char       *actualFileName,
    int maxlength
);

/** Send a (partial) result back to the master.
 *  Parameters:
 *      files    list of created output files to be sent back
 *      nfiles   number of files
 *
 *  Return: DC_OK
 */
int DC_SendResult(char **files,
		  int nfiles
		  );

/** Is it time to make a checkpoint?
 *  Return: 0 No, 1 Yes
 *
 *  Comment: fast function, it can be called very regularly.
 *           BOINC provides such functionality, for ClusterGrid,
 *           the api answers Yes every 5 minutes.
 */
int DC_TimeToCheckpoint(void);

/** User/level checkpoint made.
 *  Return: DC_OK
 */
int DC_CheckpointMade(void);


/** Check if work can be continued. (Ask master)
 *  This involves network communication and may block long.
 *  Parameters:
 *      timeout  Return after a given timeout (msec). 
 *               timeout=0: wait until answer can be acquired.
 *  
 *  Return: 0   No, finish
 *          1   Yes, continue (or after timeout)
 */
int DC_ContinueWork(void);


/** Informs about the fraction already done!
 *  Parameter:
 *       fraction - a double value of   work already done / total work
 *
 *  Return: DC_OK
 */
int DC_FractionDone(double fraction);


/** Finish computation.
 *  Tell DC to finish this work unit and start a new one.
 *  It should be called as the last function and then exit.
 */
void DC_Finish(int exitcode);

#ifdef __cplusplus
}
#endif

#endif
