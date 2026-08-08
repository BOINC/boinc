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

#ifndef BOINC_PROJECT_INIT_H
#define BOINC_PROJECT_INIT_H

#define PROJECT_INIT_FILENAME       "project_init.xml"

// represents the contents of project_init.xml,
// specifying a project, and optionally an account, to attach to initially
//
class PROJECT_INIT {
public:
    char url[256];
    char name[256];
    char account_key[256];

    // Is the project_init.xml file embedded in the installer?
    // Or was it generated from of the command line of the install.
    // No longer used.
    //
    bool embedded;

    PROJECT_INIT();
    int init();
    int remove();
    void clear();

	// Used by installers to create/modify project_init.xml in the data
	// directory at install time.
    // Useful for creating automated install processes.
    //
    int write();
};

#endif
