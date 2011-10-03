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

// flags determining what is written to standard out.
// (errors go to stderr)
//
// NOTE: all writes to stdout should have an if (log_flags.*) {} around them.
//

#ifndef _LOGFLAGS_H_
#define _LOGFLAGS_H_

#include <vector>
#include <string>

#ifndef _WIN32
#include <cstdio>
#endif

#include "file_names.h"
#include "cc_config.h"

extern LOG_FLAGS log_flags;
extern CONFIG config;
extern int read_config_file(bool init, const char* fname=CONFIG_FILE);
extern void process_gpu_exclusions();
extern bool gpu_excluded(APP* app, COPROC& cp, int ind);


#endif
