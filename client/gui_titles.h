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

#define MAX_TABS        5
#define MAX_USAGE_STR   6
#define MAX_MISC_STR    16
#define MAX_LIST_ID     4        // for column titles
#define MAX_COLS        8

extern char g_szTabItems[MAX_TABS][256];
extern char g_szColumnTitles[MAX_LIST_ID][MAX_COLS][256];
extern char g_szUsageItems[MAX_USAGE_STR][256];
extern char g_szMiscItems[MAX_MISC_STR][256];
extern int g_nColSortTypes[MAX_LIST_ID][MAX_COLS];
