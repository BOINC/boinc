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

#ifndef _PROCINFO_
#define _PROCINFO_

#include <vector>

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

	double page_fault_rate;		// derived by higher-level code
};

extern int procinfo_setup(std::vector<PROCINFO>&);
	// call this first to get data structure
extern void procinfo_app(PROCINFO&, std::vector<PROCINFO>&);
	// call this to get mem usage for a given app
	// (marks process as BOINC)
extern void procinfo_other(PROCINFO&, std::vector<PROCINFO>&);
	// After getting mem usage for all BOINC apps,
	// call this to get mem usage for everything else

#endif
