<?php
require_once("docutil.php");
page_head("Screensaver Logic");
echo "

<p>
The screensaver supports:
<ol>
<li>
Fullscreen graphics from project applications
<li>
A default BOINC logo screensaver when no running applications support
graphics
<li>
Screen blanking after a given time
</ol>

<p>
If the screensaver is running, the core client will try to devote screen
time to a different project each time the CPUs are rescheduled.


<pre>
request_ss_app():
    choose some app that can do graphics
    if app exists:
        request app (from a different project if possible)
            to do fullscreen graphics
        ACK deadline = now + 5 sec
    else:
        do_boinc_logo_ss = true

// called to force poll function to request another app to do graphics
//
// this is called by schedule_cpus() whenever CPUs are rescheduled
//
reset():
    tell any fullscreen app to hide
    do_boinc_logo_ss = false

// called when core client receives a message from the screensaver module
//
start_ss(new_blank_time):
    save all apps' modes
    hide all apps
    blank_time = new_blank_time
    if activities not suspended:
        request_ss_app()

// called when screensaver module quits
//
stop_ss():
    clean up stuff
    restore apps' modes

// called once a second
//
poll():
    if activities not suspended:
        if doing screensaver:
            if blank_time != 0 && now > blank_time:
                if not screen not already blanked:
                    tell any fullscreen app to hide
                    blank screen
                do_boinc_logo_ss = false
            else:
                get the fullscreen app
                if app exists:
                    if app has sent ACK:
                        do_boinc_logo_ss = false
                    else if ACK deadline has passed:
                        tell app to hide
                        do_boinc_logo_ss = true
                else:
                    request_ss_app()
    else:
        do_boinc_logo_ss = true


/////////////////////////
logic of screensaver program:
screensaver starts
    creates blank windows on every screen
    show \"screensaver loading\" in all windows

    if core client not running and not set to run on startup
        show \"boinc not running, auto startup not detected
            please reinstall and select this option\"
    10 times, every 1 sec, then once every 30 sec
        set_screensaver_mode RPC to core client
            pass desktop/windowstation, blank time
        get_screensaver_mode
            returns:
                - some app is doing fullscreen
                - no graphics-capable app
                - BOINC suspended
                - time to blank

    after each get_screensaver_mode (beyond initial 10 sec):
        if no app is doing graphics,
        do get_results RPC
        show string on all windows

</pre>

";
page_tail();
?>
