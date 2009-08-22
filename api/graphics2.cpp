// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// platform-independent part of graphics library
//

#ifdef _MSC_VER
#define strdup _strdup
#define snprintf _snprintf
#endif

#include "util.h"
#include "app_ipc.h"
#include "shmem.h"
#include "boinc_api.h"
#include "graphics2.h"

double boinc_max_fps = 30.;
double boinc_max_gfx_cpu_frac = 0.2;
    // needs to be fairly low.  Graphics apps run at normal priority,
    // so they can prevent main app from getting any time

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
