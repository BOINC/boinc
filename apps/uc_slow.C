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

int checkpoint(MFILE& mf, int nchars) {
    int retval;
    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    fprintf(stderr, "APP: uc_slow checkpointing\n");

    // hopefully atomic part starts here
    retval = mf.flush();
    if (retval) return retval;
    retval = rename("temp", CHECKPOINT_FILE);
    if (retval) return retval;
    // hopefully atomic part ends here

    return 0;
}

int main() {
    int c, nchars = 0, retval;
    MFILE out;
    FILE* state, *in;

    in = fopen("in", "r");
    state = fopen("uc_slow_state", "r");
    if (state) {
        fscanf(state, "%d", &nchars);
        printf("nchars %d\n", nchars);
        fseek(in, nchars, SEEK_SET);
        retval = out.open("out", "a");
    } else {
        retval = out.open("out", "w");
    }
    fprintf(stderr, "APP: uc_slow starting\n");
    if (retval) {
        fprintf(stderr, "APP: uc_slow output open failed %d\n", retval);
        exit(1);
    }
    while (1) {
        c = fgetc(in);
        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
        nchars++;
        sleep(1);

        if (nchars%5 == 0) {
            retval = checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: uc_slow checkpoint failed %d\n", retval);
                exit(1);
            }
        }
    }
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "APP: uc_slow flush failed %d\n", retval);
        exit(1);
    }
    fprintf(stderr, "APP: uc_slow ending, wrote %d chars\n", nchars);
}
