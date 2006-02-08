#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Include BOINC Headers */
#include <backend_lib.h>
#include <sched_util.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sched_config.h>

#include <dc.h>
#include "wu.h"
#include "result.h"

extern SCHED_CONFIG config;

/* PRIVATE */

/* maximum number of work units */
#define MAX_N_WU 10024

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

/* Define meximum number of input files */
#define MAX_INFILES 4

typedef struct {
    char        *wuname;
    char        *clientname;
    char        *arguments;
    char        *workdir;
    char        *wutemplate;
    char        *resulttemplate;
    char       *infiles[MAX_INFILES];
    int         ninfiles;
    int         state;
    int         priority;
} dc_wu;


static dc_wu wutable[MAX_N_WU];
static int wu_max, wu_sum;

//static int dc_wu_findByName(char *wuname);
static int dc_wu_findByState(int state, int from);
//int dc_wu_getWUbyName(const char *wuname);

static char *dc_projectRootDir;
static char *dc_uploadPrivateFile;

/* PUBLIC */

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

void dc_wu_init(const char *projectroot, const char *uploadkeyfile)
{
    int i;
    for (i = 0; i < MAX_N_WU; i++) {
	wutable[i].wuname  = NULL;
	wutable[i].clientname  = NULL;
	wutable[i].arguments  = NULL;
	wutable[i].workdir = NULL;
	wutable[i].wutemplate = NULL;
	wutable[i].resulttemplate = NULL;
	wutable[i].ninfiles = 0;
	wutable[i].state = STATE_INVALID;
	wutable[i].priority = 100;
    }
    wu_max = wu_sum = 0;

    dc_projectRootDir = strdup(projectroot);
    dc_uploadPrivateFile = strdup(uploadkeyfile);
}

DC_Workunit dc_wu_create(const char *wu_name, const char *clientname, 
		 const char *arguments, const char *workdir, 
		 const char *wutemplate, const char *resulttemplate)
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


    //DC_log(LOG_DEBUG," *** The name of the new wu is : '%s'",wu_name);
    //DC_log(LOG_DEBUG," *** The name of the old wu is : '%s'",wutable[ndx].wuname);
    
    
    /* Fill in information */ 
    wutable[ndx].wuname = NULL;
    wutable[ndx].wuname   = strdup(wu_name);
    wutable[ndx].clientname    = strdup(clientname);
    wutable[ndx].arguments    = strdup(arguments);
    wutable[ndx].workdir = strdup(workdir);
    wutable[ndx].wutemplate    = strdup(wutemplate);
    wutable[ndx].resulttemplate    = strdup(resulttemplate);
    wutable[ndx].state   = STATE_CREATED;
    wutable[ndx].ninfiles = 0;

    DC_log(LOG_DEBUG,"The name of the created wu is : '%s'     ##%d##",wutable[ndx].wuname, ndx);
    
    return (ndx);

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
    char downloadpath[256], syscmd[1024], download_dir[256];
    char filename[256], downloadfilename[256];

    if ((wu >= MAX_N_WU) || (wu < 0)) return DC_ERROR;

    if (wutable[wu].ninfiles >= MAX_INFILES-1) {
	DC_log( LOG_ERR, "Too many input files for one wu!");
	return DC_ERROR;
    }

    /* Copy input files (extended by _'wuname') into boinc download directory */
    dc_wu_getFileName_fromPath(url, filename);
    if (filename == NULL) {
	DC_log(LOG_ERR, "File name is not given in URL '%s'", url);
    }

    snprintf(downloadfilename, 256, "%s_%s", filename, wutable[wu].wuname);
//    snprintf(downloadpath, 256, "%s/download/%s", dc_projectRootDir, downloadfilename);
    snprintf(download_dir, 256, "%s/download", dc_projectRootDir);
    
#if BOINC_VERSION == 4
    dir_hier_path(downloadfilename, download_dir, config.uldl_dir_fanout, true, downloadpath, true);
#else
    dir_hier_path(downloadfilename, download_dir, config.uldl_dir_fanout, downloadpath, true);
#endif

    snprintf(syscmd, 1024, "cp %s %s", url, downloadpath);
    DC_log(LOG_DEBUG, "system command: '%s'", syscmd);

    if (system(syscmd) == -1) return DC_ERROR;

    wutable[wu].infiles[wutable[wu].ninfiles] = strdup(downloadfilename);
    DC_log( LOG_DEBUG, "Input file '%s' added to wu '%s' and copied into %s     ##%d##",
	    wutable[wu].infiles[wutable[wu].ninfiles], wutable[wu].wuname, downloadpath, wu);

    wutable[wu].ninfiles++;

    return DC_OK;
}


int dc_wu_setPriority(DC_Workunit wu, int priority)
{
    if ((wu >= MAX_N_WU) || (wu < 0)) return DC_ERROR;
    wutable[wu].priority = priority;
    return DC_OK;
}

int dc_wu_destroy(DC_Workunit wu)
{
    if ((wu >= MAX_N_WU) || (wu < 0)) return DC_ERROR;

    if (wutable[wu].wuname != NULL) free(wutable[wu].wuname);
    if (wutable[wu].clientname != NULL) free(wutable[wu].clientname);
    if (wutable[wu].arguments != NULL) free(wutable[wu].arguments);
    if (wutable[wu].workdir != NULL) free(wutable[wu].workdir);
    if (wutable[wu].wutemplate != NULL) free(wutable[wu].wutemplate);
    if (wutable[wu].resulttemplate != NULL) free(wutable[wu].resulttemplate);
    wutable[wu].state = STATE_INVALID;
    return DC_OK;
}


int dc_wu_createBoincWU(DC_Workunit wu, char *uploadkeyfile, char *boincRootDir)
{
    /* char infile_dir[256], download_url[256], upload_url[256];*/
    char wutempl_orig[4096], wutempl[4096];
    char resulttempl_relativepath[256];
    char buf[512];
    R_RSA_PRIVATE_KEY key;
    DB_WORKUNIT bwu;
    DB_APP app;
    char db_name[256], db_passwd[256],db_user[256],db_host[256];
    SCHED_CONFIG config;
    int retval;

    if ((wu >= MAX_N_WU) || (wu < 0)) return DC_ERROR;

    DC_log(LOG_DEBUG, "Create a wu for boinc based on info:\n"
	   "wuname = '%s', clientname = '%s', arguments = '%s', workdir = '%s' "
	   "wutemplate = '%s', resulttemplate = '%s'", 
	   wutable[wu].wuname, wutable[wu].clientname, wutable[wu].arguments, wutable[wu].workdir,
	   wutable[wu].wutemplate, wutable[wu].resulttemplate);
    
    DC_log(LOG_DEBUG, "\t1. Read upload private key from: %s", uploadkeyfile);
    retval = read_key_file(uploadkeyfile, key);
    if (retval) {
        DC_log(LOG_ERR, "wu.c: can't read key: %d", retval);
        return DC_ERROR;
    }

    DC_log(LOG_DEBUG, "\t2. Read wu template file: %s", wutable[wu].wutemplate);
    retval = read_filename(wutable[wu].wutemplate, wutempl_orig, 4096);
    if (retval) {
	DC_log(LOG_ERR, "wu.c: can't read template file: %d", retval);
        return DC_ERROR;
    } else { // insert command line arguments into the template
      char *ptr = strstr(wutempl_orig, "</command_line>");
      ptr[0] = '\0';
      strcpy(wutempl, wutempl_orig);
      strcat(wutempl, wutable[wu].arguments);
      ptr[0] = '<';
      strcat(wutempl, ptr);
      DC_log(LOG_DEBUG, "********\nWU Templ='%s'\n**********", wutempl);
    }


    

    DC_log(LOG_DEBUG, "\t3. Determine result template relative path");
    dc_wu_getFileName_fromPath(wutable[wu].resulttemplate, buf);
    snprintf(resulttempl_relativepath, 256, "templates/%s", buf);
    DC_log(LOG_DEBUG, "\t   %s -> %s", wutable[wu].resulttemplate, resulttempl_relativepath);

    
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


    strcpy(app.name, wutable[wu].clientname);
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

    /*
    snprintf(infile_dir, 256, "%s/download", dc_projectRootDir);
    snprintf(download_url, 256, "http://n24.hpcc.sztaki.hu/proba1/download");
    snprintf(upload_url, 256, "http://n24.hpcc.sztaki.hu/proba1/upload");
    */

    strncpy(bwu.name, wutable[wu].wuname, 256);
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
	   "\t   # of inputfiles = %d, //key.mod = '%40.40s'...",
	   bwu.appid, wutempl, resulttempl_relativepath, wutable[wu].resulttemplate,
	   wutable[wu].ninfiles/*, key.modulus*/);

    //DC_log(LOG_DEBUG, "\t  *** This is the 1. checkpoint !!! ***  ");



    retval = create_work(
        bwu,
	wutempl,                  // WU template, contents, not path
	resulttempl_relativepath, // Result template filename, relative to project root
	wutable[wu].resulttemplate,     // Result template, absolute path,
	const_cast<const char **>(wutable[wu].infiles), // array of input file names
	wutable[wu].ninfiles,
	//key,                      // upload authentication key
	config                    // data from config.xml
	);

    //DC_log(LOG_DEBUG, "\t  *** This is the 2. checkpoint !!! ***  ");

    if (retval) {
        DC_log(LOG_ERR, "wu.c, cannot create workunit: %d", retval);
	return DC_ERROR;
    }

    boinc_db.close();


    return DC_OK;

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
