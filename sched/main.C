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

#include "db.h"
#include "parse.h"
#include "shmem.h"
#include "server_types.h"
#include "handle_request.h"

#define REQ_FILE_PREFIX "/tmp/boinc_req_"
#define REPLY_FILE_PREFIX "/tmp/boinc_reply_"

int return_error(char* p) {
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
    char req_path[256], reply_path[256];
    SCHED_SHMEM* ssp;
    void* p;
    
#ifdef _USING_FCGI_

    while(FCGI_Accept() >= 0) {

#endif
    retval = attach_shmem(BOINC_KEY, &p);
    if (retval) {
        printf("can't attach shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    retval = ssp->verify();
    if (retval) {
        printf("shmem has wrong struct sizes - recompile\n");
        exit(1);
    }

    for (i=0; i<10; i++) {
        if (ssp->ready) break;
        fprintf(stderr, "Waiting for ready flag\n");
        sleep(1);
    }
    if (!ssp->ready) {
        fprintf(stderr, "handle_request(): feeder doesn't seem to be running\n");
        exit(1);
    }
    //fprintf(stderr, "got ready flag\n");

    pid = getpid();
    sprintf(req_path, "%s%d", REQ_FILE_PREFIX, pid);
    sprintf(reply_path, "%s%d", REPLY_FILE_PREFIX, pid);

    fprintf(stdout, "Content-type: text/plain\n\n");

    fout = fopen(req_path, "w");
    if (!fout) {
        exit(return_error("can't write request file"));
    }
    copy_stream(stdin, fout);
    fclose(fout);
    fin = fopen(req_path, "r");
    if (!fin) {
        exit(return_error("can't read request file"));
    }
    fout = fopen(reply_path, "w");
    if (!fout) {
        exit(return_error("can't write reply file"));
    }

    retval = db_open("boinc");
    if (retval) {
        exit(return_error("can't open database"));
    }
    handle_request(fin, fout, *ssp);
    db_close();

    fclose(fin);
    fclose(fout);
    fin = fopen(reply_path, "r");
    if (!fin) {
        exit(return_error("can't read reply file"));
    }
    copy_stream(fin, stdout);
    fclose(fin);

    unlink(req_path);
    unlink(reply_path);
#ifdef _USING_FCGI_

    }
#endif
}
