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

// read "in", convert to UC, write to "out"
//
// This version does one char/second,
// and writes its state to disk every so often

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "api.h"

#define CHECKPOINT_FILE "uc_slow_state"

int do_checkpoint(MFILE& mf, int nchars) {
    int retval;
    char resolved_name[512],res_name2[512];

    boinc_resolve_link( "temp", resolved_name );
    FILE* f = fopen(resolved_name, "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    fprintf(stderr, "APP: uc_slow checkpointing\n");

    // hopefully atomic part starts here
    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_link( CHECKPOINT_FILE, res_name2 );
    retval = rename(resolved_name, res_name2);
    if (retval) return retval;
    // hopefully atomic part ends here

    return 0;
}

int main() {
    int c, nchars = 0, retval, n, i;
    double j;
    char resolved_name[512];
    MFILE out, time_file;
    FILE* state, *in;
    APP_IN ai;
    APP_OUT ao;

    boinc_init( ai );

    boinc_resolve_link( "in", resolved_name );
    in = fopen(resolved_name, "r");
    boinc_resolve_link( CHECKPOINT_FILE, resolved_name );
    state = fopen(resolved_name, "r");
    if (state) {
        fscanf(state, "%d", &nchars);
        printf("nchars %d\n", nchars);
        fseek(in, nchars, SEEK_SET);
        boinc_resolve_link( "out", resolved_name );
        retval = out.open(resolved_name, "a");
    } else {
        boinc_resolve_link( "out", resolved_name );
        retval = out.open(resolved_name, "w");
    }
    fprintf(stderr, "APP: uc_slow starting\n");
    if (retval) {
        fprintf(stderr, "APP: uc_slow output open failed %d\n", retval);
        exit(1);
    }
    time_file.open("../../time.xml", "w");
    while (1) {
        c = fgetc(in);
        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
        nchars++;

        n = 0;
	j = 3.14159;
        for(i=0; i<10000000; i++) {
	    n++;
	    j *= n+j-3.14159;
	    j /= (float)n;
        }

        if (time_to_checkpoint()) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: uc_slow checkpoint failed %d\n", retval);
            }
            ao.percent_done = 1;
            checkpoint_completed(ao);
        }
    }
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "APP: uc_slow flush failed %d\n", retval);
        exit(1);
    }
    fprintf(stderr, "APP: uc_slow ending, wrote %d chars\n", nchars);
    ao.percent_done = 1;
    app_completed(ao);
    time_file.printf("%f\n", ao.cpu_time_at_checkpoint);
    time_file.flush();
    time_file.close();
    return 0;
}
