// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// function declarations
int UtilGetRegKey(char *name, DWORD &keyval);
int UtilSetRegKey(char *name, DWORD value);
int UtilGetRegStr(char *name, char *str);
int UtilSetRegStr(char *name, char *str);
int UtilGetRegStartupStr(char *name, char *str);
int UtilSetRegStartupStr(char *name, char *str);
int UtilInitOSVersion( void );

#define START_SS_MSG		"BOINC_SS_START"
#define STOP_SS_MSG			"BOINC_SS_END"
#define SHOW_WIN_MSG		"BOINC_SHOW_MESSAGE"
#define RUN_MUTEX			"BOINC_MUTEX"
#define NET_ACTIVITY_MSG    "BOINC_NET_ACTIVITY"

#define REG_BLANK_NAME		"Blank"
#define REG_BLANK_TIME      "Blank Time"
#define	REG_STARTUP_NAME	"BOINC"