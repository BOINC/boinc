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

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <string>
#endif

extern bool parse(char* , char* );
extern bool parse_int(const char* buf, const char*tag, int&);
extern bool parse_double(const char*, const char*, double&);
extern bool parse_str(const char*, const char*, char*, int);
extern bool parse_str(const char* buf, const char* tag, std::string& dest);
extern void parse_attr(const char* buf, const char* attrname, char* out, int len);
extern bool match_tag(const char*, const char*);
extern bool match_tag(const std::string &, const char*);
extern void copy_stream(FILE* in, FILE* out);
extern void strcatdup(char*& p, char* buf);
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
extern void xml_escape(std::string&, std::string&);
extern void xml_escape(char*, std::string&);
extern void xml_unescape(std::string&, std::string&);
extern void extract_venue(char*, char*, char*);
extern int skip_unrecognized(char* buf, FILE*);

#endif
