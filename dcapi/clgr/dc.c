#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "dc.h"
#include "cfg.h"
#include "wu.h"
//#include "assimilator.h"
#include "result.h"

static char *projectname;         /* project name under boinc */
static char *appname;             /* name of the application, used here in assimilation */
static char *workdir;             /* a working dir for this application */
static char *executabledir;          /* the executable file, with path */
//static char *boincprojectrootdir; /* where projects are stored, e.g. /usr/boinc/projects */
//static char *wutemplate;          /* path and file name */
//static char *resulttemplate;      /* path and file name */
//static char *uploadkeyfile;       /* upload private key file */
static char yesno[2][4] = {"no", "yes"};

static char *sleepinterval;

/* CONSTRAINTS

   1. One application client is supported (because of assimilation)
      Client name is defined for each WU separately in the API
      At DC_init, this one client name should be given as application_name.

   2. One workunit and one result templates are supported, given in the config file.

*/
int DC_init(const char *project_name, const char *application_name, const char *configfile)
{
    char buf[512];  //, syscmd[2048];
    time_t ltime;

    projectname = strdup(project_name);
    appname     = strdup(application_name);

    if (dc_cfg_parse(configfile) != DC_CFG_OK) return DC_ERROR;
    workdir    = dc_cfg_get("WorkingDirectory");
    executabledir = dc_cfg_get("Executabledir");
    //boincprojectrootdir = dc_cfg_get("BoincProjectRootDirectory");
    //wutemplate = dc_cfg_get("WUTemplatePath");
    //resulttemplate = dc_cfg_get("ResultTemplatePath");
    //uploadkeyfile    = dc_cfg_get("UploadPrivateKey");
    sleepinterval    = dc_cfg_get("CheckResultSleepInterval");

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
/*
    if (boincprojectrootdir == NULL) {
	DC_log(LOG_ERR, 
	       "BOINC project root directory cannot be determined from the config file");
	return DC_ERROR;
    }

    if (wutemplate == NULL) {
	DC_log(LOG_ERR, 
	       "Template file for Workunit cannot be determined from the config file");
	return DC_ERROR;
    }

    if (resulttemplate == NULL) {
	DC_log(LOG_ERR, 
	       "Template file for Result cannot be determined from the config file");
	return DC_ERROR;
    }

    if (uploadkeyfile == NULL) {
	DC_log(LOG_ERR, 
	       "Upload private key file  cannot be determined from the config file");
	return DC_ERROR;
    }
*/
    if (sleepinterval == NULL) {
	DC_log(LOG_ERR, 
	    "Check for Result sleeping interval cannot be determined from the config file\nDefault value is 5 sec.");
	strcpy(sleepinterval,"5");
	//return DC_ERROR;
    }



    DC_log(LOG_DEBUG, "init: WorkingDir=%s", workdir);
    DC_log(LOG_DEBUG, "init: Executabledir=%s", executabledir);
//    DC_log(LOG_DEBUG, "init: Boinc Project Root Dir=%s", boincprojectrootdir);
//    DC_log(LOG_DEBUG, "init: Template WU=%s", wutemplate);
//    DC_log(LOG_DEBUG, "init: Template Result=%s", resulttemplate);
//    DC_log(LOG_DEBUG, "init: Upload private key file=%s", uploadkeyfile);
    DC_log(LOG_DEBUG, "init: Check Result sleeping interval=%s", sleepinterval);
 

    /* Check config.xml in project's directory */
/*    snprintf(buf, 512, "%s/%s/config.xml", boincprojectrootdir, project_name);
    if (access(buf, R_OK)) {
	DC_log(LOG_ERR, 
	       "Invalid project name, not valid configuration found: %s, error = %d %s", 
	       buf, errno, strerror(errno));
	return DC_ERROR;
    }
*/
    /* Copy template files into project's template/ dir. */
/*    snprintf(buf, 512, "%s/%s/templates", boincprojectrootdir, project_name);
    snprintf(syscmd, 1024, "cp %s %s", resulttemplate, buf);
    if (system(syscmd) == -1) {
	DC_log(LOG_ERR, 
	       "Cannot copy result template files into project's template/ dir.\n"
	       "Command '%s' failed.", syscmd);
	return DC_ERROR;
    }
*/
//    snprintf(buf, 512, "%s/%s", boincprojectrootdir, project_name);
//    dc_wu_init(buf, uploadkeyfile);  /* init WU manager */
    snprintf(buf, 512, "%s_%s_%ld", project_name, appname, time(&ltime));

    dc_wu_init(workdir, buf, executabledir);
    
    return DC_OK;
}


DC_Workunit DC_createWU (const char *application_client,
        	         const char *arguments)
{
//    char dir[256];


    /* char inpf[256], syscmd[1024]; */

    /* Create working directory for WU 
       <workdir>/<wu name>
     */
/*
    snprintf(dir, 256, "%s/%s/%d", workdir, projectname, wundx);
    DC_log(LOG_DEBUG, "Create WU in %s", dir);
    if (mkdir(dir, S_IRWXU+S_IRGRP)) {
	if (errno != 17) { 				// 17=directory exists 
	    DC_log(LOG_ERR, 
		   "Cannot create directory %s: %s", 
		   dir, strerror(errno));
	    return -1;
	}
	else {

	    //	 errno 17: File exists
	    //   Here should be checked if 
	    //   - it is a directory and
	    //   - we can read/write into it
	    //
	}
    }
*/

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
    //char boincRootDir[256]; 
    //snprintf(boincRootDir, 512, "%s/%s", boincprojectrootdir, projectname);
    //return dc_wu_createBoincWU(wu, uploadkeyfile, boincRootDir);
    return dc_wu_submitWU(wu);
}

int DC_cancelWU (DC_Workunit wu)
{
    printf("DC_cancelWU is not implemented yet");
    return 1;
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
//    static int initialized = 0;
    //char buf[512];

    if (sleeptime < 1) sleeptime = 1;

    DC_log(LOG_INFO, "Check for results, blocking=%s, sleep_interval=%d", 
	   yesno[timeout>=0], sleeptime);

/*
    if (!initialized) {
	snprintf(buf, 512, "%s/%s", boincprojectrootdir, projectname);
	dc_assimilator_init(buf, appname); // init assimilator 
	initialized = 1;
    }
*/

    while (!ex) {
	ex = 1;
	DC_log(LOG_DEBUG, "Call dc_assimilator_dopass()");
	//retval = dc_assimilator_dopass(cb_assimilate_result);
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



