#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#include "dc.h"
#include "wu.h"
#include "result.h"
#include "defines.h"

#include "submit.h"
#include "status.h"
#include "get_out.h"
#include "remove.h"

/* PRIVATE */

/* maximum number of work units */
#define MAX_N_WU 1024

static char *state_strings[5] = {"invalid", "created", "submitted", 
				 "running", "finished"};

typedef struct {
    char        *wuname;
    char        *clientname;
    char        *arguments;
    char        *workdir;
    char        *infiles[MAX_INFILES];
    char        *localfilenames[MAX_INFILES];
    int         ninfiles;
    int         state;
    int         priority;
    time_t 	subresult_time;
    time_t	ckpt_time;
} dc_wu;

static dc_wu wutable[MAX_N_WU];
static int wu_max, wu_sum;

static int dc_wu_findByState(int state, int from);
static int dc_wu_isWUValid(DC_Workunit wu);
static int dc_wu_getWUIndex(DC_Workunit wu);
dc_wu* dc_wu_getWU(DC_Workunit wu);

static char *dc_projectRootDir;
static char *dc_exec_dir;
static char *dc_workdir_id;

int dc_wu_isWUValid(DC_Workunit wu)
{
  if ((wu < 0) || (wu >= MAX_N_WU)) return DC_ERROR;
  else if (wutable[wu].state == STATE_INVALID) return DC_ERROR;
  
  return DC_OK;
}

dc_wu* dc_wu_getWU(DC_Workunit wu)
{
  return (&wutable[wu]);
}

int dc_wu_getWUIndex(DC_Workunit wu)
{
  return (int)(wu);
}

/* Public */

int dc_wu_findByName(char *wuname)
{
   int ndx = 0;
   int ex = 0;

   while (ndx < MAX_N_WU && !ex) {
       if (wutable[ndx].wuname != NULL &&
	   !strcmp(wutable[ndx].wuname, wuname)) ex = 1;
       else {
         ndx++;
       }
   }
   return ndx;
}

void dc_wu_init(const char *projectroot, const char *workdir_id, const char *exec_dir)   //, const char *uploadkeyfile)
{
    int i;
    for (i = 0; i < MAX_N_WU; i++) {
	wutable[i].wuname  = NULL;
	wutable[i].clientname  = NULL;
	wutable[i].arguments  = NULL;
	wutable[i].workdir = NULL;
	wutable[i].ninfiles = 0;
	wutable[i].state = STATE_INVALID;
	wutable[i].priority = 100;
	wutable[i].subresult_time = 0;
 	wutable[i].ckpt_time = 0;
    }
    wu_max = wu_sum = 0;

    dc_projectRootDir = strdup(projectroot);
    dc_workdir_id = strdup(workdir_id);
    dc_exec_dir = strdup(exec_dir);
}

DC_Workunit dc_wu_create(const char *clientname, 
			 const char *arguments)
{
    int ndx = 0;
    int ex = 0;

    /* Look for an empty slow in dc_wu table */
    while (ndx < MAX_N_WU && !ex) {
	if (wutable[ndx].state == STATE_INVALID) ex = 1;
	else ndx++;
    }
    if (ndx == MAX_N_WU) {
	DC_log(LOG_ERR, "Too many work units (%d) at once",
	       MAX_N_WU);
	return -1;
    }
    if (ndx > wu_max) wu_max = ndx;
    wu_sum++;


    /* Fill in information */ 
    wutable[ndx].clientname    = strdup(clientname);
    wutable[ndx].arguments    = strdup(arguments);
    wutable[ndx].state   = STATE_CREATED;

    return (DC_Workunit) ndx;

}

void dc_wu_getFileName_fromPath(const char *path, char *fn)
{
    /* memory for fn should be allocated before calling this function! */
    char *lastdelim;
    lastdelim = strrchr( path, '/');
    if (lastdelim == NULL) 
	strcpy(fn, (char *) path);
    else if (strlen(lastdelim) == 0) 
	fn[0]='\0';
    else 
	strcpy(fn, lastdelim+1);
}

int dc_wu_setInput(DC_Workunit wu, const char *url, const char* localfilename)
{
    dc_wu* dcwu;
    
    char filename[256];  //, downloadfilename[256];

    if (dc_wu_isWUValid(wu) == DC_ERROR) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
    
    if (dcwu->ninfiles >= MAX_INFILES-1) {
	DC_log( LOG_ERR, "Too many input files for one wu!");
	return DC_ERROR;
    }

    /* Copy input files (extended by _'wuname') into boinc download directory */
    dc_wu_getFileName_fromPath(url, filename);
    if (filename == NULL) {
	DC_log(LOG_ERR, "File name is not given in URL '%s'", url);
    }

    dcwu->infiles[dcwu->ninfiles] = strdup(filename);
    dcwu->localfilenames[dcwu->ninfiles] = strdup(localfilename);
        
//    DC_log( LOG_DEBUG, "Input file '%s' added to wu '%s' and copied into %s",
//    dcwu->infiles[dcwu->ninfiles], dc->.wuname, downloadpath);
    DC_log( LOG_DEBUG, "Input file parameters added to wu:");
    DC_log( LOG_DEBUG, "Input file: %s,       Local name: %s;",dcwu->infiles[dcwu->ninfiles],dcwu->localfilenames[dcwu->ninfiles]);

    dcwu->ninfiles++;

    return DC_OK;
}

int dc_wu_setPriority(DC_Workunit wu, int priority)
{
    dc_wu* dcwu;
    
    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
    
    dcwu->priority = priority;
    return DC_OK;
}

int dc_wu_destroy(DC_Workunit wu)
{
    dc_wu* dcwu;
    char syscmd[512];
    
    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
    
    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING))
    if (remove_wu(dcwu->wuname) != DC_OK )
    {
        DC_log(LOG_ERR,"Cannot remove %s work unit from the info system.", dcwu->wuname);
	return DC_ERROR;
    }
	    
    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING) ||
        (dcwu->state == STATE_FINISHED) || (dcwu->state == STATE_SUSPENDED)) {
	snprintf(syscmd, 512, "rm -r %s", dcwu->workdir);
	if (system(syscmd) == -1) {
	    DC_log(LOG_ERR,"Cannot remove %s working directory!", dcwu->workdir);
	    return DC_ERROR;
	}
    }
			
    if (dcwu->wuname != NULL) free(dcwu->wuname);
    if (dcwu->clientname != NULL) free(dcwu->clientname);
    if (dcwu->arguments != NULL) free(dcwu->arguments);
    if (dcwu->workdir != NULL) free(dcwu->workdir);
    dcwu->state = STATE_INVALID;
    return DC_OK;
}    
    

int dc_wu_submitWU(DC_Workunit wu)
{
    dc_wu* dcwu;
    //char *workdir_base;
    //char *workdir_id;
    char buf[512];
    char *workdir_num;
    char *executable_dir, *jobname,
         *executable, *arguments;
//    char *inputfiles[MAX_INFILES], *local_files[MAX_INFILES];
    int n_of_infiles, wundx, i; //actual_infile = 0;

    const char *input_files[MAX_INFILES], *local_files[MAX_INFILES];
    
    
    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
    wundx = dc_wu_getWUIndex(wu) + 1;
    
//    working_dir = strdup("%s/%d",dc_projectRootDir, wundx);
//    snprintf(working_dir, 512, "%s/%d", dc_projectRootDir, wundx);
//    working_dir = strdup(dc_projectRootDir);

    snprintf(buf, 512, "%d", wundx);
    workdir_num = strdup(buf);

    executable_dir = strdup(dc_exec_dir);
    n_of_infiles = dcwu->ninfiles;
    jobname = strdup("Clgr-test");
    executable = strdup(dcwu->clientname);
    arguments = strdup(dcwu->arguments);
    //type = strdup("batch");

    snprintf(buf, 512, "%s/%s/%s", dc_projectRootDir, dc_workdir_id, workdir_num);
    dcwu->workdir = strdup(buf);

    DC_log(LOG_DEBUG,"Workunit_directory: '%s'", dcwu->workdir);
    
/*
    if (dcwu->ninfiles != 0)
    {
      input_files = (char **) malloc (sizeof(dcwu->infiles[actual_infile]));
      local_files = (char **) malloc (sizeof(dcwu->localfilenames[actual_infile]));
      local_files[actual_infile] = strdup(dcwu->localfilenames[actual_infile]);
      input_files[actual_infile] = strdup(dcwu->infiles[actual_infile]);
      
      actual_infile++;
      
      while (actual_infile < dcwu->ninfiles)
      {
        input_files = (char **) realloc (input_files, sizeof(input_files) + sizeof(dcwu->infiles[actual_infile]));
	input_files[actual_infile] = strdup(dcwu->infiles[actual_infile]);
	local_files = (char **) realloc (local_files, sizeof(local_files) + sizeof(dcwu->localfilenames[actual_infile]));
	local_files[actual_infile] = strdup(dcwu->localfilenames[actual_infile]);
	actual_infile++;
      }
    }
*/

    for (i = 0; i < MAX_INFILES; i++)
    {
	if (dcwu->infiles[i] != NULL)
        input_files[i] = strdup(dcwu->infiles[i]);
	else input_files[i] = NULL;

	if (dcwu->localfilenames[i] != NULL)
	local_files[i] = strdup(dcwu->localfilenames[i]);
	else local_files[i] = NULL;
    }
    
/*    
    DC_log(LOG_DEBUG,"** Workdir_base : %s", dc_projectRootDir);
    DC_log(LOG_DEBUG,"** Workdir_id : %s", dc_workdir_id);
    DC_log(LOG_DEBUG,"** Workdir_num : %s", workdir_num);    
    DC_log(LOG_DEBUG,"** Executable Directory : %s",executable_dir);
    DC_log(LOG_DEBUG,"** Executable : %s", executable);
    DC_log(LOG_DEBUG,"** Jobname : %s",jobname);
    DC_log(LOG_DEBUG,"** Arguments : %s", arguments);
    DC_log(LOG_DEBUG,"** Type : %s", type);
    DC_log(LOG_DEBUG,"** Number of input files : %d",n_of_infiles);
    if (n_of_infiles != 0) DC_log(LOG_DEBUG,"** Input File(s):");
    for (i = 0; i < n_of_infiles; i++)
	DC_log(LOG_DEBUG," - %s", dcwu->infiles[i]);
    if (n_of_infiles != 0) DC_log(LOG_DEBUG,"** Local Filename(s):");
    for (i = 0; i < n_of_infiles; i++)
	DC_log(LOG_DEBUG," - %s", dcwu->localfilenames[i]);

*/
    dcwu->wuname = submit(dc_projectRootDir, dc_workdir_id, workdir_num,
			  executable_dir, executable, jobname, arguments,
			  n_of_infiles, input_files, local_files);

    DC_log(LOG_DEBUG,"The '%s' workunit is submitted", dcwu->wuname);

    
    dcwu->state = STATE_SUBMITTED;
    
    return DC_OK;
}

int dc_wu_cancel(DC_Workunit wu)
{
    int i;
    dc_wu *dcwu;
    char syscmd[512];

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);

    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING))
    {
        if (remove_wu(dcwu->wuname) != DC_OK) {
            DC_log(LOG_ERR,"Cannot remove %s submitted workunit from the grid!", dcwu->wuname);
	    return DC_ERROR;
	}
    }

    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING) ||
        (dcwu->state == STATE_FINISHED) || (dcwu->state == STATE_SUSPENDED)) {
	snprintf(syscmd, 512, "rm -r %s", dcwu->workdir);
	if (system(syscmd) == -1) {
	    DC_log(LOG_ERR,"Cannot remove %s working directory!", dcwu->workdir);
	    return DC_ERROR;
	}
    }

    for (i = 0; i < dcwu->ninfiles; i++ ) {
	dcwu->infiles[i]=NULL;
    }
    dcwu->ninfiles = 0;

    dcwu->state = STATE_CREATED;
    DC_log(LOG_DEBUG,"%s work unit is cancelled!", dcwu->wuname);
    return DC_OK;
}

int dc_wu_suspend(DC_Workunit wu)
{
    dc_wu *dcwu;

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);

    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING))
    {
	dcwu->state = STATE_SUSPENDED;
	return remove_wu(dcwu->wuname);
    }
    DC_log(LOG_ERR,"%s work unit is not submitted, so can't be suspended!", dcwu->wuname);
    return DC_ERROR;
}

int dc_wu_resubmit(DC_Workunit wu)
{
    dc_wu *dcwu;
    char *id[1];

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);

    if (dcwu->state != STATE_SUSPENDED)
    {
        DC_log(LOG_ERR,"%s work unit is not a suspended work unit, so it can't be resubmitted.", dcwu->wuname);
	return DC_ERROR;
    }

    DC_log(LOG_DEBUG,"The '%s' workunit is under Re-submittion...", dcwu->wuname);
    if (resubmit(dcwu->workdir, id) != DC_OK)
    {
	DC_log(LOG_ERR,"Cannot Re-submitting target workunit.");
	return DC_ERROR;
    }

    dcwu->wuname = strdup(id[0]);
    DC_log(LOG_DEBUG,"The Re-submittion is finished. The new name of the workunit is '%s'", dcwu->wuname);
    dcwu->state = STATE_SUBMITTED;

    return DC_OK;
	  
}

int dc_wu_reCreate(DC_Workunit wu)
{
    dc_wu *dcwu;
    char *id[1];

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
	    
    DC_log(LOG_DEBUG,"Re-creating %s workunit.", dcwu->wuname);

    DC_log(LOG_DEBUG,"Deleting it from the info system...");
    if (remove_wu(dcwu->wuname) != DC_OK)
    {
	    DC_log(LOG_ERR,"Cannot delete the workunit under Re-creation, from the info system.");
    }
    
    DC_log(LOG_DEBUG,"Re-submitting target workunit...");
    if (resubmit(dcwu->workdir, id) != DC_OK)
    {
	    DC_log(LOG_ERR,"Cannot Re-submit workunit.");
	    return DC_ERROR;	
    }

    dcwu->wuname = strdup(id[0]);
    DC_log(LOG_DEBUG,"Receation was successful. The new name of the workunit is '%s'", dcwu->wuname);
    dcwu->state = STATE_SUBMITTED;
    return DC_OK;
}

int dc_wu_checkForResult(void (*cb_assimilate_result)(DC_Result result))
{
    static int actual_wu = MAX_N_WU - 1;
    int last_unchecked_wu = 0;

    char *outputdir[1];
    char *result_name[1];
    char *std_out[1];
    char *std_err[1];
    char *sys_log[1];
   
    int exitcode = 0;
    time_t dummy1, dummy2;
    
    last_unchecked_wu = actual_wu;
    actual_wu++;
    if (actual_wu >= MAX_N_WU) actual_wu = 0;
    
    while (actual_wu != last_unchecked_wu)
    {    
        if ((wutable[actual_wu].state == STATE_SUBMITTED) || (wutable[actual_wu].state == STATE_RUNNING))
	{
	    wutable[actual_wu].state = STATE_UNKNOWN;
	    if (ask_status(wutable[actual_wu].wuname, &(wutable[actual_wu].state), &(dummy1), &(dummy2)) != DC_OK)
	    {
		DC_log(LOG_ERR,"Error, while asking status of %s workunit.",wutable[actual_wu].wuname);
		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
		    return -1;
		}
	    }
		    
	    if (wutable[actual_wu].state == STATE_UNDEFINED)
	    {
	        DC_log(LOG_ERR,"%s work unit's state is undefined.", wutable[actual_wu].wuname);
		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
			return -1;	
		}
	    }

	    if (wutable[actual_wu].state == STATE_UNKNOWN)
	    {
		DC_log(LOG_ERR,"%s work unit's state is unknown.", wutable[actual_wu].wuname);
		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
			return -1;	
		}
	    }	    
	    
	    if (wutable[actual_wu].state == STATE_FINISHED)
	    {
                if (get_out(wutable[actual_wu].wuname, wutable[actual_wu].workdir, outputdir,
		result_name, std_out, std_err, sys_log, &exitcode, GETOUT_FINISHED) != DC_OK)
		{
		     DC_log(LOG_ERR,"Cannot get output of %s work unit.",wutable[actual_wu].wuname);

		     if (dc_wu_reCreate(actual_wu) != DC_OK)
		     {
			     return -1;
		     }   
		}
		DC_log(LOG_DEBUG,"The exitcode was: %d", exitcode);
		DC_log(LOG_DEBUG,"Result creating: %s result, to %s wu, with %s output_dir, FILES:  %s, %s, %s;",
				result_name[0], wutable[actual_wu].wuname, outputdir[0], std_out[0], std_err[0], sys_log[0]);
		DC_Result dcresult = dc_result_create(result_name[0], wutable[actual_wu].wuname, outputdir[0],
				                      std_out[0], std_err[0], sys_log[0], exitcode);

		dcresult->status = DC_RESULT_FINAL;
		cb_assimilate_result(dcresult);
		dc_result_free(dcresult);
		return 1;
	    }
                    /**********************/
                    /* GET_OUT_SUBRESULT */
                    /********************/
            if ((dummy2 - wutable[actual_wu].ckpt_time) != 0)
            {

		wutable[actual_wu].ckpt_time = dummy2;
		
		DC_log(LOG_DEBUG,"WU: checkpoint call detected.");
 
		if (get_out(wutable[actual_wu].wuname, wutable[actual_wu].workdir, outputdir,
                result_name, std_out, std_err, sys_log, &exitcode, GETOUT_NOTFINISHED) != DC_OK)
		{
		    DC_log(LOG_ERR,"Cannot get output of %s work unit.",wutable[actual_wu].wuname);
		    if (dc_wu_reCreate(actual_wu) != DC_OK)
		    {
			    return -1;
		    }
		}
	    }

            if ((dummy1 - wutable[actual_wu].subresult_time) != 0)
            {

		wutable[actual_wu].subresult_time = dummy1;
		DC_log(LOG_DEBUG,"WU: subresult detected.");
 
		if (get_out(wutable[actual_wu].wuname, wutable[actual_wu].workdir, outputdir,
                result_name, std_out, std_err, sys_log, &exitcode, GETOUT_NOTFINISHED) != DC_OK)
		{
		    DC_log(LOG_ERR,"Cannot get output of %s work unit.",wutable[actual_wu].wuname);
		    if (dc_wu_reCreate(actual_wu) != DC_OK)
		    {
			    return -1;
		    }
		}

                DC_log(LOG_DEBUG,"Result creating: %s result, to %s wu, with %s output_dir,  FILES: %s,%s,%s;",
                                result_name[0], wutable[actual_wu].wuname, outputdir[0], std_out[0], std_err[0], sys_log[0]);
                DC_Result dcresult = dc_result_create(result_name[0], wutable[actual_wu].wuname, outputdir[0],
				                      std_out[0], std_err[0], sys_log[0], exitcode);
								
		dcresult->status = DC_RESULT_SUB;
                cb_assimilate_result(dcresult);
		dc_result_free(dcresult);
                return 1;
	    }
	}
	actual_wu++;
	if (actual_wu >= MAX_N_WU) actual_wu = 0;
    }
    return 0;
}



#ifdef NEVERDEFINED
int dc_wu_wuCreated(char *wuname, char *workdir)
{
    int ndx = 0;
    int ex = 0;
    while (ndx < MAX_N_WU && !ex) {
	if (wutable[ndx].state == STATE_INVALID) ex = 1;
	else ndx++;
    }
    if (ndx == MAX_N_WU) {
	DC_log(LOG_ERR, "Too many work units (%d) at once",
	       MAX_N_WU);
	return DC_ERROR;
    }
    if (ndx > wu_max) wu_max = ndx;
    wu_sum++;

    wutable[ndx].name    = strdup(wuname);
    wutable[ndx].workdir = strdup(workdir);
    wutable[ndx].state   = STATE_CREATED;

    return DC_OK;
}

int dc_wu_wuSubmitted(char *wuname)
{
    int ndx = dc_rm_findByName(wuname);
    if ((ndx >= MAX_N_WU) || (wundx < 0)){
	DC_log(LOG_ERR, 
	       "Work unit (%s) not created before submission",
	       wuname);
	return DC_ERROR;
    }
    
    wutable[ndx].state = STATE_SUBMITTED;
  
    return DC_OK;
}



int dc_wu_checkForResult(DC_Result **result) 
{
    char cmd[256];
    char name[256];
    int  retval;
    int  ndx = 0;
    int  ex  = 0;

    ndx = dc_wu_findByState(STATE_SUBMITTED, ndx);
    while ((ndx < MAX_N_WU) && (ndx >= 0)) {

	/* check here somehow if a WU is completed */

	if (retval == 1) { /* process finished (not exists) */

	    wutable[ndx].state = STATE_FINISHED;

	    return DC_RM_RESULTEXISTS;
	}
	else if (retval < 0) { /* error */
	    return DC_ERROR;
	}


	ndx = dc_wu_findByState(STATE_SUBMITTED, ndx+1);
    }
    return DC_RM_NORESULT;
}
#endif

void dc_wu_log(void)
{
    int i;
    DC_log(LOG_DEBUG, "----------------------------------------");
    DC_log(LOG_DEBUG, "DC has the following info about WUs");
    DC_log(LOG_DEBUG, "Number of workunits = %d", 
	   wu_sum);
    DC_log(LOG_DEBUG, "Table info: wu_max = %d, wu_sum = %d",
	   wu_max, wu_sum);
    DC_log(LOG_DEBUG, "Table list:");
    DC_log(LOG_DEBUG, "Name        State      WorkDir");
    for (i = 0; i <= wu_max; i++) {
	DC_log(LOG_DEBUG, "%s\t%s\t%s",
	       wutable[i].wuname, 
	       state_strings[wutable[i].state], 
	       wutable[i].workdir);
    }
    DC_log(LOG_DEBUG, "-----------------------------------------");
}

static int dc_wu_findByState(int state, int from)
{
   int ndx = from;
   int ex = 0;
   while (ndx < MAX_N_WU && !ex) {
       if (wutable[ndx].state == state) ex = 1;
       else ndx++;
   }
   return ndx;
}
/*
int dc_wu_getWUbyName(const char *wuname)
{

  char *wu_name;
  strcpy (wu_name, wuname);

  static int ndx = dc_wu_findByName(wu_name);
  if (ndx == MAX_N_WU)
  {
    return NULL;
  }else{
    return (ndx);
  }
}
*/
