#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#include "dc.h"
#include "rm.h"

/* PRIVATE */

/* maximum number of work units */
#define MAX_N_WU 1024

/* States of the work unit.
 * In this implementation, submitted = running!
 * Running state is therefore not used below.
 */
#define STATE_INVALID   0
#define STATE_CREATED   1
#define STATE_SUBMITTED 2
#define STATE_RUNNING   3
#define STATE_FINISHED  4

static char *state_strings[5] = {"invalid", "created", "submitted", 
				 "running", "finished"};

#define MAX_N_WU 1024

typedef struct {
    char        *name;     /* name of the WU, wu0001, wu0002...*/
    char        *exe;      /* executable for the WU, full path please*/
    char        *args;     /* command line arguments for the WU */
    char        *workdir;  /* working dir of the WU, 
			      "specified workdir in config"+"wu name" */
    pid_t       pid;       /* pid of the forked process for this wu */
    int         state;     /* status */
} WUInfo;

static WUInfo wutable[MAX_N_WU];
static int wu_max, wu_sum;

static int dc_rm_findByName(char *wuname);
static int dc_rm_findByState(int state, int from);

/* PUBLIC */

void dc_rm_init(void)
{
    int i;
    for (i = 0; i < MAX_N_WU; i++) {
	wutable[i].name  = NULL;
	wutable[i].pid   = -1;
	wutable[i].state = STATE_INVALID;
    }
    wu_max = wu_sum = 0;
}

int dc_rm_wuCreate(const char *application_client,
		   const char *arguments, const char *workdir)
{
    int ndx = 0;
    int ex = 0;
    char wuname[32], dir[256];
    while (ndx < MAX_N_WU && !ex) {
	if (wutable[ndx].state == STATE_INVALID) ex = 1;
	else ndx++;
    }
    if (ndx == MAX_N_WU) {
	DC_log(LOG_ERR, "Too many work units (%d) at once\n",
	       MAX_N_WU);
	return -1;
    }
    if (ndx > wu_max) wu_max = ndx;
    wu_sum++;

    /*  wu name becomes "wu"+wuid, e.g wu0001 */
    snprintf(wuname, 32, "wu%4.4d", ndx);

    /* Create working directory for WU 
       <workdir>/<wu name>,
     */
    snprintf(dir, 256, "%s/%s", workdir, wuname);
    DC_log(LOG_INFO, "Create WU in %s\n", dir);
    if (mkdir(dir, S_IRWXU+S_IRGRP)) {
	if (errno != 17) { /* 17=directory exists */
	    DC_log(LOG_ERR, 
		   "Cannot create directory: %s\nerrno=%d  %s\n", 
		   dir, errno, strerror(errno));
	    return -1;
	}
	else {
	    /* errno 17: File exists
	       Here should be checked if 
	       - it is a directory and
	       - we can read/write into it
	    */
	}
    }
    wutable[ndx].name    = strdup(wuname);
    wutable[ndx].exe     = strdup(application_client);
    wutable[ndx].args    = strdup(arguments);
    wutable[ndx].workdir = strdup(dir);
    wutable[ndx].pid     = -1;
    wutable[ndx].state   = STATE_CREATED;

    return ndx;
}

int dc_rm_setInput (DC_Workunit wu, char * URL, char * localFileName)
{
    int ndx = (int)wu;
    char target[256], syscmd[512];
    if (ndx < 0 || ndx >= MAX_N_WU) {
	DC_log(LOG_ERR, "Invalid WU index at setInput: %d\n", ndx);
	return DC_ERROR;
    }
    if (wutable[ndx].state != STATE_CREATED) {
	DC_log(LOG_ERR, "setInput should be called only after wuCreate! wu=%d, status of this wu=%s\n", ndx, state_strings[wutable[ndx].state]);
	return DC_ERROR;
     }

    /* Copy input file into working directory */
    snprintf(target, 256, "%s/%s", wutable[ndx].workdir, localFileName);
    snprintf(syscmd, 1024, "cp %s %s", URL, target);
    system(syscmd);
    return DC_OK;
}

static char ** dc_rm_createArgList(char *arg)
{
    char *x, *y, *token, **args;
    int narg, i;
    const char delim[] = " ";
    x = strdup(arg);
    narg=0;
    token = strtok(x, delim);
    while (token != NULL) {
	narg++;
	token = strtok( NULL, delim);
    }
    DC_log(LOG_DEBUG, "ArgList: Number of args = %d\n", narg); 

    y = strdup(arg);
    args = (char**) malloc( narg * sizeof(char *));
    i = 0;
    token = strtok(y, delim);
    while (token != NULL) {
	args[i] = strdup(token);
	token = strtok(NULL, delim);
	i++;
    }

    for (i = 0; i < narg; i++) {
	DC_log(LOG_DEBUG, "Arg %d = %s\n", i, args[i]);
    }

    return args;
}

int dc_rm_submitWU (DC_Workunit wu)
{
    pid_t pid;
    int ndx = (int)wu;
    char ** args;

    if (ndx < 0 || ndx >= MAX_N_WU) {
	DC_log(LOG_ERR, "Invalid WU index at submitWU: %d\n", ndx);
	return DC_ERROR;
    }
    if (wutable[ndx].state != STATE_CREATED) {
	DC_log(LOG_ERR, "submitWU should be called only after wuCreate! wu=%d, status of this wu=%s\n", ndx, state_strings[wutable[ndx].state]);
	return DC_ERROR;
    }

    if((pid=fork())<0) {
	DC_log(LOG_ERR,"Cannot fork!\nerrno=%d  %s\n", 
	       errno, strerror(errno));
	return DC_ERROR;
    }

    if(pid==0) { /* client process */
      /* change into working directory of the WU */
      if (chdir(wutable[ndx].workdir)) {
	DC_log(LOG_ERR,"Cannot cd into %s\nerrno=%d  %s\n", 
	       wutable[ndx].workdir, errno, strerror(errno));
      }

      DC_log(LOG_DEBUG, "Convert args string into array of args: %s\n", 
	     wutable[ndx].args);
      args = dc_rm_createArgList(wutable[ndx].args);
      /* execute the client */
      DC_log(LOG_INFO, "Execute: %s %s\n", wutable[ndx].exe, wutable[ndx].args);
      execvp(wutable[ndx].exe,args);
      DC_log(LOG_ERR, "Cannot execute. Errno=%d  %s\n", 
	     errno, strerror(errno));
      exit(1);
    }
 
    wutable[ndx].pid = pid;
    wutable[ndx].state = STATE_SUBMITTED;
    return DC_OK;
}

int dc_rm_suspendWU(DC_Workunit wu)
{
    return 0;
}

int dc_rm_resubmitWU(DC_Workunit wu)
{
    return 0;
}

int dc_rm_destroyWU(DC_Workunit wu)
{
    int ndx = (int)wu;
    if (ndx < 0 || ndx >= MAX_N_WU) {
	DC_log(LOG_ERR, "Invalid WU index at destroyWU: %d\n", ndx);
	return DC_ERROR;
    }

    if (wutable[ndx].state != STATE_FINISHED) {
	DC_log(LOG_ERR, "destroyWU should be called only for finished or cancelled WUs! wu=%d, status of this wu=%s\n", ndx, state_strings[wutable[ndx].state]);
	return DC_ERROR;
    }

    free(wutable[ndx].name);
    free(wutable[ndx].exe);
    free(wutable[ndx].args);
    free(wutable[ndx].workdir);
    wutable[ndx].name    = NULL;
    wutable[ndx].exe     = NULL;
    wutable[ndx].args    = NULL;
    wutable[ndx].workdir = NULL;
    wutable[ndx].pid     = -1;
    wutable[ndx].state   = STATE_INVALID;
    return DC_OK;
}


int dc_rm_checkForResult(dc_result *result) 
{
    char cmd[256];
    char buf[256];
    int  retval;
    int  ndx = 0;

    ndx = dc_rm_findByState(STATE_SUBMITTED, ndx);
    while (ndx < MAX_N_WU) {
	snprintf(cmd, 256, "ps -p %d >/dev/null", wutable[ndx].pid);
	retval = system(cmd);

	/* retval == 0 means that the process is still in output of ps
	   but it can be <defunct>.
	   So check it again.
	*/
	if (retval == 0) {
	    snprintf(cmd, 256, "ps -p %d | grep defunct >/dev/null", 
		     wutable[ndx].pid);
	    retval = system(cmd);
	    if (retval == 0) retval = 1; /* defunct means finished */
	    else if (retval == 1) retval = 0;
	}
	if (retval == 1) { /* process finished (not exists) */
	    /* create the result object */
	    snprintf(buf, 256, "result%4.4d", ndx);
	    result->name = strdup(buf);
	    result->wu = (DC_Workunit) ndx;
	    result->outfiles_dir = wutable[ndx].workdir;
	    result->outfiles[0] = NULL;
	    result->noutfiles = 0;
	    
	    DC_log(LOG_INFO, 
		   "Work unit %s with pid %d is found to be finished\n",
		   wutable[ndx].name, wutable[ndx].pid);

	    wutable[ndx].state = STATE_FINISHED;

	    return DC_RM_RESULTEXISTS;
	}
	else if (retval < 0) { /* error */
	    return DC_ERROR;
	}


	ndx = dc_rm_findByState(STATE_SUBMITTED, ndx+1);
    }
    return DC_RM_NORESULT;
}

void dc_rm_freeResult(dc_result result)
{
    free(result.name);
    result.name = NULL;
}

void dc_rm_log(void)
{
    int i;
    DC_log(LOG_INFO, "----------------------------------------\n");
    DC_log(LOG_INFO, "DC has the following info about WUs\n");
    DC_log(LOG_INFO, "Number of workunits = %d\n", 
	   wu_sum);
    DC_log(LOG_INFO, "Table info: wu_max = %d, wu_sum = %d\n",
	   wu_max, wu_sum);
    DC_log(LOG_INFO, "Table list:\n");
    DC_log(LOG_INFO, "Name        State           PID     WorkDir\n");
    for (i = 0; i <= wu_max; i++) {
	DC_log(LOG_INFO, "%s\t%s\t%d\t%s\n",
	       wutable[i].name, state_strings[wutable[i].state], 
	       wutable[i].pid, wutable[i].workdir);
    }
    DC_log(LOG_INFO, "-----------------------------------------\n");
}

static int dc_rm_findByName(char *wuname)
{
   int ndx = 0;
   int ex = 0;
   while (ndx < MAX_N_WU && !ex) {
       if (wutable[ndx].name != NULL &&
	   !strcmp(wutable[ndx].name, wuname)) ex = 1;
       else ndx++;
   }
   return ndx;
}

static int dc_rm_findByState(int state, int from)
{
   int ndx = from;
   int ex = 0;
   while (ndx < MAX_N_WU && !ex) {
       if (wutable[ndx].state == state) ex = 1;
       else ndx++;
   }
   return ndx;
}
