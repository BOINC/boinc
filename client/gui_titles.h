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

#define MAX_TABS        5
#define MAX_USAGE_STR   6
#define MAX_MISC_STR    16
#define MAX_LIST_ID     4        // for column titles
#define MAX_COLS        8

extern char g_szTabItems[MAX_TABS][256];
extern char g_szColumnTitles[MAX_LIST_ID][MAX_COLS][256];
extern char g_szUsageItems[MAX_USAGE_STR][256];
extern char g_szMiscItems[MAX_MISC_STR][256];
