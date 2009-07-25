// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#pragma once

extern BOOL IsWindows2000Compatible();
extern BOOL IsTerminalServicesEnabled();
extern BOOL TerminateProcessById(DWORD dwProcessId);
extern BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);
extern BOOL AddAceToDesktop(HDESK hdesk, PSID psid);
extern BOOL GetAccountSid(
    LPCSTR SystemName,          // where to lookup account
    LPCSTR AccountName,         // account of interest
    PSID *Sid                   // resultant buffer containing SID
);

extern int suspend_or_resume_threads(DWORD pid, bool resume);
extern void chdir_to_data_dir();
extern bool is_remote_desktop();
