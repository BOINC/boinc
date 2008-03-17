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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// 

#ifdef _WIN32
#include "boinc_win.h"

#include <windowsx.h>
#include <mmsystem.h>
#include <regstr.h>
#include <strsafe.h>

#include "boinc_ss.h"

#endif

#include "diagnostics.h"
#include "common_defs.h"
#include "util.h"

#ifdef _WIN32
#include "gui_rpc_client.h"
#include "screensaver_win.h"
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <sys/wait.h>
#include "gui_rpc_client.h"
#include "Mac_Saver_Module.h"
#endif
#include "screensaver.h"

// Flags for testing & debugging
#define SIMULATE_NO_GRAPHICS 0


bool CScreensaver::is_same_task(RESULT* taska, RESULT* taskb) {
    if ((taska == NULL) || (taskb == NULL)) return false;
    if (taska->name != taskb->name) return false;
    if (taska->project_url != taskb->project_url) return false;
    return true;
}

int CScreensaver::count_active_graphic_apps(RESULTS& results, RESULT* exclude) {
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;
    m_bV5_GFX_app_is_running = false;

    // Count the number of active graphics-capable apps excluding the specified result.
    // If exclude is NULL, don't exclude any results.
    for (i = 0; i < results.results.size(); i++) {
        BOINCTRACE(_T("get_random_graphics_app -- active task detected\n"));
        BOINCTRACE(
            _T("get_random_graphics_app -- name = '%s', path = '%s'\n"),
            results.results[i]->name.c_str(), results.results[i]->graphics_exec_path.c_str()
        );
        if (results.results[i]->supports_graphics) m_bV5_GFX_app_is_running = true;
        if ((results.results[i]->graphics_exec_path.size() == 0) 
                && (state.executing_as_daemon || !(results.results[i]->supports_graphics))) continue;
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
RESULT* CScreensaver::get_random_graphics_app(RESULTS& results, RESULT* exclude) {
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
        if ((results.results[i]->graphics_exec_path.size() == 0) 
                && (state.executing_as_daemon || !(results.results[i]->supports_graphics))) continue;
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
int CScreensaver::launch_screensaver(RESULT* rp, HANDLE& graphics_application)
#else
int CScreensaver::launch_screensaver(RESULT* rp, int& graphics_application)
#endif
{
    int retval = 0;
    if (!rp->graphics_exec_path.empty()) {
        // V6 Graphics
#ifdef __APPLE__
        // For sandbox security, use gfx_switcher to launch gfx app 
        // as user boinc_project and group boinc_project.
        //
        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        char* argv[5];
        argv[0] = "gfx_Switcher";
        argv[1] = "-launch_gfx";
        argv[2] = strrchr(rp->slot_path.c_str(), '/');
        if (*argv[2]) argv[2]++;    // Point to the slot number in ascii
        
        argv[3] = "--fullscreen";
        argv[4] = 0;

       retval = run_program(
            rp->slot_path.c_str(),
            m_gfx_Switcher_Path,
            4,
            argv,
            0,
            graphics_application
        );
#else
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
#endif
    } else {
        // V5 and Older
        DISPLAY_INFO di;
#ifdef _WIN32
        graphics_application = NULL;

        memset(di.window_station, 0, sizeof(di.window_station));
        memset(di.desktop, 0, sizeof(di.desktop));
        memset(di.display, 0, sizeof(di.display));

        if (!m_bIs9x) {
            // Retrieve the current window station and desktop names
            GetUserObjectInformation(
                GetProcessWindowStation(), 
                UOI_NAME, 
                di.window_station,
                (sizeof(di.window_station)),
                NULL
            );
            GetUserObjectInformation(
                GetThreadDesktop(GetCurrentThreadId()), 
                UOI_NAME, 
                di.desktop,
                sizeof(di.desktop),
                NULL
            );
        }
#else
        char *p = getenv("DISPLAY");
        if (p) strcpy(di.display, p);
        
        graphics_application = 0;
#endif
        retval = rpc->show_graphics(
            rp->project_url.c_str(),
            rp->name.c_str(),
            MODE_FULLSCREEN,
            di
        );
    }
    return retval;
}


// Terminate the graphics application
//
#ifdef _WIN32
int CScreensaver::terminate_screensaver(HANDLE& graphics_application, RESULT *worker_app)
#else
int CScreensaver::terminate_screensaver(int& graphics_application, RESULT *worker_app)
#endif
{
    if (graphics_application) {
        // V6 Graphics
        kill_program(graphics_application);
#ifdef __APPLE__
        // For sandbox security, use gfx_switcher to launch gfx app 
        // as user boinc_project and group boinc_project.
    int retval = 0;
    char current_dir[MAXPATHLEN];
    char gfx_pid[16];
    pid_t thePID;
    int i;

    sprintf(gfx_pid, "%d", graphics_application);
    getcwd( current_dir, sizeof(current_dir));

    char* argv[4];
    argv[0] = "gfx_switcher";
    argv[1] = "-kill_gfx";
    argv[2] = gfx_pid;
    argv[3] = 0;

   retval = run_program(
        current_dir,
        m_gfx_Switcher_Path,
        3,
        argv,
        0,
        thePID
    );
    if (retval) return retval;
    
    for (i=0; i<200; i++) {
        boinc_sleep(0.01);      // Wait 2 seconds max
        // Prevent gfx_switcher from becoming a zombie
        if (waitpid(thePID, 0, WNOHANG) == thePID) break;
    }
    return retval;
#endif
        graphics_application = 0;
    } else {
        // V5 and Older
        DISPLAY_INFO di;

        if (worker_app == NULL) return 0;
        if (worker_app->name.empty()) return 0;

        memset(di.window_station, 0, sizeof(di.window_station));
        memset(di.desktop, 0, sizeof(di.desktop));
        memset(di.display, 0, sizeof(di.display));

        rpc->show_graphics(
            worker_app->project_url.c_str(),
            worker_app->name.c_str(),
            MODE_HIDE_GRAPHICS,
            di
        );
    }
    return 0;
}


#ifdef _WIN32
DWORD WINAPI CScreensaver::DataManagementProc() {
#else
void *CScreensaver::DataManagementProc() {
#endif
    int             retval                  = 0;
    int             suspend_reason          = 0;
    RESULT*         theResult               = NULL;
    RESULT*         graphics_app_result_ptr = NULL;
    RESULT          previous_result;
    RESULT*         previous_result_ptr     = NULL;
    int             iResultCount            = 0;
    int             iIndex                  = 0;
    double          launch_time             = 0.0;
    double          last_change_time        = 0.0;
    double          last_run_check_time     = 0.0;
#ifdef _WIN32
    BOINCTRACE(_T("CScreensaver::DataManagementProc - Display screen saver loading message\n"));
    SetError(TRUE, SCRAPPERR_BOINCSCREENSAVERLOADING);
    m_tThreadCreateTime = time(0);

    // Set the starting point for iterating through the results
    m_iLastResultShown = 0;
    m_tLastResultChangeTime = 0;
#endif         

    while (true) {
        if ((m_dwBlankScreen) && (time(0) > m_dwBlankTime)) {
            BOINCTRACE(_T("CScreensaver::DataManagementProc - Time to blank\n"));
            SetError(FALSE, SCRAPPERR_SCREENSAVERBLANKED);
            m_QuitDataManagementProc = true;
        }

        for (int i = 0; i < 4; i++) {
            // Check for exit request 4 times per second
            if (m_QuitDataManagementProc) {     // If main thread has requested we exit
                if (m_hGraphicsApplication || graphics_app_result_ptr) {
                    terminate_screensaver(m_hGraphicsApplication, graphics_app_result_ptr);
                    graphics_app_result_ptr = NULL;
                    m_hGraphicsApplication = 0;
                    graphics_app_result_ptr = NULL;
                    previous_result_ptr = NULL;
                }
                return 0;       // Exit the thread
            }
            boinc_sleep(0.25);
        }

        if (m_bResetCoreState) {
            // Try and get the current state of the CC
            retval = rpc->get_state(state);
            if (retval) {
                // CC may not yet be running
                HandleRPCError();
                continue;
            }

            m_bResetCoreState = false;
        }
    
        BOINCTRACE(_T("CScreensaver::DataManagementProc - ErrorMode = '%d', ErrorCode = '%x'\n"), m_bErrorMode, m_hrError);

        m_updating_results = true;
        retval = rpc->get_screensaver_tasks(suspend_reason, results);
        m_updating_results = false;
        if (retval) {
            // rpc call returned error
            HandleRPCError();
            m_bResetCoreState = true;
            continue;
        }

	if (suspend_reason != 0) {
            SetError(TRUE, SCRAPPERR_BOINCSUSPENDED);
            if (m_hGraphicsApplication || previous_result_ptr) {
                // use previous_result_ptr because graphics_app_result_ptr may no longer be valid
                terminate_screensaver(m_hGraphicsApplication, previous_result_ptr);
                if (m_hGraphicsApplication == 0) {
                    graphics_app_result_ptr = NULL;
                } else {
                    // waitpid test will clear m_hGraphicsApplication and graphics_app_result_ptr
                }
                previous_result_ptr = NULL;
            }
            continue;
        }

#if SIMULATE_NO_GRAPHICS /* FOR TESTING */

        SetError(TRUE, SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING);

#else                   /* NORMAL OPERATION */

        // Is the current graphics app's associated task still running?
        if ((m_hGraphicsApplication) || (graphics_app_result_ptr)) {

            iResultCount = results.results.size();
            graphics_app_result_ptr = NULL;

            // Find the current task in the new results vector (if it still exists)
            for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                theResult = results.results.at(iIndex);

                if (is_same_task(theResult, previous_result_ptr)) {
                   graphics_app_result_ptr = theResult;
                    previous_result = *theResult;
                    previous_result_ptr = &previous_result;
                    break;
                }
            }

            // V6 graphics only: if worker application has stopped running, terminate_screensaver
            if ((graphics_app_result_ptr == NULL) && (m_hGraphicsApplication != 0)) {
//                if (previous_result_ptr) print_to_log_file("%s finished", previous_result.name.c_str());
                terminate_screensaver(m_hGraphicsApplication, previous_result_ptr);
                previous_result_ptr = NULL;
                // waitpid test will clear m_hGraphicsApplication
           }
#if 0
            // Safety check that task is actually running
            if (last_run_check_time && ((dtime() - last_run_check_time) > TASK_RUN_CHECK_PERIOD)) {
                if (FindProcessPID(NULL, ? ? ?) == 0) {
                    terminate_screensaver(m_hGraphicsApplication, graphics_app_result_ptr);
                    if (m_hGraphicsApplication == 0) {
                        graphics_app_result_ptr = NULL;
                        // Save previous_result and previous_result_ptr for get_random_graphics_app() call
                    } else {
                        // waitpid test will clear m_hGraphicsApplication and graphics_app_result_ptr
                    }
                    previous_result_ptr = NULL;
                }
            }
#endif
            if (last_change_time && ((dtime() - last_change_time) > GFX_CHANGE_PERIOD)) {
                if (count_active_graphic_apps(results, previous_result_ptr) > 0) {
//                    if (previous_result_ptr) print_to_log_file("time to change: %s", previous_result.name.c_str());
                    terminate_screensaver(m_hGraphicsApplication, graphics_app_result_ptr);
                    if (m_hGraphicsApplication == 0) {
                        graphics_app_result_ptr = NULL;
                        // Save previous_result and previous_result_ptr for get_random_graphics_app() call
                    } else {
                        // waitpid test will clear m_hGraphicsApplication and graphics_app_result_ptr
                    }
                }
                last_change_time = dtime();
            }
        }
                
        // If no current graphics app, pick an active task at random and launch its graphics app
        if ((m_hGraphicsApplication == 0) && (graphics_app_result_ptr == NULL)) {
            graphics_app_result_ptr = get_random_graphics_app(results, previous_result_ptr);
            previous_result_ptr = NULL;
            
            if (graphics_app_result_ptr) {
                retval = launch_screensaver(graphics_app_result_ptr, m_hGraphicsApplication);
                if (retval) {
                    m_hGraphicsApplication = 0;
                    previous_result_ptr = NULL;
                    graphics_app_result_ptr = NULL;
                } else {
#ifdef __APPLE__
                    SetError(FALSE, SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING);
#endif
                    SetError(FALSE, SCRAPPERR_SCREENSAVERRUNNING);
                    launch_time = dtime();
                    last_change_time = launch_time;
                    last_run_check_time = launch_time;
                    // Make a local copy of current result, since original pointer 
                    // may have been freed by the time we perform later tests
                    previous_result = *graphics_app_result_ptr;
                    previous_result_ptr = &previous_result;
//                    if (previous_result_ptr) print_to_log_file("launching %s", previous_result.name.c_str());                    
                }
            } else {
                if (state.projects.size() == 0) {
                    // We are not attached to any projects
                    SetError(TRUE, SCRAPPERR_BOINCNOPROJECTSDETECTED);
                } else if (results.results.size() == 0) {
                    // We currently do not have any applications to run
                    SetError(TRUE, SCRAPPERR_BOINCNOAPPSEXECUTING);
                } else {
                    // We currently do not have any graphics capable application
                    SetError(TRUE, m_bV5_GFX_app_is_running ? 
                            SCRAPPERR_DAEMONALLOWSNOGRAPHICS : SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING
                    );
                }
            }
        } else {    // End if ((m_hGraphicsApplication == 0) && (graphics_app_result_ptr == NULL))
            // Is the graphics app still running?
            if (m_hGraphicsApplication) {
#ifdef _WIN32
                DWORD dwStatus = STILL_ACTIVE;
                BOOL  bRetVal = FALSE;
                bRetVal = GetExitCodeProcess(m_hGraphicsApplication, &dwStatus);
                BOINCTRACE(_T("CScreensaver::DataManagementProc - GetExitCodeProcess RetVal = '%d', Status = '%d'\n"), bRetVal, dwStatus);
                if (bRetVal && (dwStatus != STILL_ACTIVE)) {
                    // Something has happened to the previously selected screensaver
                    //   application. Start a different one.
                    m_hGraphicsApplication = 0;
                    graphics_app_result_ptr = NULL;
                    continue;
                } else {
                    CheckForegroundWindow();
                }
#else
                if (waitpid(m_hGraphicsApplication, 0, WNOHANG) == m_hGraphicsApplication) {
                    m_hGraphicsApplication = 0;
                    graphics_app_result_ptr = NULL;
                    continue;
                }
#endif
            }
        }
#endif      // ! SIMULATE_NO_GRAPHICS
    }                           // end while(true)
//            return noErr;       // should never get here; it fixes compiler warning
}
