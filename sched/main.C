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


#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "db.h"
#include "parse.h"
#include "shmem.h"
#include "server_types.h"
#include "handle_request.h"

#define REQ_FILE_PREFIX "/tmp/boinc_req_"
#define REPLY_FILE_PREFIX "/tmp/boinc_reply_"

int return_error(char* p) {
    assert(p!=NULL);
    fprintf(stderr, "BOINC server: %s\n", p);
    printf("<error_msg>%s</error_msg>\n", p);
    return 1;
}

// the code below is convoluted because,
// instead of going from stdin to stdout directly,
// we go via a pair of disk files
// (this makes it easy to save the input,
// and to know the length of the output).
//
int main() {
    FILE* fin, *fout;
    int i, retval, pid;
    char req_path[256], reply_path[256], path[256];
    SCHED_SHMEM* ssp;
    void* p;
    unsigned int counter=0;
    char* code_sign_key;

    sprintf(path, "%s/code_sign_public", BOINC_KEY_DIR);
    retval = read_file_malloc(path, code_sign_key);
    if (retval) {
        fprintf(stderr,
            "BOINC scheduler - compiled by BOINC_USER: can't read code sign key file (%s)\n", path
        );
        exit(1);
    }

    retval = attach_shmem(BOINC_KEY, &p);
    if (retval) {
        fprintf(stderr, "BOINC scheduler - Compiled by BOINC_USER: can't attach shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    if (retval) {
        fprintf(stderr, "BOINC scheduler - Compiled by BOINC_USER: shmem has wrong struct sizes - recompile\n");
        exit(1);
    }

    for (i=0; i<10; i++) {
        if (ssp->ready) break;
        fprintf(stderr, "BOINC scheduler - Compiled by BOINC_USER: waiting for ready flag\n");
        sleep(1);
    }
    if (!ssp->ready) {
        fprintf(stderr, "BOINC scheduler - Compiled by BOINC_USER: feeder doesn't seem to be running\n");
        exit(1);
    }
    //fprintf(stderr, "got ready flag\n");
    retval = db_open(BOINC_DB_NAME);
    if (retval) {
        exit(return_error("BOINC scheduler - Compiled by BOINC_USER: can't open database"));
    }
    pid = getpid();
#ifdef _USING_FCGI_
    while(FCGI_Accept() >= 0) {
    counter++;
#endif
    sprintf(req_path, "%s%d_%u", REQ_FILE_PREFIX, pid, counter);
    sprintf(reply_path, "%s%d_%u", REPLY_FILE_PREFIX, pid, counter);
    fprintf(stdout, "Content-type: text/plain\n\n");
    fout = fopen(req_path, "w");
    if (!fout) {
        exit(return_error("Compiled by BOINC_USER: can't write request file"));
    }
    copy_stream(stdin, fout);
    fclose(fout);
    fin = fopen(req_path, "r");
    if (!fin) {
        exit(return_error("Compiled by BOINC_USER: can't read request file"));
    }
    fout = fopen(reply_path, "w");
    if (!fout) {
        exit(return_error("Compiled by BOINC_USER: can't write reply file"));
    }
    handle_request(fin, fout, *ssp, code_sign_key);
    fclose(fin);
    fclose(fout);
    fin = fopen(reply_path, "r");
    if (!fin) {
        exit(return_error("Compiled by BOINC_USER: can't read reply file"));
    }
    copy_stream(fin, stdout);
    fclose(fin);
    unlink(req_path);
    unlink(reply_path);
#ifdef _USING_FCGI_
    }
#endif
    db_close();
}
