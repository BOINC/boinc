// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

#ifndef BOINC_LOG_FLAGS_H
#define BOINC_LOG_FLAGS_H

#include <vector>
#include <string>

#ifndef _WIN32
#include <cstdio>
#endif

#include "file_names.h"
#include "cc_config.h"

extern LOG_FLAGS log_flags;
extern CC_CONFIG cc_config;
extern int read_config_file(bool init, const char* fname=CONFIG_FILE);
extern void process_gpu_exclusions();
extern bool gpu_excluded(APP* app, COPROC& cp, int ind);
extern void set_no_rsc_config();

#endif
