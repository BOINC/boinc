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


bool is_same_task(RESULT* taska, RESULT* taskb) {
    if ((taska == NULL) || (taskb == NULL)) return false;
    if (taska->name != taskb->name) return false;
    if (taska->project_url != taskb->project_url) return false;
    return true;
}

int count_active_graphic_apps(RESULTS& results, RESULT* exclude) {
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;

    // Count the number of active graphics-capable apps excluding the specified result.
    // If exclude is NULL, don't exclude any results.
    for (i = 0; i < results.results.size(); i++) {
        BOINCTRACE(_T("get_random_graphics_app -- active task detected\n"));
        BOINCTRACE(
            _T("get_random_graphics_app -- name = '%s', path = '%s'\n"),
            results.results[i]->name.c_str(), results.results[i]->graphics_exec_path.c_str()
        );
        if (results.results[i]->graphics_exec_path.size() == 0) continue;
        BOINCTRACE(_T("get_random_graphics_app -- active task detected w/graphics\n"));
        
        if (is_same_task(results.results[i], exclude)) continue;
        graphics_app_count++;
    }
    return graphics_app_count;
}


// Choose a random graphics application out of the vector.
// Exclude the specified result unless it is the only candidate.
// If exclude is NULL or an empty string, don't exclude any results.
//
RESULT* get_random_graphics_app(RESULTS& results, RESULT* exclude) {
    RESULT*      rp = NULL;
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;
    unsigned int random_selection = 0;
    unsigned int current_counter = 0;
    RESULT *avoid = exclude;

    BOINCTRACE(_T("get_random_graphics_app -- Function Start\n"));

    graphics_app_count = count_active_graphic_apps(results, avoid);
    BOINCTRACE(_T("get_random_graphics_app -- graphics_app_count = '%d'\n"), graphics_app_count);

    // If no graphics app found other than the one excluded, count again without excluding any
    if ((0 == graphics_app_count) && (avoid != NULL)) {
        avoid = NULL;
        graphics_app_count = count_active_graphic_apps(results, avoid);
    }
        
    // If no graphics app was found, return NULL
    if (0 == graphics_app_count) {
        goto CLEANUP;
    }

    // Choose which application to display.
    random_selection = (rand() % graphics_app_count) + 1;
    BOINCTRACE(_T("get_random_graphics_app -- random_selection = '%d'\n"), random_selection);

    // Lets find the chosen graphics application.
    for (i = 0; i < results.results.size(); i++) {
        if (results.results[i]->graphics_exec_path.size() == 0) continue;
        if (is_same_task(results.results[i], avoid)) continue;

        current_counter++;
        if (current_counter == random_selection) {
            rp = results.results[i];
            break;
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

