#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <uuid/uuid.h>

#include <dc.h>
#include <dc_internal.h>

#include "wu.h"
#include "assimilator.h"
#include "result.h"

static char *appname;             /* name of the application, used here in assimilation */
static char *workdir;             /* a working dir for this application */
static char *boincprojectrootdir; /* where projects are stored, e.g. /usr/boinc/projects */
static char *wutemplate;          /* path and file name */
static char *resulttemplate;      /* path and file name */
static char *uploadkeyfile;       /* upload private key file */
static char yesno[2][4] = {"no", "yes"};

static char *sleepinterval;

/* CONSTRAINTS

   1. One application client is supported (because of assimilation)
      Client name is defined for each WU separately in the API
      At DC_init, this one client name should be given as application_name.

   2. One workunit and one result templates are supported, given in the config file.

*/
int DC_init(const char *project_name __attribute__((unused)), const char *application_name,
	const char *configfile)
{
    char buf[512], syscmd[2048];

    appname     = strdup(application_name);

    if (_DC_parseCfg(configfile)) return DC_ERROR;
    workdir    = _DC_getCfgStr("WorkingDirectory");
    boincprojectrootdir = _DC_getCfgStr("BoincProjectRootDirectory");
    wutemplate = _DC_getCfgStr("WUTemplatePath");
    resulttemplate = _DC_getCfgStr("ResultTemplatePath");
    uploadkeyfile    = _DC_getCfgStr("UploadPrivateKey");
    sleepinterval    = _DC_getCfgStr("CheckResultSleepInterval");

    if (workdir == NULL) {
	DC_log(LOG_ERR, 
	       "Working directory cannot be determined from the config file");
	return DC_ERROR;
    }

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

    if (sleepinterval == NULL) {
	DC_log(LOG_ERR, 
	    "Check for Result sleeping interval cannot be determined from the config file\nDefault value is 5 sec.");
	strcpy(sleepinterval,"5");
	//return DC_ERROR;
    }



    DC_log(LOG_DEBUG, "init: WorkingDir=%s", workdir);
    DC_log(LOG_DEBUG, "init: Boinc Project Root Dir=%s", boincprojectrootdir);
    DC_log(LOG_DEBUG, "init: Template WU=%s", wutemplate);
    DC_log(LOG_DEBUG, "init: Template Result=%s", resulttemplate);
    DC_log(LOG_DEBUG, "init: Upload private key file=%s", uploadkeyfile);
    DC_log(LOG_DEBUG, "init: Check Result sleeping interval=%s", sleepinterval);
 
    /* Boinc daemons are run from the <projectroot>/bin directory. Change to
     * the working directory so the master can find its input files */
    if (chdir(workdir)) {
	DC_log(LOG_ERR,
		"Failed to switch to the working directory %s: %s",
		workdir, strerror(errno));
	return DC_ERROR;
    }

    /* Check config.xml in project's directory */
    snprintf(buf, 512, "%s/config.xml", boincprojectrootdir);
    if (access(buf, R_OK)) {
	DC_log(LOG_ERR, 
	       "Invalid project name, not valid configuration found: %s, error = %d %s", 
	       buf, errno, strerror(errno));
	return DC_ERROR;
    }

    /* Copy template files into project's template/ dir. */
    snprintf(buf, 512, "%s/templates", boincprojectrootdir);
    snprintf(syscmd, 1024, "cp %s %s", resulttemplate, buf);
    if (system(syscmd) == -1) {
	DC_log(LOG_ERR, 
	       "Cannot copy result template files into project's template/ dir.\n"
	       "Command '%s' failed.", syscmd);
	return DC_ERROR;
    }

    snprintf(buf, 512, "%s", boincprojectrootdir);
    dc_wu_init(buf, uploadkeyfile);  /* init WU manager */
    return DC_OK;
}


DC_Workunit DC_createWU (const char *application_client,
		 const char *arguments)
{
    char dir[256];

    uuid_t id;
    char str_id[37];
    uuid_generate(id);
    uuid_unparse(id, &str_id[0]);


    const char *wu_name = strdup(str_id);

    /* char inpf[256], syscmd[1024]; */

    /* Create working directory for WU 
       <workdir>/<wu name>

	Modification by PN, 2005-07-14:
	  As we put all input files into the project's download dir and the
	 output file is located in the project's upload dir, we do not need
	 any other new directory (it is empty).
	 Two options: either we do not create it here, or it should be removed
	   in the function dc_wu_destroy()
	 I have chosen the first option: let the working dir for all wu the same,
	 i.e. the 'workdir' itself without appending /'wu_name'.
     */

    strncpy(dir, workdir, 256);
    DC_log(LOG_DEBUG, "Create WU in %s", dir);
    if (mkdir(dir, S_IRWXU+S_IRGRP)) {
	if (errno != 17) { /* 17=directory exists */
	    DC_log(LOG_ERR, 
		   "Cannot create directory %s: %s\n", 
		   dir, strerror(errno));
	    return 0;
	}
	else {
	    /* errno 17: File exists
	       Here should be checked if 
	       - it is a directory and
	       - we can read/write into it
	    */
	}
    }


    return  dc_wu_create(wu_name, application_client, arguments, 
			dir, wutemplate, resulttemplate);


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
    char boincRootDir[256]; 
    snprintf(boincRootDir, 512, "%s", boincprojectrootdir);
    return dc_wu_createBoincWU(wu, uploadkeyfile, boincRootDir);
}

int DC_cancelWU (DC_Workunit wu)
{
    //printf("DC_cancelWU is not implemented yet\n");
    
    return dc_wu_destroy(wu);
}

int DC_suspendWU(DC_Workunit wu)
{
    DC_log(LOG_WARNING, "DC_suspendWU is not implemented yet\n");
    return DC_OK;
}

int DC_resubmitWU(DC_Workunit wu)
{
    DC_log(LOG_WARNING, "DC_resubmitWU is not implemented yet\n");
    return DC_OK;
}

int DC_destroyWU(DC_Workunit wu)
{
    return dc_wu_destroy(wu);
}

int DC_checkForResult(int  timeout,
		      void (*cb_assimilate_result)(DC_Result result)
		      ) 
{
    int retval;
    int ex = 0;
    int sleeptime = atoi(sleepinterval);
    time_t tstart = time(NULL);
    static int initialized = 0;
    char buf[512];

    if (sleeptime < 1) sleeptime = 1;

    DC_log(LOG_INFO, "Check for results, blocking=%s, sleep_interval=%d", 
	   yesno[timeout>=0], sleeptime);


    if (!initialized) {
	snprintf(buf, 512, "%s", boincprojectrootdir);
	dc_assimilator_init(buf, appname); /* init assimilator */
	initialized = 1;
    }


    while (!ex) {
	ex = 1;
	DC_log(LOG_DEBUG, "Call dc_assimilator_dopass()");
	retval = dc_assimilator_dopass(cb_assimilate_result);
	if (retval == 0 && timeout == 0) /* no result and waiting forever */
	    ex = 0;
	if (retval == 0 &&                /* no result found yet  */
	    timeout > 0 &&                /* blocking mode        */
	    time(NULL) < tstart+timeout   /* timeout (>0) not exceeded */
	    ) ex = 0;
	if (!ex) sleep(sleeptime);
    }
    return DC_OK;
}



