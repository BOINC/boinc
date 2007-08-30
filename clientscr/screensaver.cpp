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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "diagnostics.h"
#include "common_defs.h"
#include "util.h"
#include "gui_rpc_client.h"
#include "screensaver.h"


// Determine if a task is active and executing
//
bool is_task_active(RESULT* result) {
    bool bIsActive     = RESULT_FILES_DOWNLOADED == result->state;
    bool bIsDownloaded = CPU_SCHED_SCHEDULED == result->scheduler_state;
    bool bIsExecuting  = result->active_task;

    if (bIsActive && bIsDownloaded && bIsExecuting)
        return true;
    return false;
}


// Choose a random graphics application out of the vector.
//
RESULT* get_random_graphics_app(RESULTS& results) {
    RESULT*      rp = NULL;
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;
    unsigned int random_selection = 0;
    unsigned int current_counter = 0;

    BOINCTRACE(_T("get_random_graphics_app -- Function Start\n"));

    // Count the number of graphics apps
    for (i = 0; i < results.results.size(); i++) {
        if (!is_task_active(results.results[i])) continue;
        BOINCTRACE(_T("get_random_graphics_app -- active task detected\n"));
        BOINCTRACE(
            _T("get_random_graphics_app -- name = '%s', path = '%s'\n"),
            results.results[i]->name.c_str(), results.results[i]->graphics_exec_path.c_str()
        );
        if (results.results[i]->graphics_exec_path.size() > 0) {
            BOINCTRACE(_T("get_random_graphics_app -- active task detected w/graphics\n"));
	        graphics_app_count++;
        }
    }
    BOINCTRACE(_T("get_random_graphics_app -- graphics_app_count = '%d'\n"), graphics_app_count);

    // If no graphics app was found, return NULL
    if (0 == graphics_app_count) {
        goto CLEANUP;
    }

    // Choose which application to display.
    random_selection = rand() % graphics_app_count;
    if (0 == random_selection) {
        random_selection = 1;
    }
    BOINCTRACE(_T("get_random_graphics_app -- random_selection = '%d'\n"), random_selection);

    // Lets find the choosen graphics application.
    for (i = 0; i < results.results.size(); i++) {
        if (!is_task_active(results.results[i])) continue;
        if (results.results[i]->graphics_exec_path.size() > 0) {
            current_counter++;
            if (current_counter == random_selection) {
	            rp = results.results[i];
            }
        }
    }

CLEANUP:
    BOINCTRACE(_T("get_random_graphics_app -- Function End\n"));

    return rp;
}


// Launch the graphics application
//
#ifdef _WIN32
int launch_screensaver(RESULT* rp, HANDLE& graphics_application)
#else
int launch_screensaver(RESULT* rp, int& graphics_application)
#endif
{
    int retval = 0;
    char* argv[3];
    argv[0] = "app_graphics";   // not used
    argv[1] = "--fullscreen";
    argv[2] = 0;
    retval = run_program(
        rp->slot_path.c_str(),
        rp->graphics_exec_path.c_str(),
        2,
        argv,
        0,
        graphics_application
    );
    return retval;
}


// Terminate the graphics application
//
#ifdef _WIN32
int terminate_screensaver(HANDLE& graphics_application)
#else
int terminate_screensaver(int& graphics_application)
#endif
{
    kill_program(graphics_application);
    return 0;
}

