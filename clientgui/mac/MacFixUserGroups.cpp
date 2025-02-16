// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#include "mac_spawn.h"

int main(int argc, char *argv[])
{
    callPosixSpawn ("dscl . -create /users/boinc_master PrimaryGroupID boinc_master");
    callPosixSpawn ("dscl . -create /users/boinc_project PrimaryGroupID boinc_project");
    return 0;
}
