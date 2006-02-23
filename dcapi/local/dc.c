#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include <dc.h>
#include <dc_internal.h>

#include "rm.h"

static char *projectname;
static char *appname;
static char *workdir;
static char *sleepinterval;

/* CHEAT 
static char workdir_fixed[] = "/home/pnorbert/SETI/dcapi/examples/workdir";
*/
int DC_init(const char *project_name, 
	    const char *application_name, 
	    const char *configfile)
{
    projectname = strdup(project_name);
    appname     = strdup(application_name);

    if (_DC_parseCfg(configfile)) return DC_ERROR;

    workdir    = _DC_getCfgStr("WorkingDirectory");
    if (workdir == NULL) {
	DC_log(LOG_ERR, 
	       "Working directory cannot be determined from the config file\n");
	return DC_ERROR;
    }
    DC_log(LOG_INFO, "init: WorkingDir=%s\n", workdir);

    sleepinterval    = _DC_getCfgStr("CheckResultSleepInterval");
    if (sleepinterval == NULL) {
        DC_log(LOG_WARNING,
            "Check for Result sleeping interval cannot be determined from the config file\nDefault value is 5 sec.");
        strcpy(sleepinterval,"5");
    }
    DC_log(LOG_INFO, "init: Sleeping interval when checking for results=%s\n", 
	   sleepinterval);


    /* CHEAT 
    workdir    = workdir_fixed;
    */

    dc_rm_init();  /* init resource manager */
    return DC_OK;
}


int DC_createWU (const char *application_client,
		 const char *arguments)
{
    return dc_rm_wuCreate(application_client, arguments, workdir);
}

int DC_setInput (DC_Workunit wu, char * URL, char * localFileName)
{
    return dc_rm_setInput(wu, URL, localFileName);
}


int DC_submitWU (DC_Workunit wu)
{
    return dc_rm_submitWU(wu);
}


int DC_cancelWU (DC_Workunit wu)
{
    printf("DC_cancelWU is not implemented yet\n");
    return 1;
}

int DC_suspendWU (DC_Workunit wu)
{
    return dc_rm_suspendWU(wu);
}

int DC_resubmitWU(DC_Workunit wu)
{
    return dc_rm_resubmitWU(wu);
}

int DC_destroyWU(DC_Workunit wu)
{
    dc_rm_destroyWU(wu);
    return 0;
}

int DC_checkForResult(int  timeout,
		      void cb_assimilate_result(DC_Result result)
		      ) 
{
    dc_result result;
    int retval;
    int ex = 0;
    int sleeptime = atoi(sleepinterval);
    time_t tstart = time(NULL);

    if (sleeptime < 1) sleeptime = 1;
 

    while (!ex) {
	ex = 1;
	retval = dc_rm_checkForResult(&result);
	if (retval == DC_RM_RESULTEXISTS) {

	    /*if (cb_check_result(result) == DC_RESULT_ACCEPT)*/
		cb_assimilate_result((DC_Result)&result);

            /* rmdir Does not work because dir should be empty first */
            /*
	    if (rmdir(result->outfiles_dir)) {
		DC_log(LOG_ERR, 
		       "Cannot delete directory: %s\nerrno=%d  %s\n", 
		       result->outfiles_dir, errno, strerror(errno));
		return DC_ERROR;
	    }
            */

		dc_rm_freeResult(result);

	}
	else if (retval == DC_RM_NORESULT) {
	    if (timeout==0) ex = 0;  /* stay in loop, blocking */
            if (timeout > 0 &&                /* blocking mode        */
		time(NULL) < tstart+timeout   /* timeout (>0) not exceeded */
		) 
		ex = 0;
	}
	else if (retval == DC_ERROR) {
	    return DC_ERROR;
	}
	if (!ex) sleep(sleeptime);
    }
    return DC_OK;
}



