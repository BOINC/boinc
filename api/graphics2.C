#include "util.h"
#include "app_ipc.h"
#include "shmem.h"
#include "boinc_api.h"
#include "graphics2.h"

double boinc_max_fps = 30.;
double boinc_max_gfx_cpu_frac;

bool throttled_app_render(int x, int y, double t) {
    static double total_render_time = 0;
    static double time_until_render = 0;
    static double last_now = 0;
    static double elapsed_time = 0;
    double now, t0, t1, diff, frac;
    bool ok_to_render = true;

    now = dtime();
    diff = now - last_now;
    last_now = now;

    // ignore interval if negative or more than 1 second
    //
    if ((diff<0) || (diff>1.0)) {
        diff = 0;
    }

    // enforce frames/sec restriction
    //
    if (boinc_max_fps) {
        time_until_render -= diff;
        if (time_until_render < 0) {
            time_until_render += 1./boinc_max_fps;
        } else {
            ok_to_render = false;
        }
    }

    // enforce max CPU time restriction
    //
    if (boinc_max_gfx_cpu_frac) {
        elapsed_time += diff;
        if (elapsed_time) {
            frac = total_render_time/elapsed_time;
            if (frac > boinc_max_gfx_cpu_frac) {
                ok_to_render = false;
            }
        }
    }

    // render if allowed
    //
    if (ok_to_render) {
        if (boinc_max_gfx_cpu_frac) {
            boinc_calling_thread_cpu_time(t0);
        }
        app_graphics_render(x, y, t);
        if (boinc_max_gfx_cpu_frac) {
            boinc_calling_thread_cpu_time(t1);
            total_render_time += t1 - t0;
        }
        return true;
    }
    return false;
}

void get_window_title(char* buf, int len) {
    APP_INIT_DATA aid;
    boinc_get_init_data(aid);
    if (aid.app_version) {
        snprintf(buf, len,
            "%s version %.2f [workunit: %s]",
            aid.app_name, aid.app_version/100.0, aid.wu_name
        );
    } else {
        snprintf(buf, len,
            "%s [workunit: %s]",
            aid.app_name, aid.wu_name
        );
    }
}
