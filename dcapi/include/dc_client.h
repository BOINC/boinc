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

/* For return value */
#define DC_OK	 0
#define DC_ERROR 1

/* For the DC_ResolveFileName function */
#define DC_FILE_IN	0
#define DC_FILE_OUT	1
#define DC_FILE_CKPT	2
#define DC_FILE_TMP	3


/** Inicialize the client API.
 *  Some kind of init is usually required in most of the GRIDs!
 *
 *  Return: DC_OK
 */
int DC_Init(void);


/** File name resolution
 * The real file name (and path) of an input/output file may be different from what
 * the application expects. It should ask the API about the actual path.
 * It may want to open input file 'inp.txt' in the actual directory but
 * the actual file may be e.g. '../project/szdg_lpds_sztaki_hu_szdg/inp.txt09247814354'
 *
 * The first "type" parameter decides what is the type of the requested file:
 * --> input: 0, output: 1, ckpt: 2, tmp: 3
 *
 * Usage:
 *   char fname[256];
 *   dc_ResolveFileName(INPUT_FILE, "inp.txt", fname, 256);
 *   f = fopen(fname);
 *
 * Return: DC_OK always
 */
int DC_ResolveFileName(
    int type,
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
void DC_Finish(void);

#ifdef __cplusplus
}
#endif

#endif
