
// function declarations
int UtilGetRegKey(char *name, DWORD &keyval);
int UtilSetRegKey(char *name, DWORD value);
int UtilGetRegStr(char *name, char *str);
int UtilSetRegStr(char *name, char *str);
int UtilSetRegStartupStr(char *name, char *str);
int UtilInitOSVersion( void );

#define START_SS_MSG		"BOINC_SS_START"
#define SHOW_WIN_MSG		"BOINC_SHOW_MESSAGE"
#define RUN_MUTEX			"BOINC_MUTEX"
#define NET_ACTIVITY_MSG    "BOINC_NET_ACTIVITY"
#define APP_SET_MSG			"BOINC_APP_SET"
#define APP_GET_MSG			"BOINC_APP_GET"
