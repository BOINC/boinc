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

#include "error_numbers.h"
#include "file_names.h"
#include "client_types.h"
#include "parse.h"

#include "accounts.h"

ACCOUNTS::ACCOUNTS() {
}

int ACCOUNTS::parse_file() {
    char buf[256];
    PROJECT* project;
    FILE* f = fopen(ACCOUNT_FILE_NAME, "r");

    if (!f) {
        fprintf(stderr, "Can't open accounts file: %s\n", ACCOUNT_FILE_NAME);
        return ERR_FOPEN;
    }
    fgets(buf, 256, f);
    if (!match_tag(buf, "<accounts>")) return ERR_XML_PARSE;
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</accounts>")) {
            return 0;
        } else if (match_tag(buf, "<project>")) {
            project = new PROJECT;
            project->parse(f);
            projects.push_back(project);
        } else {
            fprintf(stderr, "ACCOUNTS::parse_file(): unrecognized: %s\n", buf);
        }
    }
    return ERR_XML_PARSE;
}
