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

#ifndef _GRAPHICS2_H_
#define _GRAPHICS2_H_

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
extern void* boinc_graphics_make_shmem(const char*, int);
extern void* boinc_graphics_get_shmem(const char*);
extern void boinc_set_windows_icon(const char* icon16,const char* icon48);
extern void boinc_close_window_and_quit(const char*);

// Implementation stuff
//
extern double boinc_max_fps;
extern double boinc_max_gfx_cpu_frac;
extern void get_window_title(char* buf, int len);
extern bool throttled_app_render(int, int, double);

#endif
