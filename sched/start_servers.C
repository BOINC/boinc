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
#include <stdlib.h>

#include "config.h"

int main() {
    CONFIG config;
    int retval;

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't read config\n");
        exit(1);
    }

    if (config.start_assimilator) {
        system("assimilator >> assimilator.out 2>&1");
    }
    if (config.start_feeder) {
        system("feeder >> feeder.out 2>&1");
    }
    if (config.start_file_deleter) {
        system("file_deleter >> file_deleter.out 2>&1");
    }
    if (config.start_make_work) {
        system("make_work >> make_work.out 2>&1");
    }
    if (config.start_result_retry) {
        system("result_retry >> result_retry.out 2>&1");
    }
    if (config.start_validate) {
        system("validate >> validate.out 2>&1");
    }
}
