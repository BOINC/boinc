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

#include <time.h>

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
