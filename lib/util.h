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

#include <time.h>

extern int double_to_ydhms (double x, int smallest_timescale, char *buf);
extern double dtime();
extern void boinc_sleep( int seconds );
extern int parse_command_line( char *, char ** );
extern int lock_file(char*);
extern double drand();
extern void unescape_url(char *url);
extern void escape_url(char *in, char*out);
extern void safe_strncpy(char*, char*, int);

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define SECONDS_PER_DAY 86400
