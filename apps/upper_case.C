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

// read stdin, convert to UC, write to stdout

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "api.h"

int main() {
    int c, n=0;
    APP_IN ai;
    APP_OUT ao;

    boinc_init(ai);
    fprintf(stderr, "APP: upper_case starting\n");
    fflush(stderr);
    while (1) {
        c = getchar();
        if (c == EOF) break;
        c = toupper(c);
        putchar(c);
        n++;
        if(time_to_checkpoint()) {
	    fflush(stdout);
            ao.percent_done = 1;
	    checkpoint_completed(ao);
        }
    }
    fprintf(stderr, "APP: upper_case ending, wrote %d chars\n", n);
    return 0;
}
