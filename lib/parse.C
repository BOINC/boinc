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

// A very crude interface for parsing XML files;
// assumes all elements are either single-line or
// have start and end tags on separate lines.
// This is meant to be used ONLY for parsing XML files produced
// by the BOINC scheduling server or client.
// Could replace this with a more general parser.

#ifdef _WIN32
#include <afxwin.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "parse.h"
#include "error_numbers.h"

// return true if the tag appears in the line
//
bool match_tag(char* buf, char* tag) {
    if (strstr(buf, tag)) return true;
    return false;
}

// parse an integer of the form <tag>1234</tag>
// return true if it's there
// Note: this doesn't check for the end tag
//
bool parse_int(char* buf, char* tag, int& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = strtol(p+strlen(tag), 0, 0);        // this parses 0xabcd correctly
    return true;
}

// Same, for doubles
//
bool parse_double(char* buf, char* tag, double& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = atof(p+strlen(tag));
    return true;
}

// parse a string of the form <tag>string</tag>
//
bool parse_str(char* buf, char* tag, char* dest, int len) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    p = strchr(p, '>');
    char* q = strchr(p+1, '<');
    *q = 0;
    strncpy(dest, p+1, len);
    dest[len-1] = 0;
    return true;
}

// parse a string of the form name="string"
//
void parse_attr(char* buf, char* name, char* dest, int len) {
    char* p, *q;

    strcpy(dest, "");
    p = strstr(buf, name);
    if (!p) return;
    p = strchr(p, '"');
    if (!p) return;
    q = strchr(p+1, '"');
    if (!q) return;
    *q = 0;
    strncpy(dest, p+1, len);
    dest[len-1] = 0;
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

// append to a malloc'd string
//
void strcatdup(char*& p, char* buf) {
    p = (char*)realloc(p, strlen(p) + strlen(buf)+1);
    if (!p) {
        fprintf(stderr, "strcatdup: realloc failed\n");
        exit(1);
    }
    strcat(p, buf);
}

// copy from a file to a malloc'd string until the end tag is reached
//
int dup_element_contents(FILE* in, char* end_tag, char** pp) {
    char buf[256];

    char* p = strdup("");
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            *pp = p;
            return 0;
        }
        strcatdup(p, buf);
    }
    fprintf(stderr, "dup_element_contents(): no end tag\n");
    return 1;
}

// read a file into a malloc'd string
//
int read_file_malloc(char* pathname, char*& str) {
    char buf[256];
    FILE* f;

    f = fopen(pathname, "r");
    if (!f) return -1;
    str = strdup("");
    while (fgets(buf, 256, f)) {
        strcatdup(str, buf);
    }
    fclose(f);
    return 0;
}

#if 0
// replace XML element contents.  not currently used
//
void replace_element(char* buf, char* start, char* end, char* replacement) {
    char temp[MAX_BLOB_SIZE], *p, *q;

    p = strstr(buf, start);
    p += strlen(start);
    q = strstr(p, end);
    strcpy(temp, q);
    strcpy(p, replacement);
    strcat(p, temp);
}
#endif
