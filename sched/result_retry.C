// result_retry - create new results to make up for lost ones
//
// result_retry
//   [ -dwu n ]
//   [ -dresult n ]
//   [ -nerror n ]
//   [ -ndet n ]
//   [ -nredundancy n ]

#include <vector>
#include <sys/time.h>

#include "db.h"

int max_errors = 999;
int max_done = 999;
int nredundancy = 999;
int startup_time;

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

    sprintf(buf, "%d_%d", startup_time, seqno);
    p = strstr(name, "__");
    if (p) {
        strcpy(p+2, buf);
    } else {
        strcat(name, buf);
    }
}

// convert a result's XML document to generate new output filenames
// Look for <name>...</name> elements and convert the name;
// apply the same conversion to the <file_name> element later on.
//
int assign_new_names(char* in, char* out) {
    char *p = in, *n1, *n2;
    char name[256], buf[MAX_BLOB_SIZE];
    int len;

    while (1) {
        n1 = strstr(p, "<name>");
        if (!n1) break;
        n1 += strlen("<name>");
        n2 = strstr(p, "</name>");
        if (!n2) {
            fprintf(stderr, "malformed XML:\n%s", in);
            return 1;
        }
        len = n2 - n1;
        memcpy(name, n1, len);
        name[len] = 0;
        make_unique_name(name);
        strcpy(buf, n2);
        strcpy(n1, name);
        strcat(n1, buf);
        p = n1;
    }
    return 0;
}

void main_loop() {
    WORKUNIT wu;
    RESULT result;
    int nerrors, ndone;
    unsigned int i, n;

    wu.retry_check_time = time(0);

    // loop over WUs that are due to be checked
    //
    while (db_workunit_enum_retry_check_time(wu)) {
        vector<RESULT> results;

        // enumerate all the results for the WU
        //
        result.workunitid = wu.id;
        while (db_result_enum_wuid(result)) {
            results.push_back(result);
        }

        nerrors = 0;
        ndone = 0;
        for (i=0; i<results.size(); i++) {
            result = results[i];

            // if any result is unsent, give up on the WU
            //
            if (result.state == RESULT_STATE_UNSENT) {
                fprintf(stderr, "WU %s has unsent result\n", wu.name);
                wu.state = WU_STATE_SEND_FAIL;
                db_workunit_update(wu);
                goto next_wu;
            }
            if (result.state == RESULT_STATE_ERROR) {
                nerrors++;
            }
            if (result.state == RESULT_STATE_DONE) {
                ndone++;
            }
        }

        // it too many errors or too many different results, bail
        //
        if (nerrors > max_errors) {
            fprintf(stderr, "WU %s has too many errors\n", wu.name);
            wu.state = WU_STATE_TOO_MANY_ERRORS;
            db_workunit_update(wu);
            goto next_wu;
        }
        if (ndone > max_done) {
            fprintf(stderr, "WU %s has too many answers\n", wu.name);
            wu.state = WU_STATE_TOO_MANY_DONE;
            db_workunit_update(wu);
            goto next_wu;
        }

        // Generate new results if needed.
        // Munge the XML of an existing result
        // to create unique new output filenames.
        //
        n = nredundancy - ndone;
        for (i=0; i<n; i++) {
            create_result(
                wu, result_template_file, suffix, key,
                config.upload_url, config.download_url
            );
        }
next_wu:
    }
}

int main(int argc, char** argv) {
    int i;
    bool asynch = false;

    startup_time = time(0);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-nerror")) {
            max_errors = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-ndet")) {
            max_done = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-nredundancy")) {
            nredundancy = atoi(argv[++i]);;
        }
    }
    if (asynch) {
        if (fork()==0) {
            while(1) {
                main_loop();
            }
        }
    } else {
        while (1) {
            main_loop();
        }
    }
}
