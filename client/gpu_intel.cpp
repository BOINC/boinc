// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Detection of Intel GPUs
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "coproc.h"
#include "util.h"

#include "client_msgs.h"
#include "gpu_detect.h"

void COPROC_INTEL::get(
    bool,
    vector<string>&,
    vector<int>&
) {
}
