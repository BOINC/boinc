// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//
// TO DO: 
//        1) Need to add support for compressed zip of archived files that may contain max X number
//         of items (similiar to db_dump.C).
//        2) Need to escape XML output

// purges workunit and result records that are no longer needed from 
// the database
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

#include "boinc_db.h"
#include "util.h"
#include "parse.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#include "error_numbers.h"

#define WU_FILENAME_PREFIX    "wuarchive"
#define RESULT_FILENAME_PREFIX    "resultarchive"


SCHED_CONFIG    config;
FILE            *wu_stream;
FILE            *re_stream;
    
//open archive files for purged workunits with the name- wuarchive_TIMESTAMP.xml
//open archive files for purged results with the name- resultarchive_TIMESTAMP.xml
int open_archive(char* filename_prefix, FILE*& f){
    int   retval=0;
    char  path[256];

    sprintf(path,"../archives/%s_%d.xml", filename_prefix, (int)time(0));

    log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Opening archive %s\n", path);
    
    if ((f = fopen( path,"a+")) == NULL) {  
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Can't open archive file %s\n", path);
    return ERR_FOPEN;
    }

    fprintf(f,
        "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
    );

    fprintf(f,
        "<%s>\n",filename_prefix
    );

    return retval;
}


int archive_result(DB_RESULT& result) {
    fprintf(re_stream,
        "<result_archive>\n"
        "    <id>%d</id>\n",
        result.id
    );
    
    string r1, r2;
    r1= result.stderr_out;
    xml_escape(r1, r2);

    fprintf(
        re_stream,
        "  <create_time>%d</create_time>\n"
        "  <workunitid>%d</workunitid>\n"
        "  <server_state>%d</server_state>\n"
        "  <outcome>%d</outcome>\n"
        "  <client_state>%d</client_state>\n"
        "  <hostid>%d</hostid>\n"
        "  <userid>%d</userid>\n"
        "  <report_deadline>%d</report_deadline>\n"
        "  <sent_time>%d</sent_time>\n"
        "  <received_time>%d</received_time>\n"
        "  <name>%s</name>\n"
        "  <cpu_time>%.15e</cpu_time>\n"
        "  <xml_doc_in>%s</xml_doc_in>\n"
        "  <xml_doc_out>%s</xml_doc_out>\n"
        "  <stderr_out>%s</stderr_out>\n"
        "  <batch>%d</batch>\n"
        "  <file_delete_state>%d</file_delete_state>\n"
        "  <validate_state>%d</validate_state>\n"
        "  <claimed_credit>%.15e</claimed_credit>\n"
        "  <granted_credit>%.15e</granted_credit>\n"
        "  <opaque>%f</opaque>\n"
        "  <random>%d</random>\n"
        "  <app_version_num>%d</app_version_num>\n"
        "  <appid>%d</appid>\n"
        "  <exit_status>%d</exit_status>\n"
        "  <teamid>%d</teamid>\n",                                            
        result.create_time, 
        result.workunitid,
        result.server_state, 
        result.outcome,
        result.client_state,
        result.hostid, 
        result.userid,
        result.report_deadline, 
        result.sent_time, 
        result.received_time,
        result.name, 
        result.cpu_time,
        result.xml_doc_in, 
        result.xml_doc_out, 
        r2.c_str(),
        result.batch, 
        result.file_delete_state, 
        result.validate_state,
        result.claimed_credit, 
        result.granted_credit, 
        result.opaque, 
        result.random,
        result.app_version_num, 
        result.appid, 
        result.exit_status, 
        result.teamid
    );

    fprintf(re_stream,
        "</result_archive>\n"
    );
    return 0;
}

int archive_wu(DB_WORKUNIT& wu) {
    fprintf(wu_stream,
        "<workunit_archive>\n"
        "    <id>%d</id>\n",
        wu.id
    );
    fprintf(wu_stream,
        "  <create_time>%d</create_time>\n"
        "  <appid>%d</appid>\n"
        "  <name>%s</name>\n"
        "  <xml_doc>%s</xml_doc>\n"
        "  <batch>%d</batch>\n"
        "  <rsc_fpops_est>%.15e</rsc_fpops_est>\n"
        "  <rsc_fpops_bound>%.15e</rsc_fpops_bound>\n"
        "  <rsc_memory_bound>%.15e</rsc_memory_bound>\n"
        "  <rsc_disk_bound>%.15e</rsc_disk_bound>\n"
        "  <need_validate>%d</need_validate>\n"
        "  <canonical_resultid>%d</canonical_resultid>\n"
        "  <canonical_credit>%.15e</canonical_credit>\n"
        "  <transition_time>%d</transition_time>\n"
        "  <delay_bound>%d</delay_bound>\n"
        "  <error_mask>%d</error_mask>\n"
        "  <file_delete_state>%d</file_delete_state>\n"
        "  <assimilate_state>%d</assimilate_state>\n"
        "  <workseq_next>%d</workseq_next>\n"
        "  <opaque>%f</opaque>\n"
        "  <min_quorum>%d</min_quorum>\n"
        "  <target_nresults>%d</target_nresults>\n"
        "  <max_error_results>%d</max_error_results>\n"
        "  <max_total_results>%d</max_total_results>\n"
        "  <max_success_results>%d</max_success_results>\n"
        "  <result_template_file>%s</result_template_file>\n",
        wu.create_time, 
        wu.appid,
        wu.name, 
        wu.xml_doc, 
        wu.batch,
        wu.rsc_fpops_est, 
        wu.rsc_fpops_bound, 
        wu.rsc_memory_bound, 
        wu.rsc_disk_bound,
        wu.need_validate,
        wu.canonical_resultid, 
        wu.canonical_credit,
        wu.transition_time, 
        wu.delay_bound,
        wu.error_mask, 
        wu.file_delete_state, 
        wu.assimilate_state,
        wu.workseq_next, 
        wu.opaque,
        wu.min_quorum,
        wu.target_nresults,
        wu.max_error_results,
        wu.max_total_results,
        wu.max_success_results,
        wu.result_template_file
    );

    fprintf(wu_stream,
        "</workunit_archive>\n"
    );
    return 0;
}

void cleanUpResources() {
    //close archive files
    fprintf(wu_stream,
        "</%s>\n",WU_FILENAME_PREFIX
    );
    fclose(wu_stream);

    fprintf(re_stream,
        "</%s>\n",RESULT_FILENAME_PREFIX
    );
    fclose(re_stream);
    
    //close DB connection
    boinc_db.close();
}

int purge_and_archive_results(DB_WORKUNIT& wu, int& number_results) {
    int retval= 0;
    DB_RESULT result;
    char buf[256];
    
    number_results=0;

    sprintf(buf, "where workunitid=%d", wu.id);
    while (!result.enumerate(buf)) {
       retval= archive_result(result);
       if (retval) return retval;
       log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Archived result [%d] to a file\n", result.id);
        
       retval= result.delete_from_db();
       if (retval) return retval;
       log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Purged result [%d] from database\n", result.id);

       number_results++;
    }
    return 0;
}


int main(int argc, char** argv) {
    int purged_workunits= 0, purged_results= 0;
    int retval= 0;
    
    retval= config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Starting DB Purger\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user,
    config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't open DB\n");
        exit(1);
    }
    log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Opened DB\n");

    mkdir("../archives", 0777);

    retval = open_archive(WU_FILENAME_PREFIX,wu_stream);
    retval = open_archive(RESULT_FILENAME_PREFIX,re_stream);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't open archives\n");
        exit(1);
    }
    
    bool did_something = false;
    DB_WORKUNIT wu;
    char buf[256];
    
    // select all workunits with file_delete_state='DONE'
    //
    sprintf(buf, "where file_delete_state=%d limit 1000", FILE_DELETE_DONE);
    int n=0;
    while (!wu.enumerate(buf)) {
        did_something = true;
        
        retval = purge_and_archive_results(wu, n);
        purged_results += n;
        
        retval= archive_wu(wu);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "Failed to write to XML file workunit:%d\n", wu.id
            );
            exit(1);
        }
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Archived workunit [%d] to a file\n", wu.id);
        
        //purge workunit from DB        
        retval= wu.delete_from_db();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Can't delete workunit [%d] from database:%d\n", wu.id, retval);
            exit(1);
        }
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,"Purged workunit [%d] from database\n", wu.id);

        purged_workunits++;
    }
    
    if (!did_something) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Did not do anything\n");
        exit(1);
    }
    
    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
        "Archived %d workunits and %d results\n",
        purged_workunits,purged_results
    );
}
