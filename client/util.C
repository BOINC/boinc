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

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <time.h>
#include <windows.h>

/* Replacement gettimeofday 
   Sets the microseconds to clock() * 1000 which is microseconds in Windows */
void gettimeofday(timeval *t, void *tz) {
	t->tv_sec = time(NULL);
	t->tv_usec = 1000 * (long)(clock());
}

#endif

#include "util.h"

// return time of day as a double
//
double dtime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
}

#ifdef _WIN32
void boinc_sleep( int seconds ) {
	::Sleep( 1000*seconds );
}

#else

void boinc_sleep( int seconds ) {
	sleep( seconds );
}

#endif
