// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "gui_titles.h"

// TODO: the code is riddled with constants that are indices
// into these arrays.  This is bad.
// Also can't these be made char* instead of char[256]s?

char g_szTabItems[MAX_TABS][256] = {
    "Projects",
    "Work",
    "Transfers",
    "Messages",
    "Disk"
};

char g_szColumnTitles[MAX_LIST_ID][MAX_COLS][256] = {
    {"Project",	"Account",		"Total Credit",	"Avg. Credit",	"Resource Share",	"",					""},
    {"Project",	"Application",	"Name",			"CPU time",		"Progress",			"To Completion",	"Report Deadline", "Status"},
    {"Project",	"File",			"Progress",		"Size",			"Time",				"Speed",			"Status"},
    {"Project",	"Time",			"Message",		"",				"",					"",					""}
};

int g_nColSortTypes[MAX_LIST_ID][MAX_COLS] = {
	{0,0,1,1,1,0,0},
	{0,0,0,0,0,0,0},
	{0,0,0,1,0,0,0},
	{0,0,0,0,0,0,0}
};

char g_szUsageItems[MAX_USAGE_STR][256] = {
	"Free space",
	"Used space: non-BOINC",
	"Used space: BOINC",
    "Free space: BOINC",
	"BOINC software",
    "BOINC free space",
};

char g_szMiscItems[MAX_MISC_STR][256] = {
    "New",
    "Running",
    "Ready to run",
    "Computation error", // Computation done
    "Ready to report", // Results uploaded / Files uploaded
    "Acknowledged",
    "Error: invalid state",
    "Completed",
    "Uploading",
    "Downloading",
    "Retry in",
    "Download failed",
    "Upload failed",
    "Suspended",
    "Paused"
};

const char *BOINC_RCSID_7f109697b8 = "$Id$";
