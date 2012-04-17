// NOTE: this is from Derrick Kondo (INRIA).  Needs to be cleaned up a bit.

/* Assigns uncompleted (and unassigned) workunits of a batch to hosts
     registered under a DEDICATED_USER_ID. The constant DEDICATED_USER_ID
     should be defined accordingly below. 

     The constant MAX_TO_ASSIGN should be defined according to the size of
     the feeder's shmem array.

     The project dir must be defined in the projects config.xml file.

     Input parameter of the form 'batch=[id]' can be passed via url or commmand line.

     Example usage from command line:
     ./target_batch batch=1

     Example usage from URL:
     http://abenaki.imag.fr/clouds_ops/target_batch?batch=1

     ----------------

     To enable targeted jobs in general in BOINC, add
     <enable_assignment>1</enable_assignment>
     to the config.xml file.

*/

/* TESTS

PLAIN OLD SCHEDULING OF A BATCH

Works as long as boinc client is up-to-date.    

-----------------------

SCHEDULING OF BATCH TARGETTED TO DEDICATED HOST

Create a batch of low-priority tasks:
./create_test_work.pl -appname example_app -numwu 10 -batch 0 -label "lowpri" -db clouds -password "" -wu "templates/example_app_in.xml" -result "templates/example_app_out.xml"


Target batch to dedicated workers:
/bin/target_batch batch=0

Start dedicated work with USER_ID corresonding to DEDICATED_USER_ID.

Check that these tasks are only assigned to dedicated workers.

* Assignment table appears correct
* Cloud worker downloads and processes targetted results.    Results in ops
* php page have status "Didn't need"
* Non-cloud workers do not download targetted results

-----------------------

OOO What cleans up the assignment table?

*/ 


#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <string>

using std::vector;
using std::string;

#include "boinc_db.h"
#include "crypt.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "parse.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"
#include "svn_version.h"




#define DEDICATED_USER_ID 1
#define MAX_TO_ASSIGN 100

int main(int argc, char *argv[])
{
    //must have two lines after content header.    Else you will get "Internal
    //Server Error" when executing script
    printf ("Content-type: text/plain\n\n");
    printf ("Targeting batch\n\n");

    int retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                                                "Can't parse config.xml: %s\n", boincerror(retval)
                                                );
        exit(1);
        }


    
    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open DB\n");
        exit(1);
    }


    char buf[256];

    ////////////////////////////////////////////////////////////////////////////
    // Get targetted batch id
    char * query_str = getenv("QUERY_STRING");
    if (query_str == NULL){
        printf ("Batch id not provided in url.    Checking for command line argument...\n");

        if (argc==2 && argv[1] != NULL){
            printf ("\tUsing batch id from command line\n\n");
            query_str = argv[1];
        } else {
            printf ("Error: batch id not provided, argc: %d\n", argc);
            exit(1);
        }
    }

    char batch_str[256];
    int batch;
    if (sscanf(query_str, "batch=%256s", &batch_str) == 1){
            batch = atoi(batch_str);
            printf ("Targetting batch id: %d\n\n", batch);
    } else {
        printf ("Bad argument format. Should be \"batch=[id]\"\n");
        exit(1);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Left join to get workunits not already in the
    // assignment table

    // TESTED: DOESN'T RESELECT ASSIGNED WUS
    sprintf(buf,
"SELECT workunit.* \
FROM workunit \
LEFT JOIN assignment \
ON workunit.id = assignment.workunitid \
WHERE assignment.workunitid IS NULL \
AND canonical_resultid = 0 AND batch = %d \
LIMIT %d",
                    batch, MAX_TO_ASSIGN);
    // Tried moving "AND canonical_resultid = 0 AND batch = %d \" in line
    // where join appears, but the results of the query were wrong

    
        retval = boinc_db.do_query(buf);
        if (retval) {
            printf ("Problem with db\n");
            boinc_db.close();
            exit(1);
        }

        MYSQL_RES* rp;

        rp = mysql_store_result(boinc_db.mysql);
        if (!rp) {
            printf ("Problem with db\n");
            boinc_db.close();
            exit(1);

        }

        MYSQL_ROW row;
        DB_WORKUNIT workunit;

        int num_assigned=0;
        while ((row = mysql_fetch_row(rp))){
            workunit.db_parse(row);
            printf ("Assigning WU %d to user \n", workunit.id, DEDICATED_USER_ID);
        
            restrict_wu_to_user (workunit, DEDICATED_USER_ID);

            num_assigned++;
            printf ("End of this WU assignment\n\n");
            
        }

        mysql_free_result(rp);

    ///////////////////////////////////////////////////////////////////////////
    // reset the feeder

    char cmd_buf[256];

    if (num_assigned>0){
        sprintf (cmd_buf, "touch %s/reread_db", config.project_dir);

        printf ("Running cmd: %s", cmd_buf);

        int ret_val = system (cmd_buf);
        if (ret_val > 0){
            printf ("Houston: we have a reread_db problem\n");
        }
    }

    printf ("\nDONE.\n");
}


