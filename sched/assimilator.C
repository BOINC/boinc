#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "db.h"
#include "parse.h"
#include "config.h"

CONFIG config;

void write_log(char* p) {
    time_t now = time(0);
    char* timestr = ctime(&now);
    *(strchr(timestr, '\n')) = 0;
    fprintf(stderr, "%s: %s", timestr, p);
}

// return nonzero if did anything
//
bool do_pass(APP app) {
    WORKUNIT wu;
    RESULT result;
    bool did_something = false;
    int retval;

    wu.appid = app.id;
    wu.assimilate_state = ASSIMILATE_READY;
    while (!db_workunit_enum_app_assimilate_state(wu)) {
        did_something = true;
        switch(wu.main_state) {
        case WU_MAIN_STATE_INIT:
            write_log("ERROR; WU shouldn't be in init state\n");
            break;
        case WU_MAIN_STATE_DONE:
            if (!wu.canonical_resultid) {
                write_log("ERROR: canonical resultid zero\n");
                break;
            }
            retval = db_result(wu.canonical_resultid, result);
            if (retval) {
                write_log("can't get canonical result\n");
                break;
            }
            printf("canonical result for WU %s:\n%s", wu.name, result.xml_doc_out);
            result.file_delete_state = FILE_DELETE_READY;
            db_result_update(result);
            break;
        case WU_MAIN_STATE_ERROR:
            printf("WU %s had an error\n", wu.name);
            break;
        }
        wu.assimilate_state = ASSIMILATE_DONE;
        db_workunit_update(wu);
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    APP app;
    int i;
    char buf[256];

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-app")) {
            strcpy(app.name, argv[++i]);
        } else {
            sprintf(buf, "Unrecognized arg: %s\n", argv[i]);
            write_log(buf);
        }
    }

    retval = config.parse_file();
    if (retval) {
        write_log("Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        write_log("Can't open DB\n");
        exit(1);
    }
    retval = db_app_lookup_name(app);
    if (retval) {
        write_log("Can't find app\n");
        exit(1);
    }
    if (one_pass) {
        do_pass(app);
    } else {
        while (1) {
            if (!do_pass(app)) sleep(10);
        }
    }
}
