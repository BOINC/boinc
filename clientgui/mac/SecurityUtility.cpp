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

#define CREATE_LOG 1    /* for debugging */

#include <sys/param.h>  // for MAXPATHLEN
#include <unistd.h>     // for getwd
#include <sys/stat.h>   // for chmod

#include <Carbon/Carbon.h>

#include "SetupSecurity.h"
#include "mac_branding.h"

void print_to_log_file(const char *format, ...);

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

void strip_cr(char *buf)
{
    char *theCR;

    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}

// For debugging
void print_to_log_file(const char *format, ...) {
    va_list args;
    char buf[256];
    time_t t;
    sprintf(buf, "/Users/Shared/test_log_finish_install.txt");
    FILE *f;
    f = fopen(buf, "a");
    if (!f) return;

    // File may be owned by various users, so make it world readable & writable
    chmod(buf, 0666);

    time(&t);
    strlcpy(buf, asctime(localtime(&t)), sizeof(buf));

    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    fputs("\n", f);
    fflush(f);
    fclose(f);
}
