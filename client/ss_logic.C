#include "client_state.h"

#include "ss_logic.h"

extern void create_curtain();
extern void delete_curtain();

SS_LOGIC::SS_LOGIC() {
    do_ss = false;
    do_boinc_logo_ss = false;
    do_blank = false;
    blank_time = 0;
    ack_deadline = 0;
}

// this is called when the core client receives a message
// from the screensaver module.
//
void SS_LOGIC::start_ss(time_t new_blank_time) {
    ACTIVE_TASK* atp;

    if (do_ss) return;
    do_ss = true;
    do_blank = do_boinc_logo_ss = false;
    sprintf(ss_msg, "");
    blank_time = new_blank_time;
    gstate.active_tasks.save_app_modes();
    gstate.active_tasks.hide_apps();
    create_curtain();
    atp = gstate.active_tasks.get_graphics_capable_app();
    if (atp) {
        atp->request_graphics_mode(MODE_FULLSCREEN);
        ack_deadline = time(0) + 5;
    } else {
        do_boinc_logo_ss = true;
    }
}

void SS_LOGIC::stop_ss() {
    if (!do_ss) return;
    do_ss = do_boinc_logo_ss = do_blank = false;
    delete_curtain();
    gstate.active_tasks.restore_apps();
}


// called every second
//
void SS_LOGIC::poll() {
    ACTIVE_TASK* atp;

    gstate.active_tasks.check_graphics_mode_ack();

    if (do_ss) {
        if (blank_time && (time(0) > blank_time)) {
            if (!do_blank) {
                atp = gstate.active_tasks.get_app_requested(MODE_FULLSCREEN);
                if (atp) {
                    atp->request_graphics_mode(MODE_HIDE_GRAPHICS);
                }
            }
            do_boinc_logo_ss = false;
            sprintf(ss_msg, "");
            do_blank = true;
        } else {
            atp = gstate.active_tasks.get_app_requested(MODE_FULLSCREEN);
            if (atp) {
                if (atp->graphics_acked_mode == MODE_FULLSCREEN) {
                    do_boinc_logo_ss = false;
                    sprintf(ss_msg, "");
                } else {
                    if (time(0)>ack_deadline) {
                        do_boinc_logo_ss = true;
                        sprintf(ss_msg, "App can't display graphics");
                    }
                }
            } else {
                atp = gstate.active_tasks.get_graphics_capable_app();
                if (atp) {
                    atp->request_graphics_mode(MODE_FULLSCREEN);
                    ack_deadline = time(0) + 5;
                } else {
                    do_boinc_logo_ss = true;
                    sprintf(ss_msg, "No work available");
                }
            }
        }
    }
}
