// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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

//  SecurityUtility.cpp

#include <sys/param.h>  // for MAXPATHLEN
#include <unistd.h>     // for getwd, getlogin

#include <Carbon/Carbon.h>

#include "SetupSecurity.h"

// Standalone utility to set up BOINC security owners, groups, permissions

int main(int argc, char *argv[]) {
    OSStatus            err;
    char boincPath[MAXPATHLEN];
    
    err = CreateBOINCUsersAndGroups();
    if (err != noErr)
        return err;

    err = AddAdminUserToGroups(getlogin());
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

