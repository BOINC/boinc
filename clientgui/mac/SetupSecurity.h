// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// SetupSecurity.h

#ifdef _DEBUG
// Comment out this #define for easier debugging of project applications.
// Make it active for better debugging of SANDBOX permissions logic.
// #define DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP
#endif


int CreateBOINCUsersAndGroups(void);
int SetBOINCAppOwnersGroupsAndPermissions(char *path);
int SetBOINCDataOwnersGroupsAndPermissions(void);
int AddAdminUserToGroups(char *user_name);
void ShowSecurityError(const char *format, ...);
