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

// concat [-run_slow] file1 ... filen outfile
//
// concatenate files, write to outfile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filesys.h"
#include "boinc_api.h"
#include "util.h"

int run_slow = 0;

#define CHECKPOINT_FILE "concat_state"

int do_checkpoint(MFILE& mf, int filenum, int nchars) {
    int retval;
    char res_name2[512];

    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d %d", filenum, nchars);
    fclose(f);

    fprintf(stderr, "APP: concat checkpointing\n");

    boinc_resolve_filename(CHECKPOINT_FILE, res_name2, sizeof(res_name2));

    retval = mf.flush();
    if (retval) return retval;
    retval = boinc_rename("temp", res_name2);
    if (retval) return retval;

    return 0;
}

void file_append(FILE* in, MFILE &out, int skip, int filenum) {
    char buf[1];
    int n,nread,retval;

    fseek(in, skip, SEEK_SET);
    nread = skip;

    while (1) {
        n = fread(buf, 1, 1, in);
        if (n == 0) break;
        out.write(buf, 1, n);
        nread += n;

        if (boinc_time_to_checkpoint()) {
            fprintf(stderr, "Checkpoint.\n");
            retval = do_checkpoint(out, filenum, nread);
            if (retval) {
                fprintf(stderr, "APP: concat checkpoint failed %d\n", retval);
                exit(1);
            }
            boinc_checkpoint_completed();
        }

        if (run_slow) boinc_sleep(1.);
    }
}

#ifdef _WIN32
#include <afxwin.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPWSTR command_line;
    LPWSTR *args;
    char* argv[100];
    int i, argc;

    command_line = GetCommandLineW();
    args = CommandLineToArgvW(command_line, &argc);

    // uh, why did MS have to "improve" on char*?

    for (i=0; i<argc; i++) {
        argv[i] = (char*)args[i];
    }
    return main(argc, argv);
}
#endif

int main(int argc, char** argv) {
    FILE* in, *state;
    MFILE out;
    char file_name[512];
    int i;
    int file_num,nchars,retval;
    char *mode;

    boinc_init();
    fprintf(stderr, "APP: concat: starting, argc %d\n", argc);

    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: concat: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "-run_slow")) run_slow = 1;
    }
    boinc_resolve_filename(CHECKPOINT_FILE, file_name, sizeof(file_name));
    state = fopen(file_name, "r");
    if (state) {
        fscanf(state, "%d %d", &file_num, &nchars);
        mode = "a";
    } else {
        file_num = (run_slow ? 2 : 1);
        nchars = 0;
        mode = "w";
    }
    boinc_resolve_filename(argv[argc-1], file_name, sizeof(file_name));
    fprintf(stderr, "res: %s\n", file_name);
    retval = out.open(file_name, mode);
    if (retval) {
        fprintf(stderr, "APP: concat: can't open out file %s\n", argv[argc-1]);
        exit(1);
    }
    for (i=file_num; i<argc-1; i++) {
        boinc_resolve_filename(argv[i], file_name, sizeof(file_name));
        fprintf(stderr, "res: %s\n", file_name);
        in = fopen(file_name, "r");
        if (!in) {
            fprintf(stderr, "APP: concat: can't open in file %s\n", argv[i]);
            exit(1);
        }
        file_append(in, out, nchars, i);
        nchars = 0;
        fclose(in);
    }
    out.close();
    fprintf(stderr, "APP: concat: done\n");
    boinc_finish(0);
    return 0;
}
