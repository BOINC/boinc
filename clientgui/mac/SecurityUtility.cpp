// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

// Standalone utility to set up BOINC security owners, groups, permissions
// usage:
// first cd to the directory containing BOINCManager.app (usually /Applications)
// then run this application from Terminal
//

//  SecurityUtility.cpp

#include <sys/param.h>  // for MAXPATHLEN
#include <unistd.h>     // for getwd

#include <Carbon/Carbon.h>

#include "SetupSecurity.h"
#include "mac_branding.h"

int main(int argc, char *argv[]) {
    OSStatus            err;
    char boincPath[MAXPATHLEN];

    if (!check_branding_arrays(boincPath, sizeof(boincPath))) {
        printf("Branding array has too few entries: %s\n", boincPath);
        return -1;
    }

    err = CreateBOINCUsersAndGroups();
    if (err != noErr)
        return err;

    err = AddAdminUserToGroups(getenv("USER"));
    if (err != noErr)
        return err;

    boincPath[0] = 0;
    getwd(boincPath);
    //ShowSecurityError("Current Working Directory is %s", wd);

    strlcat(boincPath, "/BOINCManager.app", sizeof(boincPath));
    err = SetBOINCAppOwnersGroupsAndPermissions(boincPath);
    if (err != noErr)
        return err;

    err = SetBOINCDataOwnersGroupsAndPermissions();
    return err;
}

