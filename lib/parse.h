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

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <string>
using std::string;
#endif


extern bool parse(char* , char* );
extern bool parse_int(const char* buf, const char*tag, int&);
extern bool parse_double(const char*, const char*, double&);
extern bool parse_str(const char*, const char*, char*, int);
extern bool parse_str(const char* buf, const char* tag, string& dest);
extern void parse_attr(const char* buf, const char* attrname, char* out, int len);
extern bool match_tag(const char*, const char*);
extern bool match_tag(const std::string &, const char*);
extern void copy_stream(FILE* in, FILE* out);
extern void strcatdup(char*& p, char* buf);
extern int dup_element_contents(FILE* in, const char* end_tag, char** pp);
extern int copy_element_contents(FILE* in, const char* end_tag, char* p, int len);
extern int copy_element_contents(FILE* in, const char* end_tag, string&);
extern int read_file_malloc(const char* pathname, char*& str);
extern void replace_element(char* buf, char* start, char* end, char* replacement);
extern char* sgets(char* buf, int len, char* &in);
extern void xml_escape(string&, string&);
extern void xml_unescape(string&, string&);
