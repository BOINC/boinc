// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// A very crude interface for parsing XML files;
// assumes all elements are either single-line or
// have start and end tags on separate lines.
// This is meant to be used ONLY for parsing XML files produced
// by the BOINC scheduling server or client.
// Could replace this with a more general parser.

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstring>
#include <cstdlib>
#include <locale>
#include <string>
#if HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#endif

#include "error_numbers.h"
#include "util.h"
#include "parse.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::string;

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
    const char* p = strstr(buf, tag);
    if (!p) return false;
    std::string strLocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    x = strtol(p+strlen(tag), 0, 0);        // this parses 0xabcd correctly
    setlocale(LC_NUMERIC, strLocale.c_str());
    return true;
}

// Same, for doubles
//
bool parse_double(const char* buf, const char* tag, double& x) {
    double y;
    const char* p = strstr(buf, tag);
    if (!p) return false;
    std::string strLocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    y = atof(p+strlen(tag));
    setlocale(LC_NUMERIC, strLocale.c_str());
    if (finite(y)) {
        x = y;
        return true;
    }
    return false;
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
    const char* p;

    // sanity check on NULL and empty cases. 
    if (!buf || !tag || !strlen(tag))
    return false;

    p = strstr(buf, tag);
    if (!p) return false;
    p = strchr(p, '>');
    ++p;
    const char* q = strchr(p, '<');
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
    if (len > q-p) len = (int)(q-p);
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
    int n;

    strcpy(p, "");
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        n = (int)strlen(buf);
        if (n >= len-1) return ERR_XML_PARSE;
        strcat(p, buf);
        len -= n;
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

void file_to_str(FILE* in, string& str) {
    char buf[256];

    str = "";
    while (fgets(buf, 256, in)) {
        str += buf;
    }
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


// replace XML element contents (element must be present)
//
void replace_element_contents(
    char* buf, const char* start, const char* end, const char* replacement
) {
    char temp[4096], *p, *q;

    p = strstr(buf, start);
    p += strlen(start);
    q = strstr(p, end);
    safe_strncpy(temp, q, sizeof(temp));
    strcpy(p, replacement);
    strcat(p, temp);
}

// if the string contains a substring of the form X...Y,
// remove the first such.
bool remove_element(char* buf, const char* start, const char* end) {
    char* p, *q;
    p = strstr(buf, start);
    if (!p) return false;
    q = strstr(p+strlen(start), end);
    if (!q) return false;
    strcpy(p, q+strlen(end));
    return true;
}

// replace a substring.  Do at most one instance.
//
bool str_replace(char* str, const char* substr, const char* replacement) {
    char temp[4096], *p;

    p = strstr(str, substr);
    if (!p) return false;
    int n = (int)strlen(substr);
    strcpy(temp, p+n);
    strcpy(p, replacement);
    strcat(p, temp);
    return true;
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
    char buf[256];

    out = "";
    for (i=0; i<(int)in.length(); i++) {
        int x = (int) in[i];
        x &= 0xff;   // just in case
        if (in[i] == '<') {
            out += "&lt;";
        } else if (in[i] == '&') {
            out += "&amp;";
        } else if (x>127) {
            sprintf(buf, "&#%d;", x);
            out += buf;
        } else if (x<32) {
            switch(x) {
            case 9:
            case 10:
            case 13:
                sprintf(buf, "&#%d;", x);
                out += buf;
                break;
            }
        } else {
            out += in[i];
        }
    }
}

void xml_escape(char* in, string& out) {
    string foo = in;
    xml_escape(foo, out);
}

void xml_unescape(string& in, string& out) {
    size_t i;
    out = "";
    for (i=0; i<in.length();) {
        if (in.substr(i, 4) == "&lt;") {
            out += "<";
            i += 4;
        } else if (in.substr(i, 5) == "&amp;") {
            out += "&";
            i += 5;
        } else if (in.substr(i, 2) == "&#") {
            char c = atoi(in.substr(i+2, 3).c_str());
            out += c;
            i = in.find(";", i);
            if (i==std::string::npos) break;
            i++;
        } else {
            out += in[i];
            i++;
        }
    }
}

// we got an unrecognized line.
// If it has two <'s (e.g. <foo>xx</foo>) return 0.
// If it's of the form <foo> then scan for </foo> and return 0.
// Otherwise return ERR_XML_PARSE
//
int skip_unrecognized(char* buf, FILE* in) {
    char* p, *q, buf2[256];
    std::string close_tag;

    p = strchr(buf, '<');
    if (!p) {
        return ERR_XML_PARSE;
    }
    if (strchr(p+1, '<')) {
        return 0;
    }
    q = strchr(p+1, '>');
    if (!q) {
        return ERR_XML_PARSE;
    }
    *q = 0;
    close_tag = string("</") + string(p+1) + string(">");
    while (fgets(buf2, 256, in)) {
        if (strstr(buf2, close_tag.c_str())) {
            return 0;
        }
        
    }
    return ERR_XML_PARSE;
}

const char *BOINC_RCSID_3f3de9eb18 = "$Id$";
