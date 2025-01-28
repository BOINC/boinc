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

#ifndef BOINC_PROCINFO_H
#define BOINC_PROCINFO_H

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
    bool scanned;
    double page_fault_rate;        // derived by higher-level code
    std::vector<int> children;
#ifdef _WIN32
    LARGE_INTEGER create_time;
#endif

    PROCINFO() {
      clear();
      working_set_size_smoothed = 0;
    }
    void clear() {
        id = 0;
        parentid = 0;
        swap_size = 0;
        working_set_size = 0;
        //working_set_size_smoothed = 0;
            // *Don't* clear this
        page_fault_count = 0;
        user_time = 0;
        kernel_time = 0;
        is_boinc_app = false;
        is_low_priority = false;
        command[0] = 0;
        scanned = false;
        page_fault_rate = 0;
        children.clear();
    }
};

typedef std::map<int, PROCINFO> PROC_MAP;

extern void find_children(PROC_MAP&);
    // fill in the children fields

extern int procinfo_setup(PROC_MAP&);
    // call this first to get data structure

extern void procinfo_app(
    PROCINFO&, std::vector<int>* other_pids, PROC_MAP&, char* graphics_exec_file
);
    // get info for a given app, and mark processes as BOINC

extern void procinfo_non_boinc(PROCINFO&, PROC_MAP&);
    // After getting info for all BOINC apps,
    // call this to get info for everything else

extern double process_tree_cpu_time(int pid);
    // get the CPU time of the given process and its descendants

extern double total_cpu_time();
    // total user-mode CPU time, as reported by OS

extern double boinc_related_cpu_time(PROC_MAP&, bool vbox_app_running);
    // total CPU of current BOINC processes, low-priority processes,
    // and (if a VBox app is running) VBox-related processes
#endif
