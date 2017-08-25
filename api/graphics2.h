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

#ifndef BOINC_GRAPHICS2_H
#define BOINC_GRAPHICS2_H

struct BOINC_STATUS;

// Functions that must be supplied by the app
//
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_init(void);
extern void app_graphics_resize(int width, int height);
extern void boinc_app_mouse_button(int x, int y, int which, int is_down);
extern void boinc_app_mouse_move(int x, int y, int left, int middle, int right);
extern void boinc_app_key_press(int, int);
extern void boinc_app_key_release(int, int);

// Functions that the app can call
//
extern void boinc_graphics_loop(int argc, char** argv, const char* title=0);
extern void boinc_set_windows_icon(const char* icon16,const char* icon48);
extern void boinc_close_window_and_quit(const char*);

// functions for communication between main app and graphics app
//
extern void* boinc_graphics_make_shmem(const char*, int);
extern void* boinc_graphics_get_shmem(const char*);
extern int boinc_write_graphics_status(
    double cpu_time, double elapsed_time,
    double fraction_done
);
extern int boinc_parse_graphics_status(
    double* update_time, double* cpu_time,
    double* elapsed_time, double* fraction_done, BOINC_STATUS* status
);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern void get_window_title(char* buf, int len);
extern bool throttled_app_render(int, int, double);

#endif
