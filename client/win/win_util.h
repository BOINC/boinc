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
