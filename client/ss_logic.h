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

// BOINC core client screensaver logic.
// The basic idea:
// when the core client goes into screensaver mode,
// it immediately creates a "curtain" fullscreen window
// (this ensures that no user input gets lost)
// It tells all apps to hide their graphics.
// It asks a graphic-capable app to do fullscreen graphics.
// If there is none, or if the app doesn't ack in 5 seconds,
// the core client draws the BOINC logo graphics in the curtain window.
// (There's no attempt to request graphics from another app.)
// When it's time to blank screen, it tells the graphics app,
// if any, to hide itself, and draws black in the curtain window.
// Leave screensaver mode if either:
// - the core client gets user input in the curtain window, or
// - the core client gets an END_SS message from the graphics app.
// In either case, it close the curtain window and restores all apps
// to their pre-screensaver state

#ifndef _SS_LOGIC_
#define _SS_LOGIC_

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <time.h>
#endif

class SS_LOGIC {
    time_t blank_time;          // 0 or time to blank screen
    time_t ack_deadline;        // when to give up on graphics app
public:
    SS_LOGIC();

    char ss_msg[256];           // message to display on BOINC screensaver

    void start_ss(time_t blank_time);
    void stop_ss();
    void poll();
    bool do_ss;                 // true if we're acting like a screensaver
    bool do_boinc_logo_ss;      // true if we're bouncing the logo
    bool do_blank;              // true if we're drawing black

// invariants
//
// do_boinc_logo_ss and do_blank are not both true
// at most one app has request_mode = FULLSCREEN
// pre-ss mode of all apps is not FULLSCREEN
};

#endif
