#include <string.h>
#include <unistd.h>

#include "db.h"
#include "parse.h"
#include "config.h"

CONFIG config;

int wu_delete_files(WORKUNIT& wu) {
    char* p;
    char filename[256], pathname[256];
    bool no_delete;

    p = strtok(wu.xml_doc, "\n");
    strcpy(filename, "");
    while (p) {
        p = strtok(0, "\n");
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
            continue;
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                sprintf(pathname, "%s/%s", config.download_dir, filename);
                unlink(pathname);
            }
        }
    }
    return 0;
}

int result_delete_files(RESULT& result) {
    char* p;
    char filename[256], pathname[256];
    bool no_delete;

    p = strtok(result.xml_doc_in, "\n");
    while (p) {
        p = strtok(0, "\n");
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
            continue;
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                sprintf(pathname, "%s/%s", config.upload_dir, filename);
                unlink(pathname);
            }
        }
    }
    return 0;
}

// return nonzero if did anything
//
bool do_pass() {
    WORKUNIT wu;
    RESULT result;
    bool did_something = false;

    wu.file_delete_state = FILE_DELETE_READY;
    while (db_workunit_enum_file_delete_state(wu)) {
        did_something = true;
        wu_delete_files(wu);
        wu.file_delete_state = FILE_DELETE_DONE;
        db_workunit_update(wu);
    }

    result.file_delete_state = FILE_DELETE_READY;
    while (db_result_enum_file_delete_state(result)) {
        did_something = true;
        result_delete_files(result);
        result.file_delete_state = FILE_DELETE_DONE;
        db_result_update(result);
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    int i;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else {
            fprintf(stderr, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    retval = db_open(config.db_name, config.db_passwd);
    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(10);
        }
    }
}
