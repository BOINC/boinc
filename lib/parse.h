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

extern bool parse(char*, char*);
extern bool parse_int(char*, char*, int&);
extern bool parse_double(char*, char*, double&);
extern bool parse_str(char*, char*, char*, int);
extern void parse_attr(char* buf, char* attrname, char* out, int len);
extern bool match_tag(char*, char*);
extern void copy_stream(FILE* in, FILE* out);
extern void strcatdup(char*& p, char* buf);
extern int dup_element_contents(FILE* in, char* end_tag, char** pp);
extern int copy_element_contents(FILE* in, char* end_tag, char* p, int len);
extern int read_file_malloc(char* pathname, char*& str);
extern void replace_element(char* buf, char* start, char* end, char* replacement);
extern void extract_venue(char* in, char* venue_name, char* out);
