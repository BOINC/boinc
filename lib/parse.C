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
#include "config.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include <math.h>
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



// Parse a boolean; tag is of form "foobar"
// Accept either <foobar/> or <foobar>1</foobar>
//
bool parse_bool(const char* buf, const char* tag, bool& result) {
    char single_tag[256], start_tag[256];
    int x;

    sprintf(single_tag, "<%s/>", tag);
    if (match_tag(buf, single_tag)) {
        result = true;
        return true;
    }
    sprintf(start_tag, "<%s>", tag);
    if (parse_int(buf, start_tag, x)) {
        result = (x != 0);
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
bool parse_str(const char* buf, const char* tag, char* dest, int destlen) {
    string str;
    const char* p;
    char tempbuf[1024];
    int len;

    // sanity check on NULL and empty cases. 
    if (!buf || !tag || !strlen(tag))
    return false;

    p = strstr(buf, tag);
    if (!p) return false;
    p = strchr(p, '>');
    p++;
    const char* q = strchr(p, '<');
    if (!q) return false;
    len = (int)(q-p);
    if (len >= destlen) len = destlen-1;
    memcpy(tempbuf, p, len);
    tempbuf[len] = 0;
    strip_whitespace(tempbuf);
    xml_unescape(tempbuf, dest);
    return true;
}

bool parse_str(const char* buf, const char* tag, string& dest) {
    char tempbuf[1024];
    if (!parse_str(buf, tag, tempbuf, 1024)) return false;
    dest = tempbuf;
    return true;
}

// parse a string of the form name="string";
// returns string in dest
//
void parse_attr(const char* buf, const char* name, char* dest, int len) {
    const char* p;
    const char *q;

    strcpy(dest, "");
    p = strstr(buf, name);
    if (!p) return;
    p = strchr(p, '"');
    if (!p) return;
    q = strchr(p+1, '"');
    if (!q) return;
    if (len > q-p) len = (int)(q-p);
    strlcpy(dest, p+1, len);
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
int strcatdup(char*& p, char* buf) {
    p = (char*)realloc(p, strlen(p) + strlen(buf)+1);
    if (!p) {
        return ERR_MALLOC;
    }
    strcat(p, buf);
    return 0;
}

// copy from a file to a malloc'd string until the end tag is reached
//
int dup_element_contents(FILE* in, const char* end_tag, char** pp) {
    char buf[256];
    int retval;

    char* p = strdup("");
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            *pp = p;
            return 0;
        }
        retval = strcatdup(p, buf);
        if (retval) return retval;
    }
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
    int retval;

    f = fopen(pathname, "r");
    if (!f) return ERR_FOPEN;
    str = strdup("");
    while (fgets(buf, 256, f)) {
        retval = strcatdup(str, buf);
        if (retval) return retval;
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
    strlcpy(temp, q, sizeof(temp));
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
    strlcpy(buf, in, len);
    *p = '\n';
    in = p+1;
    return buf;
}

// NOTE: these used to take std::string instead of char* args.
// But this performed poorly.
//
// NOTE: output buffer should be 6X size of input
//
void xml_escape(const char* in, char* out) {
    char buf[256], *p;

    p = out;

    for (; *in; in++) {
        int x = (int) *in;
        x &= 0xff;   // just in case
        if (x == '<') {
            strcpy(p, "&lt;");
            p += 4;
        } else if (x == '&') {
            strcpy(p, "&amp;");
            p += 5;
        } else if (x>127) {
            sprintf(buf, "&#%d;", x);
            strcpy(p, buf);
            p += strlen(buf);
        } else if (x<32) {
            switch(x) {
            case 9:
            case 10:
            case 13:
                sprintf(buf, "&#%d;", x);
                strcpy(p, buf);
                p += strlen(buf);
                break;
            }
        } else {
            *p++ = x;
        }
    }
    *p = 0;
}

// output buffer need not be larger than input
//
void xml_unescape(const char* in, char* out) {
    char* p = out;
    while (*in) {
        if (!strncmp(in, "&lt;", 4)) {
            *p++ = '<';
            in += 4;
        } else if (!strncmp(in, "&amp;", 5)) {
            *p++ = '&';
            in += 5;
        } else if (!strncmp(in, "&#", 2)) {
            in += 2;
            char c = atoi(in);
            *p++ = c;
            in = strchr(in, ';');
            if (in) in++;
        } else {
            *p++ = *in++;
        }
    }
    *p = 0;
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

// Get next XML element or tag.
//
// NOTE: this can't be used for XML docs that have
// nested elements and data elements at the same level, i.e.
// <foo>
//   <bar>1</bar>
// </foo>
// <blah>asdf</blah>
//
// If it's a close tag, or the contents pointer is NULL, just return the tag.
// Otherwise return the contents also.
//
// Skips comments, single- or multi-line (<!-- ... -->)
//
// Returns false if text was found that wasn't a tag.
// (can just call again in this case).
//
// Note: this is not a general XML parser, but it can parse both
// <foo>X</foo>
// and
// <foo>
//    X
// </foo>
//
bool get_tag(FILE* f, char* tag, char* contents) {
    char buf[1024], *p, *q;

    if (contents) *contents = 0;
    while (1) {
        if (!fgets(buf, 1024, f)) return false;
        if (strstr(buf, "<!--")) {
            while (1) {
                if (strstr(buf, "-->")) break;
                if (!fgets(buf, 1024, f)) return false;
            }
            continue;
        }
        p = strchr(buf, '<');
        if (p) break;
    }
    p++;

    q = strchr(p, '>');
    if (!q) return false;

    // see if it's a self-closed tag (like <foo/>)
    //
    if (q[-1]=='/') {
        q[-1] = 0;
        strcpy(tag, p);
        return true;
    }
    *q = 0;
    strcpy(tag, p);

    // see if this is a close tag
    //
    if (*p == '/') {
        return true;
    }
    if (!contents) return true;

    // see if close tag is on the same line; copy contents if so
    //
    q++;
    p = strchr(q, '<');
    if (p) {
        *p = 0;
        if (contents) {
            strcpy(contents, q);
            strip_whitespace(contents);
        }
        return true;
    }

    // close tag is not on same line.
    // Copy contents until find close tag
    //
    while (1) {
        if (!fgets(buf, 1024, f)) return false;
        if (strchr(buf, '<')) return true;
        strip_whitespace(buf);
        strcat(contents, buf);
    }
}

bool get_bool(char* contents) {
    if (!strlen(contents)) return true;
    if (atoi(contents)) return true;
    return false;
}

int get_int(char* contents) {
    return strtol(contents, 0, 0);
}

double get_double(char* contents) {
    double x = atof(contents);
    if (finite(x)) return x;
    return 0;
}

const char *BOINC_RCSID_3f3de9eb18 = "$Id$";
