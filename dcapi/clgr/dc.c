#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <dc.h>
#include <dc_internal.h>

#include "wu.h"
#include "result.h"

static char *projectname;         /* project name under boinc */
static char *appname;             /* name of the application, used here in assimilation */
static char *workdir;             /* a working dir for this application */
static char *executabledir;          /* the executable file, with path */
static char yesno[2][4] = {"no", "yes"};

static char *sleepinterval;

int DC_init(const char *project_name, const char *application_name, const char *configfile)
{
    char buf[512];
    time_t ltime;

    projectname = strdup(project_name);
    appname     = strdup(application_name);

    if (_DC_parseCfg(configfile)) return DC_ERROR;
    workdir    = DC_getCfgStr("WorkingDirectory");
    executabledir = DC_getCfgStr("Executabledir");
    sleepinterval    = DC_getCfgStr("CheckResultSleepInterval");

    if (workdir == NULL) {
	DC_log(LOG_ERR,
	       "Working directory cannot be determined from the config file");
	return DC_ERROR;
    }

    if (executabledir == NULL) {
        DC_log(LOG_ERR,
	       "Executable file cannot be determined from the config file");
        return DC_ERROR;
    }
    if (sleepinterval == NULL) {
	DC_log(LOG_ERR,
	    "Check for Result sleeping interval cannot be determined from the config file");
	DC_log(LOG_ERR,
	    "Default value is 5 sec.");
	sleepinterval = "5";
    }

    DC_log(LOG_DEBUG, "init: WorkingDir=%s", workdir);
    DC_log(LOG_DEBUG, "init: Executabledir=%s", executabledir);
    DC_log(LOG_DEBUG, "init: Check Result sleeping interval=%s", sleepinterval);

    snprintf(buf, 512, "%s_%s_%ld", project_name, appname, time(&ltime));

    dc_wu_init(workdir, buf, executabledir);

    return DC_OK;
}

DC_Workunit DC_createWU (const char *application_client,
        	         const char *arguments)
{
    return dc_wu_create(application_client, arguments);
}

int DC_setInput (DC_Workunit wu, char * URL, char * localFileName)
{
    return dc_wu_setInput(wu, URL, localFileName);
}


int DC_setPriority (DC_Workunit wu, int priority)
{
    return dc_wu_setPriority(wu, priority);
}

int DC_submitWU (DC_Workunit wu)
{
    return dc_wu_submitWU(wu);
}

int DC_cancelWU (DC_Workunit wu)
{
    return dc_wu_cancel(wu);
}

int DC_destroyWU(DC_Workunit wu)
{
    return dc_wu_destroy(wu);
}

int DC_suspendWU(DC_Workunit wu)
{
    return dc_wu_suspend(wu);
}

int DC_resubmitWU(DC_Workunit wu)
{
    return dc_wu_resubmit(wu);
}

int DC_checkForResult(int  timeout,
		      void (*cb_assimilate_result)(DC_Result result)
		      )
{
    int retval;
    int ex = 0;
    int sleeptime = atoi(sleepinterval);
    time_t tstart = time(NULL);

    if (sleeptime < 1) sleeptime = 1;

    DC_log(LOG_INFO, "Check for results, blocking=%s, sleep_interval=%d",
	   yesno[timeout>=0], sleeptime);

    while (!ex) {
	ex = 1;
	DC_log(LOG_DEBUG, "Call dc_assimilator_dopass()");
	retval = dc_wu_checkForResult(cb_assimilate_result);
	if (retval == 0 && timeout == 0)  /* no result and waiting forever */
	    ex = 0;
	if (retval == 0 &&                /* no result found yet  */
	    timeout > 0 &&                /* blocking mode        */
	    time(NULL) < tstart+timeout   /* timeout (>0) not exceeded */
	    ) ex = 0;
	if (!ex) sleep(sleeptime);
    }
    if (retval == -1) return DC_ERROR;
    else return DC_OK;
}
