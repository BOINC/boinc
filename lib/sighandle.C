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
// Thomas Horsten <thomas@horsten.com>
//


#ifdef _WIN32
#include "stdafx.h"
#else
#include "config.h"
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "sighandle.h"

#ifdef HAVE_SIGNAL_H
// Set a signal handler but only if it is not currently ignored
void boinc_set_signal_handler(int sig, RETSIGTYPE (*handler)(int))
{
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
    if (temp.sa_handler != SIG_IGN) {
	 temp.sa_handler = handler;
	 sigemptyset(&temp.sa_mask);
	 sigaction(sig, &temp, NULL);
    }
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
    if (temp == SIG_IGN) {
	 signal(sig, SIG_IGN);
    }
#endif /* HAVE_SIGACTION */
}

// Set a signal handler but even if it is currently ignored
void boinc_set_signal_handler_force(int sig, RETSIGTYPE (*handler)(int))
{
#ifdef HAVE_SIGACTION
    struct sigaction temp;
    sigaction(sig, NULL, &temp);
	 temp.sa_handler = handler;
	 sigemptyset(&temp.sa_mask);
	 sigaction(sig, &temp, NULL);
#else
    void (*temp)(int);
    temp = signal(sig, boinc_catch_signal);
	 signal(sig, SIG_IGN);
#endif /* HAVE_SIGACTION */
}

#endif /* HAVE_SIGNAL_H */
