#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <dc_client.h>
#include <logger.c>

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

/** Reolves a filename needed by the client application!
 */
int DC_ResolveFileName(DC_Filetype type,
		       const char *requestedFileName,
		       char *actualFileName,
		       int maxlength
		       )
{
    switch (type)
    {
	case DC_FILE_IN:
	    snprintf(actualFileName, maxlength, "./input/%s", requestedFileName);
	    break;
	case DC_FILE_OUT:
	    snprintf(actualFileName, maxlength, "./output/%s", requestedFileName);
	    break;
	case DC_FILE_CKPT:
	    snprintf(actualFileName, maxlength, "./ckpt/%s", requestedFileName);
	    break;
	case DC_FILE_TMP:
	    snprintf(actualFileName, maxlength, "./tmp/%s", requestedFileName);
	    break;
	default:
	    return DC_ERROR;
    }
    return DC_OK;
}

/** Finalize client API if needed.
 */ 
void DC_Finish(int exitcode)
{
}

