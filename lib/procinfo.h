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

#ifndef _PROCINFO_
#define _PROCINFO_

#include <vector>
#include <map>

struct PROCINFO {
	int id;
	int parentid;
    double swap_size;
    double working_set_size;
	double working_set_size_smoothed;
	unsigned long page_fault_count;
    double user_time;
    double kernel_time;
	bool is_boinc_app;
    bool is_low_priority;
        // running at or below priority of BOINC apps
    char command[256];

	double page_fault_rate;		// derived by higher-level code
    std::vector<int> children;
};

typedef std::map<int, PROCINFO> PROC_MAP;

extern void find_children(PROC_MAP&);
    // fill in the children fields

extern int procinfo_setup(PROC_MAP&);
	// call this first to get data structure
extern void procinfo_app(PROCINFO&, PROC_MAP&, char* graphics_exec_file);
	// get mem usage for a given app (marks process as BOINC)
extern void procinfo_non_boinc(PROCINFO&, PROC_MAP&);
	// After getting info for all BOINC apps,
	// call this to get info for everything else
extern void get_descendants(int pid, std::vector<int>& pids);
extern bool any_process_exists(std::vector<int>& pids);
extern void kill_all(std::vector<int>& pids);
#ifdef _WIN32
extern void kill_descendants();
#else
extern void kill_descendants(int child_pid=0);
#endif
extern void suspend_or_resume_descendants(int pid, bool resume);
extern void suspend_or_resume_process(int pid, bool resume);

#endif
