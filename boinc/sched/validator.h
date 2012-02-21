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

#include "sched_config.h"

extern double max_granted_credit;
    // the --max_granted_credit cmdline arg, or 0
extern double max_claimed_credit;
    // the --max_claimed_credit cmdline arg, or 0
extern bool grant_claimed_credit;
    // the --grant_claimed_credit cmdline arg, or false

extern WORKUNIT* g_wup;
    // A pointer to the WU currently being processed;
    // you can use this in your init_result() etc. functions
