
#define MAX_TABS		5
#define MAX_USAGE_STR		5
#define MAX_MISC_STR		13
#define MAX_LIST_ID		4			// for column titles
#define MAX_COLS		7

char g_szTabItems[MAX_TABS][256] = {
	"Projects",
	"Work",
	"Transfers",
	"Messages",
	"Disk"
};

char g_szColumnTitles[MAX_LIST_ID][MAX_COLS][256] = {
        {"Project",	"Account",		"Total Credit",	"Avg. Credit",	"Resource Share",	"",					""},
        {"Project",	"Application",	"Name",			"CPU time",		"Progress",			"To Completion",	"Status"},
        {"Project",	"File",			"Progress",		"Size",			"Time",				"Status",		""},
        {"Project",	"Time",			"Message",		"",				"",					"",					""}
};

char g_szUsageItems[MAX_USAGE_STR][256] = {
	"Free space: not available for use",
	"Free space: available for use",
	"Used space: other than BOINC",
	"Used space: BOINC",
	"Used space:"
};

char g_szMiscItems[MAX_MISC_STR][256] = {
	"New",
	"Running",
	"Ready to run",
	"Computation done",
	"Results uploaded",
	"Acknowledged",
	"Error: invalid state",
	"Completed",
	"Uploading",
	"Downloading",
	"Retry in",
	"Download failed",
	"Upload failed"
};
