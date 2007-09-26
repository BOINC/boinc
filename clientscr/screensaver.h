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
// 

#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__

// Determine if two RESULT pointers refer to the same task
extern bool is_same_task(RESULT* taska, RESULT* taskb);

// Count the number of active graphics-capable apps
extern int count_active_graphic_apps(RESULTS& results, RESULT* exclude = NULL);

// Choose a ramdom graphics application from the vector that
//   was passed in.
extern RESULT* get_random_graphics_app(RESULTS& results, RESULT* exclude = NULL);

// Launch the screensaver
#ifdef _WIN32
extern int launch_screensaver(RESULT* rp, HANDLE& graphics_application, RPC_CLIENT* rpc);
#else
extern int launch_screensaver(RESULT* rp, int& graphics_application, RPC_CLIENT* rpc);
#endif

// Terminate the screensaver
#ifdef _WIN32
extern int terminate_screensaver(HANDLE& graphics_application, RESULT *worker_app, RPC_CLIENT* rpc);
#else
extern int terminate_screensaver(int& graphics_application, RESULT *worker_app, RPC_CLIENT* rpc);
#endif

#endif