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


// Choose a random graphics application out of the vector.
// NOTE: Right now it just selects the first graphics app
//   found.
RESULT* random_graphics_app(RESULTS& results) {
    bool     bIsActive       = false;
    bool     bIsExecuting    = false;
    bool     bIsDownloaded   = false;

    for (unsigned i=0; i < results.results.size(); i++) {
        if (results.results[i]->graphics_exec_path.size() > 0) {
            bIsDownloaded = (RESULT_FILES_DOWNLOADED == results.results[i]->state);
            bIsActive     = (results.results[i]->active_task);
            bIsExecuting  = (CPU_SCHED_SCHEDULED == results.results[i]->scheduler_state);
            if (!(bIsActive) || !(bIsDownloaded) || !(bIsExecuting)) continue;
	        return results.results[i];
        }
    }
    return NULL;
}

