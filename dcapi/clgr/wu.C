/* Include BOINC Headers */
//#include "rsaeuro.h"
//#include "crypt.h"
//#include "backend_lib.h"

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

//#define MAX_N_WU 1024

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

//static int dc_wu_findByName(char *wuname);

static int dc_wu_findByState(int state, int from);
static int dc_wu_isWUValid(DC_Workunit wu);
static int dc_wu_getWUIndex(DC_Workunit wu);
dc_wu* dc_wu_getWU(DC_Workunit wu);

//int dc_wu_getWUbyName(const char *wuname);

static char *dc_projectRootDir;
static char *dc_exec_dir;
static char *dc_workdir_id;
//static char *dc_uploadPrivateFile;

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
//	wutable[i].wutemplate = NULL;
//	wutable[i].resulttemplate = NULL;
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

    //dc_uploadPrivateFile = strdup(uploadkeyfile);
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
	return 0;
    }
    if (ndx > wu_max) wu_max = ndx;
    wu_sum++;


    /* Fill in information */ 
    wutable[ndx].clientname    = strdup(clientname);
    wutable[ndx].arguments    = strdup(arguments);
    //wutable[ndx].workdir = strdup(workdir);
    //wutable[ndx].wutemplate    = strdup(wutemplate);
    //wutable[ndx].resulttemplate    = strdup(resulttemplate);
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
    
    //char downloadpath[256], syscmd[1024];
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

//    snprintf(downloadfilename, 256, "%s_%s", filename, dcwu->wuname);
//    snprintf(downloadpath, 256, "%s/download/%s", dc_projectRootDir, downloadfilename);
//    snprintf(syscmd, 1024, "cp %s %s", url, downloadpath);
//    if (system(syscmd) == -1) return DC_ERROR;

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
    
    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
    
    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING) || (dcwu->state == STATE_FINISHED))
    if (remove_wu(dcwu->wuname, dcwu->workdir) != DC_OK )
    {
        DC_log(LOG_ERR,"Cannot remove %s work unit from the info system.", dcwu->wuname);
	return DC_ERROR;
    }
	    
			
    if (dcwu->wuname != NULL) free(dcwu->wuname);
    if (dcwu->clientname != NULL) free(dcwu->clientname);
    if (dcwu->arguments != NULL) free(dcwu->arguments);
    if (dcwu->workdir != NULL) free(dcwu->workdir);
//    if (dcwu->wutemplate != NULL) free(dcwu->wutemplate);
//    if (dcwu->resulttemplate != NULL) free(dcwu->resulttemplate);
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

int dc_wu_suspend(DC_Workunit wu)
{
    dc_wu *dcwu;

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);

    if ((dcwu->state == STATE_SUBMITTED) || (dcwu->state == STATE_RUNNING) || (dcwu->state == STATE_FINISHED))
    {
	dcwu->state = STATE_SUSPENDED;
	return remove_wu(dcwu->wuname, dcwu->workdir);
    }
    DC_log(LOG_ERR,"%s work unit is not submitted, so can't be suspended!");
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
/*
int dc_wu_createBoincWU(int wundx, char *uploadkeyfile, char *boincRootDir)
{
    // char infile_dir[256], download_url[256], upload_url[256];/
    char wutempl[4096];
    char resulttempl_relativepath[256];
    char buf[512];
    //R_RSA_PRIVATE_KEY key;
    //DB_WORKUNIT bwu;
    //DB_APP app;
    char db_name[256], db_passwd[256],db_user[256],db_host[256];
    //SCHED_CONFIG config;
    int retval;

    if ((wundx >= MAX_N_WU) || (wundx < 0)) return DC_ERROR;

    DC_log(LOG_DEBUG, "Create a wu for boinc based on info:\n"
	   "wuname = '%s', clientname = '%s', arguments = '%s', workdir = '%s' "
	   "wutemplate = '%s', resulttemplate = '%s'", 
	   wutable[wundx].wuname, wutable[wundx].clientname, wutable[wundx].arguments, wutable[wundx].workdir,
	   wutable[wundx].wutemplate, wutable[wundx].resulttemplate);
    
    DC_log(LOG_DEBUG, "\t1. Read upload private key from: %s", uploadkeyfile);
//    retval = read_key_file(uploadkeyfile, key);
//    if (retval) {
//        DC_log(LOG_ERR, "wu.c: can't read key: %d", retval);
//        return DC_ERROR;
//    }

    DC_log(LOG_DEBUG, "\t2. Read wu template file: %s", wutable[wundx].wutemplate);
    retval = read_filename(wutable[wundx].wutemplate, wutempl, 4096);
    if (retval) {
	DC_log(LOG_ERR, "wu.c: can't read template file: %d", retval);
        return DC_ERROR;
    } // else {
      //DC_log(LOG_DEBUG, "WU Templ='%s'", wutempl);
      //} 

    DC_log(LOG_DEBUG, "\t3. Determine result template relative path");
    dc_wu_getFileName_fromPath(wutable[wundx].resulttemplate, buf);
    snprintf(resulttempl_relativepath, 256, "templates/%s", buf);
    DC_log(LOG_DEBUG, "\t   %s -> %s", wutable[wundx].resulttemplate, resulttempl_relativepath);

    
    DC_log(LOG_DEBUG, "\t3. Parse boinc project config file");
    retval = config.parse_file(boincRootDir);
    if (retval) {
        DC_log(LOG_ERR, "Can't parse config file: %d", retval);
        return DC_ERROR;
    } else {
        strcpy(db_name, config.db_name);
        strcpy(db_passwd, config.db_passwd);
        strcpy(db_user, config.db_user);
        strcpy(db_host, config.db_host);
    }


    strcpy(app.name, wutable[wundx].clientname);
    DC_log(LOG_DEBUG, 
	   "\t4. Init MySQL connection and get appid for client %s\n"
	   "\t   DB name=%s, host=%s, user=%s", 
	   app.name, db_name, db_host, db_user);
    retval = boinc_db.open(db_name, db_host, db_user, db_passwd);
    if (retval) {
        DC_log(LOG_ERR, "wu.c: error opening database: %d", retval );
        return DC_ERROR;
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        DC_log(LOG_ERR, "wu.c: app not found in database");
        return DC_ERROR;
    } else {
	DC_log(LOG_DEBUG, "\t   Application id in database = %d", app.id); 
    }
    

    DC_log(LOG_DEBUG, "\t5. Create Boinc workunit");

    //
    //snprintf(infile_dir, 256, "%s/download", dc_projectRootDir);
    //snprintf(download_url, 256, "http://n24.hpcc.sztaki.hu/proba1/download");
    //snprintf(upload_url, 256, "http://n24.hpcc.sztaki.hu/proba1/upload");
    //

    strncpy(bwu.name, wutable[wundx].wuname, 256);
    bwu.appid = app.id;
    bwu.batch = 1;
    bwu.rsc_fpops_est = 200000 ;
    bwu.rsc_fpops_bound = 2000000000 ;
    bwu.rsc_memory_bound = 2000000;
    bwu.rsc_disk_bound = 2000000;
    bwu.delay_bound = 720000;
    
    DC_log(LOG_DEBUG, 
	   "\t   Call Boinc create_work with parameters:\n"
	   "\t   appid = %d, wutempl = '%50.50s...'\n"
	   "\t   resulttemplate = '%s' and '%s'\n"
	   "\t   # of inputfiles = %d, key.mod = '%40.40s'...",
	   bwu.appid, wutempl, resulttempl_relativepath, wutable[wundx].resulttemplate,
	   wutable[wundx].ninfiles, key.modulus);

    DC_log(LOG_DEBUG, "\t  *** This is the 1. checkpoint !!! ***");



    retval = create_work(
        bwu,
	wutempl,                  // WU template, contents, not path
	resulttempl_relativepath, // Result template filename, relative to project root
	wutable[wundx].resulttemplate,     // Result template, absolute path,
	const_cast<const char **>(wutable[wundx].infiles), // array of input file names
	wutable[wundx].ninfiles,
	key,                      // upload authentication key
	config                    // data from config.xml
	);

    DC_log(LOG_DEBUG, "\t  *** This is the 2. checkpoint !!! ***");

    if (retval) {
        DC_log(LOG_ERR, "wu.c, cannot create workunit: %d", retval);
	return DC_ERROR;
    }

    boinc_db.close();


    return DC_OK;

}
*/

int dc_wu_reCreate(DC_Workunit wu)
{
    dc_wu *dcwu;
    char *id[1];

    if (dc_wu_isWUValid(wu) != DC_OK) return DC_ERROR;
    dcwu = dc_wu_getWU(wu);
	    
    DC_log(LOG_DEBUG,"Re-creating %s workunit.", dcwu->wuname);

    DC_log(LOG_DEBUG,"Deleting it from the info system...");
    if (remove_wu(dcwu->wuname, dcwu->workdir) != DC_OK)
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

    static int counter1 = 0;
    //int wustatus;    

    //char *result_name = NULL;
    //char *result_std_out = NULL;
    //char *result_std_err = NULL;
    //char *result_std_log = NULL;
    //int n_of_outfiles;
    
    char *outputdir[1];
    char *result_name[1];

    char *std_out[1];
    char *std_err[1];
    char *sys_log[1];
   
    int exitcode = 0;
    //int i, n_of_outfiles;
    //char *outfiles[MAX_OUTFILES];

    //static bool did_something;
    
    time_t dummy1, dummy2;
    
    last_unchecked_wu = actual_wu;
    actual_wu++;
    if (actual_wu >= MAX_N_WU) actual_wu = 0;
    
    counter1++;

    DC_log(LOG_DEBUG,"Check for result %d. running.", counter1);
    //DC_log(LOG_DEBUG," ");
  /* 
    if (counter1 == 3)
    {
wutable[0].wuname = strdup("https://testentry.niif.grid/clgr-broker/jobs/2206820f-b038-41ac-9e28-88dceae5b16a");
wutable[0].workdir = strdup("./workdir/clusterGrid_ClgrProba1_1110378262/1");
wutable[1].wuname = strdup("https://testentry.niif.grid/clgr-broker/jobs/e2fee82b-e7a9-439f-a072-fb0262d7b071");
wutable[1].workdir = strdup("./workdir/clusterGrid_ClgrProba1_1110378262/2");
    }
  */

    
    //did_something = false;
    //for (actual_wu = 0; actual_wu < MAX_N_WU; actual_wu++)
    while (actual_wu != last_unchecked_wu)
    {    
        if ((wutable[actual_wu].state == STATE_SUBMITTED) || (wutable[actual_wu].state == STATE_RUNNING))
	{
	    //wutable[actual_wu].state = ask_status(wutable[actual_wu].wuname);  
	    wutable[actual_wu].state = STATE_UNKNOWN;
	    if (ask_status(wutable[actual_wu].wuname, &(wutable[actual_wu].state), &(dummy1), &(dummy2)) != DC_OK)
	    {
	        //DC_log(LOG_ERR,"Error in %s work unit's status.",wutable[actual_wu].wuname);
		//return -1;

		DC_log(LOG_ERR,"Error, while asking status of %s workunit.",wutable[actual_wu].wuname);
		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
		    return -1;
		}
	    }
		    
	    if (wutable[actual_wu].state == STATE_UNDEFINED)
	    {
	        DC_log(LOG_ERR,"%s work unit's state is undefined.", wutable[actual_wu].wuname);
		/*
		DC_Result dcresult = dc_result_create("error_result", wutable[actual_wu].wuname,
				 	 	      "./", NULL, NULL, NULL, exitcode);
		dcresult->status = result_failed;
		cb_assimilate_result(dcresult);
		return 1;
		*/

		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
			return -1;	
		}
	    }

	    if (wutable[actual_wu].state == STATE_UNKNOWN)
	    {
		DC_log(LOG_ERR,"%s work unit's state is unknown.", wutable[actual_wu].wuname);
		/*
		DC_Result dcresult = dc_result_create("error_result", wutable[actual_wu].wuname,
		     				      "./", NULL, NULL, NULL, exitcode);
	        dcresult->status = result_failed;
	        cb_assimilate_result(dcresult);
	        return 1;
	        */    

		if (dc_wu_reCreate(actual_wu) != DC_OK)
		{
			return -1;	
		}
	    }	    
	    
	    //DC_log(LOG_DEBUG,"*%s,\n %s,\n %i", wutable[actual_wu].wuname, wutable[actual_wu].workdir, wutable[actual_wu].state);
	    
	    if (wutable[actual_wu].state == STATE_FINISHED)
	    {
		//did_something = true;
		/*DC_log(LOG_DEBUG,"*** workdir: %s,  outputdir: %s,  result_name: %s,  n_of_files: %d",
				wutable[actual_wu].workdir, outputdir[0], result_name[0], n_of_outfiles);*/
		
                if (get_out(wutable[actual_wu].wuname, wutable[actual_wu].workdir, outputdir,
		result_name, std_out, std_err, sys_log, &exitcode, GETOUT_FINISHED) != DC_OK)
		{
		     DC_log(LOG_ERR,"Cannot get output of %s work unit.",wutable[actual_wu].wuname);
		     /*
		     DC_Result dcresult = dc_result_create("error_result", wutable[actual_wu].wuname,
				     			   "./", NULL, NULL, NULL, exitcode);
		     dcresult->status = result_failed;
		     cb_assimilate_result(dcresult);
		     return 1;
		     */

		     if (dc_wu_reCreate(actual_wu) != DC_OK)
		     {
			     return -1;
		     }   
		}
		DC_log(LOG_DEBUG,"The exitcode was: %d", exitcode);
		//I think this is the point where the exitcode should be mined from the log fil
		//but correct me if I am wrong

		
		//DC_log(LOG_DEBUG,"***%s, %i", wutable[actual_wu].wuname, wutable[actual_wu].state);
		/*DC_log(LOG_DEBUG,"*** workdir: %s,  outputdir: %s,  result_name: %s,  n_of_files: %d",
				wutable[actual_wu].workdir, outputdir[0], result_name[0]);
		*/
		DC_log(LOG_DEBUG,"Result creating: %s result, to %s wu, with %s output_dir, FILES:  %s, %s, %s;",
				result_name[0], wutable[actual_wu].wuname, outputdir[0], std_out[0], std_err[0], sys_log[0]);
		DC_Result dcresult = dc_result_create(result_name[0], wutable[actual_wu].wuname, outputdir[0],
				                      std_out[0], std_err[0], sys_log[0], exitcode);
		/*for (i = 0; i < n_of_outfiles; i++)
		{
	    //DC_log(LOG_DEBUG,"%s/%s added to %s result.", outputdir[0], outfiles[i], result_name[0]);
		    dc_result_addOutputFile(dcresult, outfiles[i]);
		}*/

		dcresult->status = DC_RESULT_FINAL;
		cb_assimilate_result(dcresult);
		return 1;

		
	    }else
                    /**********************/
                    /* GET_OUT_SUBRESULT */
                    /********************/
            if (((dummy1 - wutable[actual_wu].subresult_time) != 0) && ((dummy2 - wutable[actual_wu].ckpt_time) != 0))
            {

		wutable[actual_wu].subresult_time = dummy1;
		wutable[actual_wu].ckpt_time = dummy2;
		
		DC_log(LOG_DEBUG,"WU: subresult detected.");
		/*DC_log(LOG_DEBUG,"*** work unit state: %s,  subresult_time: %d,  ckpt_time: %d",
				wutable[actual_wu].state, dummy1, dummy2);*/
 
		if (get_out(wutable[actual_wu].wuname, wutable[actual_wu].workdir, outputdir,
                result_name, std_out, std_err, sys_log, &exitcode, GETOUT_NOTFINISHED) != DC_OK)
		{
		    DC_log(LOG_ERR,"Cannot get output of %s work unit.",wutable[actual_wu].wuname);
		    /*
		    DC_Result dcresult = dc_result_create("error_result", wutable[actual_wu].wuname,
					                  "./", NULL, NULL, NULL, exitcode);
		    dcresult->status = result_failed;
		    cb_assimilate_result(dcresult);
		    return 1;								 
		    */
		                    
		    if (dc_wu_reCreate(actual_wu) != DC_OK)
		    {
			    return -1;
		    }
		}

                //DC_log(LOG_DEBUG,"***%s, %i", wutable[actual_wu].wuname, wutable[actual_wu].state);
                /*DC_log(LOG_DEBUG,"*** workdir: %s,  outputdir: %s,  result_name: %s  FILES: %s,%s,%s;",
                                wutable[actual_wu].workdir, outputdir[0], result_name[0], std_out[0], std_err[0], sys_log[0]);
                */
                DC_log(LOG_DEBUG,"Result creating: %s result, to %s wu, with %s output_dir,  FILES: %s,%s,%s;",
                                result_name[0], wutable[actual_wu].wuname, outputdir[0], std_out[0], std_err[0], sys_log[0]);
                DC_Result dcresult = dc_result_create(result_name[0], wutable[actual_wu].wuname, outputdir[0],
				                      std_out[0], std_err[0], sys_log[0], exitcode);
                /*for (i = 0; i < n_of_outfiles; i++)
                {
                    DC_log(LOG_DEBUG,"%s/%s added to %s result.", outputdir[0], outfiles[i], result_name[0]);
                    dc_result_addOutputFile(dcresult, outfiles[i]);
                }*/
								
		dcresult->status = DC_RESULT_SUB;
                cb_assimilate_result(dcresult);
                return 1;
	    }
	    //else DC_log(LOG_DEBUG,"%d;  %d ", dummy1, dummy2);
	}
	actual_wu++;
	if (actual_wu >= MAX_N_WU) actual_wu = 0;
    }
    //result = create_result();
    //if (did_something) return 1;
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
