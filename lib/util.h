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

extern double time_t_to_jd(time_t unix_time);
extern time_t jd_to_time_t(double jd);
extern int double_to_ydhms (double x, int smallest_timescale, char *buf);
extern double dtime();
extern void boinc_sleep( int seconds );

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef __MWERKS__
// Note: Although Macintosh time is based on 1/1/1904, the
//MetroWerks Standard Libraries time routines use a base of 1/1/1900.
//#define JD0 2416480.5  /* Time at 0:00 GMT 1904 Jan 1 */
#define JD0 2415020.5  /* Time at 0:00 GMT 1900 Jan 1 */
#else
#define JD0 2440587.5  /* Time at 0:00 GMT 1970 Jan 1 */
#endif

#define SECONDS_PER_DAY 86400
