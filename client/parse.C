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

#include <string.h>
#include <stdlib.h>

#include "parse.h"

bool match_tag(char* buf, char* tag) {
    if (strstr(buf, tag)) return true;
    return false;
}

bool parse_int(char* buf, char* tag, int& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = atoi(p+strlen(tag));
    return true;
}

bool parse_double(char* buf, char* tag, double& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = atof(p+strlen(tag));
    return true;
}

bool parse_str(char* buf, char* tag, char* x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    char* q = strchr(p+1, '<');
    *q = 0;
    strcpy(x, p+strlen(tag));
    return true;
}

void copy_stream(FILE* in, FILE* out) {
    char buf[1024];
    int n, m;

    while (1) {
        n = fread(buf, 1, 1024, in);
        m = fwrite(buf, 1, n, out);
        if (n < 1024) break;
    }
}

