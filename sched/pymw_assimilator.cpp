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

// A sample assimilator that:
// 1) if success, copy the output file(s) to a directory
// 2) if failure, append a message to an error log
//
#include <vector>
#include <string>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"
#include "assimilate_handler.h"

using std::vector;
using std::string;

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& /*results*/, RESULT& canonical_result
) {
    SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::MSG_NORMAL);
    // Parse pym_dir
    string pymw_dir("");
    for (int i=1; i<g_argc;i++) {
        if(!strcmp(g_argv[i], "-pymw_dir")) {
            pymw_dir = string(g_argv[++i]);
        }
    }
    // Check if pymw working directory is properly set
    if (pymw_dir.length() == 0) {
        SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::MSG_CRITICAL);
        scope_messages.printf("PyMW working directory is not set\n");
        return -1;
    }

    if (pymw_dir[pymw_dir.length()-1] != '/') {
        // Append / to the path
        pymw_dir.append("/");
    }

    scope_messages.printf("[%s] Assimilating...\n", wu.name);
        
    int retval;
    char buf[1024];
    unsigned int i;

    if (wu.canonical_resultid) {
        vector<string> output_file_paths;
        scope_messages.printf("[%s] Found canonical result\n", wu.name);
    
        char copy_path[256];
    
        get_output_file_paths(canonical_result, output_file_paths);
        unsigned int n = output_file_paths.size();
    
        for (i = 0; i < n; i++) {
            string s = string(canonical_result.xml_doc_in);
            string::size_type start = s.find("<open_name>", 0);
            string::size_type end = s.find("</open_name>", 0);
            string name = string(s, start+11, end - start - 11);
        
            // absoulte path to copy result to
            string target(pymw_dir);
            target = target.append(name);
    
            string source = output_file_paths[i];
        
            retval = boinc_copy(source.c_str() , target.c_str());
            if (retval) {
                scope_messages.printf("[%s] An error occured, while copying result to %s\n", wu.name, target.c_str());
                return retval;
            }
        }
    } else {
        scope_messages.printf("[%s] Workunit failed 0x%x\n", wu.name, wu.error_mask);
        return -1;
    }
    return 0;
}
