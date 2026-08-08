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
//
//  mac_branding.cpp
//

#include <string.h>
#include "mac_branding.h"

// The strings for these arrays are initialized here rather than in mac_branding.h, to avoid
// duplicate definition errors when mac_branding.h is included from multiple source files.

// appName[] is used by PostInstall.cpp, uninstall.cpp and AddRemoveuser.cpp
char *appName[] = {
                "BOINCManager",
                "GridRepublic Desktop",
                "Progress Thru Processors Desktop",
                "Charity Engine Desktop",
                "World Community Grid"
                };

// appPath[] is used by PostInstall.cpp, uninstall.cpp, AddRemoveUser.cpp and mac_saver_module.cpp
char *appPath[] =  {
                "/Applications/BOINCManager.app",
                "/Applications/GridRepublic Desktop.app",
                "/Applications/Progress Thru Processors Desktop.app",
                "/Applications/Charity Engine Desktop.app",
                "/Applications/World Community Grid.app"
                };

// brandName[] is used by PostInstall.cpp, uninstall.cpp and AddRemoveuser.cpp
char *brandName[] = {
                "BOINC",
                "GridRepublic",
                "Progress Thru Processors",
                "Charity Engine",
                "World Community Grid"
                };

// saverName[] is used by PostInstall.cpp, uninstall.cpp, AddRemoveuser.cpp,
// check_security.cpp and SetupSecurity.cpp
char *saverName[] = {
                "BOINCSaver",
                "GridRepublic",
                "Progress Thru Processors",
                "Charity Engine",
                "World Community Grid"
                };

// receiptName[] is used by PostInstall.cpp and uninstall.cpp
char *receiptName[] = {
                "/Library/Receipts/BOINC Installer.pkg",
                "/Library/Receipts/GridRepublic Installer.pkg",
                "/Library/Receipts/Progress Thru Processors Installer.pkg",
                "/Library/Receipts/Charity Engine Installer.pkg",
                "/Library/Receipts/World Community Grid Installer.pkg"
                };

// skinName[] is used by PostInstall.cpp
char *skinName[] = {
                "Default",
                "GridRepublic",
                "Progress Thru Processors",
                "Charity Engine",
                "World Community Grid"
                };

// logoFile[] is used by ss_app.cpp
char *logoFile[] = {
                "boinc_logo_black.jpg",
                "gridrepublic_ss_logo.jpg",
                "progress_ss_logo.jpg",
                "CE_ss_logo.jpg",
                "wcg_ss_logo.jpg"
                };

// Returns false if any of these arrays has < NUMBRANDS entries
bool check_branding_arrays(char* badArrayNames, int len) {
    bool isOK = true;
    *badArrayNames = '\0';

    if(appName[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "appName ", len);
    }
    if(appPath[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "appPath ", len);
    }
    if(brandName[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "brandName ", len);
    }
    if(saverName[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "saverName ", len);
    }
    if(receiptName[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "receiptName ", len);
    }
    if(skinName[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "skinName ", len);
    }
    if(logoFile[NUMBRANDS-1]==NULL) {
        isOK = false;
        strlcat(badArrayNames, "logoFile ", len);
    }

    return isOK;
}
