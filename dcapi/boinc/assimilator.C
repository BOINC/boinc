#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <strings.h>

#include "dc_boinc.h"

#include <boinc_db.h>
#include <parse.h>
#include <util.h>
#include <sched_config.h>
#include <sched_util.h>
#include <sched_msgs.h>
#include "validate_util.h"

using std::vector;
using std::string;

#define LOCKFILE "assimilator.out"
#define PIDFILE  "assimilator.pid"

SCHED_CONFIG config;
static DB_APP app;


void assimilate_handler(
   WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result,
   void (*cb_assimilate_result)(DC_Result *result)) 
{
    if (wu.canonical_resultid) {
	string output_file_name;
	char *id, *output_dir;
	
        DC_log(LOG_INFO, "\t[%s] Found canonical result: %s", 
	       wu.name, canonical_result.xml_doc_out);
	char filename[256], *p;
	size_t len;

	p = strstr(canonical_result.xml_doc_out, "<name>");
	if (p != NULL) {
	    p = strchr(p, '>') + 1;   /* first char after <name> */
	    len = strcspn(p, "<");    /* # of chars until next opening <, i.e. </name> */
	    if (len > 256) len = 256;
	    snprintf(filename, len+1, "%s", p);


	    DC_log(LOG_DEBUG, "\t   Result file = '%s', output dir = '%s'", 
		   filename, config.upload_dir);

	    /* Create a DC_Result and pass it to the callback function */
	    DC_Result *dcresult = dc_result_create(filename, wu.name, config.upload_dir);
	    dc_result_addOutputFile(dcresult, filename);

	    if (get_output_file_path(canonical_result, output_file_name))
	    {
		DC_log(LOG_ERR,"   Cannot get output files dir. (get_output_file_path)" );
	    }
	    else
	    {
		output_dir = strdup(output_file_name.c_str());
		DC_log(LOG_DEBUG, "\t   The output file name is : '%s'", output_dir);
		id = strtok(output_dir, "/");
	        while (strcmp(id,"upload")){
		    id = strtok(NULL, "/");
		}
	        id = strtok(NULL, "/");

		sprintf(output_dir, "%s/%s", dcresult->outfiles_dir, id);
		//strcpy(dcresult->outfiles_dir, output_dir);
		dcresult->outfiles_dir = strdup(output_dir);
		DC_log(LOG_DEBUG, "output dir = '%s'", output_dir);

	    }

	    cb_assimilate_result(dcresult);
	}
	else {
	    DC_log(LOG_ERR, "\t  Cannot determine result file name");
	}
	
    } else {
        DC_log(LOG_WARNING, "\t[%s] No canonical result", wu.name);
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        DC_log(LOG_ERR, "\t[%s] Error: couldn't send a result", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        DC_log(LOG_ERR,"\t[%s] Error: too many error results", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        DC_log(LOG_ERR, "\t[%s] Error: too many total results", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        DC_log(LOG_ERR, "\t[%s] Error: too many success results", wu.name);
    }
}

// assimilate one WU that needs it
// return nonzero if did anything
//
bool do_pass(APP& app, void (*cb_assimilate_result)(DC_Result *result)) {
    DB_WORKUNIT wu;
    DB_RESULT canonical_result, result;
    bool did_something = false;
    char buf[256];
    // int retval;


    check_stop_daemons();

    sprintf(buf, "where appid=%d and assimilate_state=%d limit 1", 
	    app.id, ASSIMILATE_READY);
    while (!wu.enumerate(buf)) {
        vector<RESULT> results;     // must be inside while()!
        did_something = true;

        DC_log(LOG_INFO,
            "[%s] assimilating; state=%d", wu.name, wu.assimilate_state
        );

        sprintf(buf, "where workunitid=%d", wu.id);
        while (!result.enumerate(buf)) {
            results.push_back(result);
            if (result.id == wu.canonical_resultid) {
                canonical_result = result;
            }
        }

        assimilate_handler(wu, results, canonical_result, cb_assimilate_result);

        wu.assimilate_state = ASSIMILATE_DONE;
        wu.transition_time = time(0);
        wu.update();
    }
    return did_something;
}

int dc_assimilator_init(char * projectroot, char * appname)
{
    int retval;
    char buf[256];

    DC_log(LOG_DEBUG, "Initialise assimilator");

    DC_log(LOG_DEBUG, "\t1. check_stop_daemons");
    check_stop_daemons();

    DC_log(LOG_DEBUG, "\t2. Parse boinc project config file");
    retval = config.parse_file(projectroot);
    if (retval) {
        DC_log(LOG_ERR, "Can't parse config file: %d", retval);
        return DC_ERROR;
    }

    strcpy(app.name, appname);
    DC_log(LOG_DEBUG, 
	   "\t3. Init MySQL connection and get appid for client %s"
	   "\t   DB name=%s, host=%s, user=%s", 
	   app.name, config.db_name, config.db_host, config.db_user);
    retval = boinc_db.open(config.db_name, config.db_host, 
			   config.db_user, config.db_passwd);
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

    DC_log(LOG_DEBUG, "\t4. install SIGHUP handler again");
    install_stop_signal_handler();
    boinc_db.close();
     
    return DC_OK;
}

int dc_assimilator_dopass(void (*cb_assimilate_result)(DC_Result result))
{
    int retval;
    
    retval = boinc_db.open(config.db_name, config.db_host, 
			   config.db_user, config.db_passwd);
    if (retval) {
        DC_log(LOG_ERR, 
	       "wu.c: error opening database: %d, DB name=%s, host=%s, user=%s", 
	       retval, config.db_name, config.db_host, config.db_user );
        return DC_ERROR;
    }    
    

    if (do_pass(app, cb_assimilate_result)) retval = 1;
    else retval = 0;

    boinc_db.close();

    return retval;
}
