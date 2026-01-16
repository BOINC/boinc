// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// MacFixUserGroups.cpp
//
// MacOS updates often change the PrimaryGroupID of boinc_master and
// boinc_project to 20 (staff.) This tiny executable fixes that.
//
// This must be called setuid root.

#include <grp.h>
#include <stdio.h>
#include "mac_spawn.h"

int main(int argc, char *argv[])
{
    struct group    *grp;
    char cmd[1024];

    grp = getgrnam("boinc_master");
    if (grp) {
        snprintf(cmd, sizeof(cmd), "dscl . -create /users/boinc_master PrimaryGroupID %d", grp->gr_gid);
        callPosixSpawn (cmd);
    }

    grp = getgrnam("boinc_project");
    if (grp) {
        snprintf(cmd, sizeof(cmd), "dscl . -create /users/boinc_project PrimaryGroupID %d", grp->gr_gid);
        callPosixSpawn (cmd);
    }

    return 0;
}
