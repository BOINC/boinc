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

#ifndef PARSE_H
#define PARSE_H

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <cstdio>
#include <cstdlib>
#include <string>
#include <math.h>
#endif

// return true if the tag appears in the line
//
inline bool match_tag(const char* buf, const char* tag) {
    if (strstr(buf, tag)) return true;
    return false;
}

inline bool match_tag(const std::string &s, const char* tag) {
    return match_tag(s.c_str(), tag);
}

// parse an integer of the form <tag>1234</tag>
// return true if it's there
// Note: this doesn't check for the end tag
//
inline bool parse_int(const char* buf, const char* tag, int& x) {
    const char* p = strstr(buf, tag);
    if (!p) return false;
    x = strtol(p+strlen(tag), 0, 0);        // this parses 0xabcd correctly
    return true;
}

// Same, for doubles
//
inline bool parse_double(const char* buf, const char* tag, double& x) {
    double y;
    const char* p = strstr(buf, tag);
    if (!p) return false;
    y = atof(p+strlen(tag));
    if (finite(y)) {
        x = y;
        return true;
    }
    return false;
}

extern bool parse(char* , char* );
extern bool parse_str(const char*, const char*, char*, int);
extern bool parse_str(const char* buf, const char* tag, std::string& dest);
extern void parse_attr(const char* buf, const char* attrname, char* out, int len);
extern bool parse_bool(const char*, const char*, bool&);
extern void copy_stream(FILE* in, FILE* out);
extern int strcatdup(char*& p, char* buf);
extern int dup_element_contents(FILE* in, const char* end_tag, char** pp);
extern int copy_element_contents(FILE* in, const char* end_tag, char* p, int len);
extern int copy_element_contents(FILE* in, const char* end_tag, std::string&);
extern void file_to_str(FILE* in, std::string& str);
extern int read_file_malloc(const char* pathname, char*& str);
extern void replace_element_contents(
    char* buf, const char* start, const char* end, const char* replacement
);
extern bool remove_element(char* buf, const char* start, const char* end);
extern bool str_replace(char* str, const char* old, const char* neww);
extern char* sgets(char* buf, int len, char* &in);
extern void xml_escape(const char*, char*);
extern void xml_unescape(const char*, char*);
extern void extract_venue(char*, char*, char*);
extern int skip_unrecognized(char* buf, FILE*);

#endif
