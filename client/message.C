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
#include <stdarg.h>
#endif

//#include "client_msgs.h"

vector<MESSAGE_DESC> message_descs;

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
// TODO: add translation functionality
//
void msg_printf(PROJECT *p, int priority, char *fmt, ...) {
    char        buf[512];
    va_list     ap;

    if (fmt == NULL) return;

    // Since Windows doesn't support vsnprintf, we have to do a
    // workaround to prevent buffer overruns
    //
    if (strlen(fmt) > 512) fmt[511] = '\0';
    va_start(ap, fmt); // Parses string for variables
    vsprintf(buf, fmt, ap); // And convert symbols To actual numbers
    va_end(ap); // Results are stored in text

    show_message(p, buf, priority);
}
