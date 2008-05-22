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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#pragma once

extern BOOL IsWindows2000Compatible();
extern BOOL IsTerminalServicesEnabled();
extern BOOL ValidateProductSuite(LPSTR SuiteName);
extern BOOL TerminateProcessById(DWORD dwProcessId);
extern BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);
extern BOOL AddAceToDesktop(HDESK hdesk, PSID psid);
extern BOOL GetAccountSid(
    LPCTSTR SystemName,         // where to lookup account
    LPCTSTR AccountName,        // account of interest
    PSID *Sid                   // resultant buffer containing SID
);
extern int suspend_or_resume_threads(DWORD pid, bool resume);
extern void chdir_to_data_dir();
