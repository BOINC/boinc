/* API for Master-worker applications on Distributed Computing Platforms
   API for the Client program
   v0.2 Norbert Podhorszki
   MTA SZTAKI, 2004
*/
#ifndef __DCCLIENT_H_
#define __DCCLIENT_H_

#define DC_OK    0
#define DC_ERROR 1

/** Send a (partial) result back to the master.
 *  Parameters:
 *      files    list of created output files to be sent back
 *      nfiles   number of files
 *
 *  Return: DC_OK
 */
int DC_SendResult(char **files,  // output files
		  int nfiles     // number of output files
		  );

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


/** Finish computation.
 *  Tell DC to finish this work unit and start a new one.
 *  It should be called as the last function and then exit.
 */
void DC_Finish(void);

#endif
