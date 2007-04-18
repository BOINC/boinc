// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// sample_work_generator.C: an example BOINC work generator.
// This work generator has the following properties
// (you may need to change some or all of these):
//
// - Runs as a daemon, and creates an unbounded supply of work.
//   It attempts to maintain a "cushion" of 100 unsent job instances.
//   (your app may not work this way; e.g. you might create work in batches)
// - Creates work for the application "uppercase".
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so that they're unique.

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"

#include "sched_msgs.h"
#include "sched_config.h"
#include "sched_util.h"

#define CUSHION 100
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  2

// globals
//
char* wu_template;
DB_APP app;
int start_time;
int seqno;
SCHED_CONFIG config;

// create one new job
//
int make_job() {
    DB_WORKUNIT wu;
    char name[256], path[256], buf[256];
    const char* infiles[1];

    // make a unique name (for the job and its input file)
    //
    sprintf(name, "uc_%d_%d", start_time, seqno++);

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    config.upload_path(name, path);
    FILE* f = fopen(path, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f, "This is the input file for job %s", name);
    fclose(f);

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
    wu.target_nresults = REPLICATION_FACTOR;
    infiles[0] = path;

    // Register the job with BOINC
    //
    return create_work(
        wu,
        wu_template,
        "result_uppercase.xml",
        "../templates/results_uppercase.xml",
        infiles,
        1,
        config
    );
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n);
        if (n > CUSHION) {
            sleep(60);
        } else {
            int njobs = n/REPLICATION_FACTOR;
            for (int i=0; i<njobs; i++) {
                retval = make_job();
            }
        }
    }
}

int main() {
    int retval;

    if (config.parse_file("..")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "can't read config file\n"
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't open db\n");
        exit(1);
    }
    if (app.lookup("where name='uppercase'")) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't find app\n");
        exit(1);
    }
    if (read_file_malloc("../templates/wu_uppercase.xml", wu_template)) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't read template\n");
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    main_loop();
}
