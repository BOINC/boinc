#include "gui_titles.h"

// TODO: the code is riddles with constants that are indices
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

char g_szUsageItems[MAX_USAGE_STR][256] = {
	"Free space",
	"Used space: non-BOINC",
	"Used space: BOINC",
	"BOINC Core Client",
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
    "Suspended"
};
