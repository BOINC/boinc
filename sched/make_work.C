// make_work.C
//
// Create result records as needed to maintain a pool to send
//
// This reads a result record from the DB, then makes clones of it.
// Assumes the result has a single output file,
// so overwrites the first <name> element with a new name

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "db.h"
#include "config.h"

#define TRIGGER_FILENAME    "stop_server"

void check_trigger() {
    FILE* f = fopen(TRIGGER_FILENAME, "r");
    if (!f) return;
    exit(0);
}

void replace_element(char* buf, char* start, char* end, char* replacement) {
    char temp[MAX_BLOB_SIZE], *p, *q;

    p = strstr(buf, start);
    p += strlen(start);
    q = strstr(p, end);
    strcpy(temp, q);
    strcpy(p, replacement);
    strcat(p, temp);
}

void make_work() {
    CONFIG config;
    RESULT result;
    int retval, i=time(0), n;
    char buf[256];

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "make_work: can't read config file\n");
        exit(1);
    }

    retval = db_open(config.db_name, config.db_passwd);
    if (retval) {
        fprintf(stderr, "make_work: can't open db\n");
        exit(1);
    }

    retval = db_result(1, result);
    if (retval) {
        fprintf(stderr, "make_work: can't read result\n");
        exit(1);
    }

    while (true) {
        fflush(stdout);
        retval = db_result_count_state(RESULT_STATE_UNSENT, n);
        if (retval) {
            fprintf(stderr, "make_work: can't counts results\n");
            exit(1);
        }
        printf("make_work: %d results\n", n);
        if (n > 10) {
            sleep(1);
            continue;
        }
        result.id = 0;
        result.create_time = time(0);
        sprintf(result.name, "result_%d", i++);
        result.state = RESULT_STATE_UNSENT;
        result.validate_state = VALIDATE_STATE_INITIAL;
        replace_element(result.xml_doc_in, "<name>", "</name>", result.name);
        replace_element(result.xml_doc_in, "<file_name>", "</file_name>", result.name);
        retval = db_result_new(result);
        if (retval) {
            fprintf(stderr, "make_work: can't create result\n");
            exit(1);
        }
        printf("make_work: added a result\n");
    }
}

int main(int argc, char** argv) {
    bool asynch = false;
    int i;

    unlink(TRIGGER_FILENAME);
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        }
    }

    if (asynch) {
        if (!fork()) {
            make_work();
        }
    } else {
        make_work();
    }
}
