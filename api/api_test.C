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

// test program for MFILE class

#include "api.h"

int main() {
    MFILE mf;
    unsigned long int i = 0;
    int temp=0;
    APP_IN ai;
    boinc_init(ai);
    mf.open("foobar", "w");
    mf.printf("blah %d %f\n", 17, 34.5);
    mf.printf("foo\n");
    for(; i<100000000; i++) {
	if(time_to_checkpoint()) {
	    mf.printf("checkpoint\n");
	    mf.flush();
	    checkpoint_completed();
	}
	temp++;
    }
    mf.close();
    return 0;
}
