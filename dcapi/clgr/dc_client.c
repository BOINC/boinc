#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "dc_client.h"
#include "logger.c"

static time_t result_time = 0;
static time_t ckpt_time = 0;
static char statfilename[] = "job_out_stats.dat";


/* Print out result_time and ckpt_time in the format "%d %d" 
 * into the file 'statfilename'
 * ClusterGrid infrastructure takes care of delivering information
 * up to the information system so that job status query results in
 *   subresult_time = <time>
 *   ckpt_time = <time>
 */

static int dc_client_writeout(void)
{
   FILE *out;
   int retval = DC_OK;
   if ( (out = fopen(statfilename, "w")) != NULL) {
      fprintf(out, "%d %d\n", (int) result_time, (int)ckpt_time);
      fclose(out);
   }
   else {
      DC_log(LOG_ERR, "dc-client: Cannot open file %s", statfilename);
      retval = DC_ERROR;
   }
   return retval;

}

/** Notify master (somehow) that a subresult is provided 
 */ 
int DC_SendResult(char **files,  // output files
		  int nfiles     // number of output files
		  )
{
   result_time = time(NULL);
   return dc_client_writeout();
}


/** Notify master (somehow) that a user-level checkpoint is provided 
 */ 
int DC_CheckpointMade(void)
{
  ckpt_time = time(NULL);
  return dc_client_writeout();
}

/** Finalize client API if needed.
 */ 
void DC_Finish(int exitcode)
{
}

