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
#include <stdlib.h>
#include <string>

extern bool parse(char* , char* );
extern bool parse_int(const char* buf, const char*tag, int&);
extern bool parse_double(const char*, const char*, double&);
extern bool parse_str(const char*, const char*, char*, int);
extern void parse_attr(const char* buf, const char* attrname, char* out, int len);
extern bool match_tag(const char*, const char*);
extern bool match_tag(const std::string &, const char*);
extern void copy_stream(FILE* in, FILE* out);
extern void strcatdup(char*& p, char* buf);
extern int dup_element_contents(FILE* in, const char* end_tag, char** pp);
extern int copy_element_contents(FILE* in, const char* end_tag, char* p, int len);
extern int read_file_malloc(const char* pathname, char*& str);
extern void replace_element(char* buf, char* start, char* end, char* replacement);
extern void extract_venue(const char* in, const char* venue_name, char* out);
extern char* sgets(char* buf, int len, char* &in);
extern bool extract_xml_record(const std::string &field, const char *tag, std::string &record);

