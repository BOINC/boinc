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

#ifndef _SS_LOGIC_
#define _SS_LOGIC_

#ifndef _WIN32
#include <ctime>
#endif

#define SS_STATUS_ENABLED                           1
#define SS_STATUS_RESTARTREQUEST                    2
#define SS_STATUS_BLANKED                           3
#define SS_STATUS_BOINCSUSPENDED                    4
#define SS_STATUS_NOTGRAPHICSCAPABLE                5
#define SS_STATUS_NOAPPSEXECUTING                   6
#define SS_STATUS_NOGRAPHICSAPPSEXECUTING           7
#define SS_STATUS_QUIT                              8
#define SS_STATUS_NOAPPSEXECUTINGNOPROJECTSDETECTED 9

class SS_LOGIC {
public:

    SS_LOGIC();

    void start_ss(GRAPHICS_MSG&, double blank_time);
    void stop_ss();
    void poll();
    void reset();
    int  get_ss_status() { return ss_status; };

private:
    double blank_time;          // 0 or time to blank screen
    double ack_deadline;        // when to give up on graphics app
    int  ss_status;             // the status of the screensaver from the core
                                // client perspective.
    bool do_ss;                 // true if we're acting like a screensaver

// invariants
//
// do_boinc_logo_ss and do_blank are not both true
// at most one app has request_mode = FULLSCREEN
// pre-ss mode of all apps is not FULLSCREEN
};

#endif
