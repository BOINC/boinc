// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

// result_retry - create new results to make up for lost ones
//
// result_retry
//   -app appname
//   [ -nerror n ]          if get this many errors, bail on WU
//   [ -ndet n ]            if get this results w/o consensus, bail
//   [ -nredundancy n ]     try to get at least this many done results
//   [ -asynch ]            be asynchronous

using namespace std;

#include <vector>
#include <unistd.h>
#include <sys/time.h>

#include "db.h"
#include "backend_lib.h"
#include "config.h"

#define TRIGGER_FILENAME      "stop_server"

int max_errors = 999;
int max_done = 999;
int nredundancy = 0;
int startup_time;
CONFIG config;
R_RSA_PRIVATE_KEY key;
char app_name[256];

void check_trigger() {
    FILE* f = fopen(TRIGGER_FILENAME, "r");
    if (!f) return;
    exit(0);
}

void write_log(char* p) {
    time_t now = time(0);
    char* timestr = ctime(&now);
    *(strchr(timestr, '\n')) = 0;
    fprintf(stderr, "%s: %s", timestr, p);
}

// The scheme for generating unique output filenames is as follows.
// If the original filename is of the form x__y,
// then y is replaced with a string of the form time_seqno,
// where "time" is when this program started up.
// NOTE: if you ever need to start up multiple copies of this,
// you'll need to add a PID in there somewhere.
//
// If the original filename doesn't have __, add a string
// of the form __time_seqno

void make_unique_name(char* name) {
    char buf[256], *p;
    static int seqno;

    sprintf(buf, "%d_%d", startup_time, seqno++);
    p = strstr(name, "__");
    if (p) {
        strcpy(p+2, buf);
    } else {
        strcat(name, "__");
        strcat(name, buf);
    }
}

// convert a result's XML document to generate new output filenames.
// The input has the form
// <file_info>
//    <name>xxx</name>
//    ...
// </file_info>
// ...
// <result>
//    <file_ref>
//       <file_name>xxx</file_name>
//       ...
//    </file_ref>
//    ...
// </result>
//
// Look for <name>...</name> elements within <file_info>
// and make a unique name based on it;
// apply the same conversion to the <file_name> element later on.
//
// TODO: this is ad-hoc.  Would be nice to use some generic
// XML parsing routines, or XSLT or something.
//
int assign_new_names(char* in) {
    char *p = in, *n1, *n2, *r;
    char name[256], newname[256], element[256], buf[MAX_BLOB_SIZE];
    int len;

    // notice where the <result> is so we don't try to convert
    // the result name
    //
    r = strstr(in, "<result>");

    while (1) {
        n1 = strstr(p, "<name>");
        if (!n1) break;

        if (n1 > r) break;      // don't go past <result>

        n1 += strlen("<name>");
        n2 = strstr(p, "</name>");
        if (!n2) {
            sprintf(buf, "assign_new_names(): malformed XML:\n%s", in);
            write_log(buf);
            return 1;
        }
        len = n2 - n1;
        memcpy(name, n1, len);
        name[len] = 0;
        strcpy(newname, name);
        make_unique_name(newname);
        strcpy(buf, n2);
        strcpy(n1, newname);
        strcat(n1, buf);

        // replace the name in the <file_name> element
        //
        sprintf(element, "<file_name>%s</file_name>", name);
        n2 = strstr(n1, element);
        if (!n2) {
            sprintf(buf, "assign_new_names(): no <file_name>:\n%s", in);
            write_log(buf);
            return 1;
        }
        strcpy(buf, n2+strlen(element));
        sprintf(element, "<file_name>%s</file_name>", newname);
        strcpy(n2, element);
        strcat(n2, buf);
        p = n1;
    }
    return 0;
}

bool do_pass(APP& app) {
    WORKUNIT wu;
    RESULT result;
    int nerrors, ndone, retval;
    unsigned int i, n;
    bool did_something = false;
    char buf[256];

    wu.retry_check_time = time(0);
    wu.appid = app.id;

    // loop over WUs that are due to be checked
    //
    while (!db_workunit_enum_retry_check_time(wu)) {
        vector<RESULT> results;

        did_something = true;
        // if this WU has a canonical result, we're done
        // (this normally doesn't happen since the retry check time
        // is zeroed when canonical result found, but just in case).
        //
        if (wu.canonical_resultid) {
            wu.retry_check_time = 0;
            goto update_wu;
        }

        // enumerate all the results for the WU
        //
        result.workunitid = wu.id;
        while (!db_result_enum_wuid(result)) {
            results.push_back(result);
        }

        nerrors = 0;
        ndone = 0;
        for (i=0; i<results.size(); i++) {
            result = results[i];

            // if any result is unsent, give up on the WU
            //
            if (result.server_state == RESULT_SERVER_STATE_UNSENT) {
                sprintf(buf, "WU %s has unsent result\n", wu.name);
                write_log(buf);
                wu.main_state = WU_MAIN_STATE_ERROR;
                wu.error = SEND_FAIL;
                wu.file_delete_state = FILE_DELETE_READY;
                wu.assimilate_state = ASSIMILATE_READY;
                wu.retry_check_time = 0;
                goto update_wu;
            }
            if (result.server_state == RESULT_SERVER_STATE_ERROR) {
                nerrors++;
            }
            if (result.server_state == RESULT_SERVER_STATE_DONE) {
                ndone++;
            }
        }

        // it too many errors or too many different results, bail
        //
        if (nerrors > max_errors) {
            sprintf(buf, "WU %s has too many errors\n", wu.name);
            write_log(buf);
            wu.main_state = WU_MAIN_STATE_ERROR;
            wu.error = TOO_MANY_ERRORS;
            wu.file_delete_state = FILE_DELETE_READY;
            wu.assimilate_state = ASSIMILATE_READY;
            wu.retry_check_time = 0;
            goto update_wu;
        }
        if (ndone > max_done) {
            sprintf(buf, "WU %s has too many answers\n", wu.name);
            write_log(buf);
            wu.main_state = WU_MAIN_STATE_ERROR;
            wu.error = TOO_MANY_DONE;
            wu.file_delete_state = FILE_DELETE_READY;
            wu.assimilate_state = ASSIMILATE_READY;
            wu.retry_check_time = 0;
            goto update_wu;
        }

        // Generate new results if needed.
        // Munge the XML of an existing result
        // to create unique new output filenames.
        //
        if (nredundancy > ndone) {
            n = nredundancy - ndone;
            
	    for (i=0; i<n; i++) {
                result = results[0];
                make_unique_name(result.name);
                initialize_result(result, wu);
                remove_signatures(result.xml_doc_in);
                assign_new_names(result.xml_doc_in);
                add_signatures(result.xml_doc_in, key);
                retval = db_result_new(result);
                if (retval) {
                    sprintf(buf, "db_result_new %d\n", retval);
                    write_log(buf);
                    break;
                }
            }
        }

        // update the WU's result retry check time
        //
        wu.retry_check_time = time(0) + wu.delay_bound;
update_wu:
        retval = db_workunit_update(wu);
        if (retval) {
            sprintf(buf, "db_workunit_update %d\n", retval);
            write_log(buf);
        }
    }
    return did_something;
}

void main_loop(bool one_pass) {
    APP app;
    bool did_something;
    int retval;
    char buf[256];

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        sprintf(buf, "db_open: %d\n", retval);
        write_log(buf);
        exit(1);
    }

    strcpy(app.name, app_name);
    retval = db_app_lookup_name(app);
    if (retval) {
        sprintf(buf, "can't find app %s\n", app.name);
        write_log(buf);
        exit(1);
    }

    while (1) {
        did_something = do_pass(app);
        if (one_pass) break;
        if (!did_something) sleep(1);
        check_trigger();
    }
}

int main(int argc, char** argv) {
    int i, retval;
    bool asynch = false, one_pass=false;
    char path[256];

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "can't read config file\n");
        exit(1);
    }

    sprintf(path, "%s/upload_private", config.key_dir);
    retval = read_key_file(path, key);
    if (retval) {
        fprintf(stderr, "can't read key\n");
        exit(1);
    }

    startup_time = time(0);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-app")) {
            strcpy(app_name, argv[++i]);
        } else if (!strcmp(argv[i], "-nerror")) {
            max_errors = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-ndet")) {
            max_done = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-nredundancy")) {
            nredundancy = atoi(argv[++i]);;
        }
    }
    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }
    main_loop(one_pass);
}
