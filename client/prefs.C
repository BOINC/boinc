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
#include <string.h>

#include "parse.h"
#include "error_numbers.h"

#include "prefs.h"

PREFS::PREFS() {
    memset(this, 0, sizeof(PREFS));
};

int PREFS::parse(FILE* in) {
    char buf[256];

    memset(this, 0, sizeof(PREFS));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</prefs>")) return 0;
        else if (parse_int(buf, "<modified_time>", modified_time)) continue;
        else if (parse_int(buf, "<create_time>", create_time)) continue;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (match_tag(buf, "<stop_on_batteries/>")) {
            stop_on_batteries = true;
            continue;
        }
        else if (match_tag(buf, "<stop_user_active/>")) {
            stop_user_active = true;
            continue;
        }
        fprintf(stderr, "PREFS::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int PREFS::write(FILE* out) {
    fprintf(out,
        "<prefs>\n"
        "    <modified_time>%d</modified_time>\n"
        "    <create_time>%d</create_time>\n"
        "    <name>%s</name>\n",
        modified_time, create_time, name
    );
    if (stop_on_batteries) fprintf(out, "    <stop_on_batteries/>\n");
    if (stop_user_active) fprintf(out, "    <stop_on_user_active/>\n");
    fprintf(out, "</prefs>\n");
    return 0;
}
