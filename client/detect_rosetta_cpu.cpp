// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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

// This helper program is used to detect an emulated x86_64 CPU on Apples
// ARM64 CPUs (M1). It should be compiled _only_ for x86_64 architecture.
// It writes the feature string of the emulated CPU to a file
// EMULATED_CPU_INFO_FILENAME in the current working directory.

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include "hostinfo.h"   // for P_FEATURES_SIZE
#include "filesys.h"    // for boinc_fopen()
#include "file_names.h" // for EMULATED_CPU_INFO_FILENAME

int main () {
    size_t len;
    char features[P_FEATURES_SIZE];
    FILE*fp;

    len = sizeof(features);
    sysctlbyname("machdep.cpu.features", features, &len, NULL, 0);
    if ((fp = boinc_fopen(EMULATED_CPU_INFO_FILENAME, "w"))) {
        fprintf(fp," %s\n", features);
        fclose(fp);
    }
    return 0;
}
