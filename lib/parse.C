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

// A very crude interface for parsing XML files;
// assumes all elements are either single-line or
// have start and end tags on separate lines.
// This is meant to be used ONLY for parsing XML files produced
// by the BOINC scheduling server or client.
// Could replace this with a more general parser.

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <string.h>
#include <stdlib.h>
#include <string>
using namespace std;
#endif

#include "error_numbers.h"
#include "util.h"
#include "parse.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

// return true if the tag appears in the line
//
bool match_tag(const char* buf, const char* tag) {
    if (strstr(buf, tag)) return true;
    return false;
}

bool match_tag(const std::string &s, const char* tag) {
    return match_tag(s.c_str(), tag);
}

// parse an integer of the form <tag>1234</tag>
// return true if it's there
// Note: this doesn't check for the end tag
//
bool parse_int(const char* buf, const char* tag, int& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = strtol(p+strlen(tag), 0, 0);        // this parses 0xabcd correctly
    return true;
}

// Same, for doubles
//
bool parse_double(const char* buf, const char* tag, double& x) {
    char* p = strstr(buf, tag);
    if (!p) return false;
    x = atof(p+strlen(tag));
    return true;
}

// parse a string of the form ...<tag attrs>string</tag>...;
// returns the "string" part.
// Does XML unescaping (replace &lt; with <)
// "string" may not include '<'
// Strips white space from ends.
// Use "<tag", not "<tag>", if there might be attributes
//
bool parse_str(const char* buf, const char* tag, string& dest) {
    string str;
    char const* p = strstr(buf, tag);
    if (!p) return false;
    p = strchr(p, '>');
    ++p;
    char const* q = strchr(p, '<');
    if (!q) return false;
    str.assign(p, q-p);
    strip_whitespace(str);
    xml_unescape(str, dest);
    return true;
}

bool parse_str(const char* buf, const char* tag, char* dest, int len) {
    string str;
    if (!parse_str(buf, tag, str)) return false;
    safe_strncpy(dest, str.c_str(), len);
    return true;
}

// parse a string of the form name="string";
// returns string in dest
//
void parse_attr(const char* buf, const char* name, char* dest, int len) {
    char* p, *q;

    strcpy(dest, "");
    p = strstr(buf, name);
    if (!p) return;
    p = strchr(p, '"');
    if (!p) return;
    q = strchr(p+1, '"');
    if (!q) return;
    if (len > q-p) len = q-p;
    safe_strncpy(dest, p+1, len);
}

void copy_stream(FILE* in, FILE* out) {
    char buf[1024];
    int n, m;
    while (1) {
        n = (int)fread(buf, 1, 1024, in);
        m = (int)fwrite(buf, 1, n, out);
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
int dup_element_contents(FILE* in, const char* end_tag, char** pp) {
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
    return ERR_XML_PARSE;
}

// copy from a file to static buffer
//
int copy_element_contents(FILE* in, const char* end_tag, char* p, int len) {
    char buf[256];

    strcpy(p, "");
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        strcat(p, buf);
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}

int copy_element_contents(FILE* in, const char* end_tag, string& str) {
    char buf[256];

    str = "";
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        str += buf;
    }
    fprintf(stderr, "copy_element_contents(): no end tag\n");
    return ERR_XML_PARSE;
}


// read a file into a malloc'd string
//
int read_file_malloc(const char* pathname, char*& str) {
    char buf[256];
    FILE* f;

    f = fopen(pathname, "r");
    if (!f) return ERR_FOPEN;
    str = strdup("");
    while (fgets(buf, 256, f)) {
        strcatdup(str, buf);
    }
    fclose(f);
    return 0;
}


// replace XML element contents.  not currently used
//
void replace_element(char* buf, char* start, char* end, char* replacement) {
    char temp[4096], *p, *q;

    p = strstr(buf, start);
    p += strlen(start);
    q = strstr(p, end);
    safe_strncpy(temp, q, sizeof(temp));
    strcpy(p, replacement);
    strcat(p, temp);
}

// if the given XML has an element of the form
// <venue name="venue_name">
//   ...
// </venue>
// then return the contents of that element.
// Otherwise strip out all <venue> elements
//
void extract_venue(char* in, char* venue_name, char* out) {
    char* p, *q;
    char buf[256];
    sprintf(buf, "<venue name=\"%s\">", venue_name);
    p = strstr(in, buf);
    if (p) {
        p += strlen(buf);
        strcpy(out, p);
        q = strstr(out, "</venue");
        if (q) *q = 0;
    } else {
        strcpy(out, in);
        while (1) {
            p = strstr(out, "<venue");
            if (!p) break;
            q = strstr(p, "</venue>\n");
            if (!q) break;
            strcpy(p, q+strlen("</venue>\n"));
        }
    }
}

// copy a line from the given string.
// kinda like fgets() when you're reading from a string
//
char* sgets(char* buf, int len, char*& in) {
    char* p;

    p = strstr(in, "\n");
    if (!p) return NULL;
    *p = 0;
    safe_strncpy(buf, in, len);
    *p = '\n';
    in = p+1;
    return buf;
}

void xml_escape(string& in, string& out) {
	int i;
	out = "";
	for (i=0; i<(int)in.length(); i++) {
		if (in[i] == '<') {
			out += "&lt;";
		} else if (in[i] == '&') {
			out += "&amp;";
		} else {
			out += in[i];
		}
	}
}

void xml_unescape(string& in, string& out) {
	int i;
	 out = "";
	 for (i=0; i<(int)in.length(); i++) {
		 if (in.substr(i, 4) == "&lt;") {
			 out += "<";
			 i += 3;
		 } else if (in.substr(i, 5) == "&amp;") {
			 out += "&";
			 i += 4;
		 } else {
			 out += in[i];
		 }
	 }
}

