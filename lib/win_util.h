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

#ifndef _WIN_UTIL_
#define _WIN_UTIL_

extern BOOL IsWindows2000Compatible();
extern BOOL IsTerminalServicesEnabled();
extern BOOL ValidateProductSuite(LPSTR SuiteName);
extern BOOL TerminateProcessById(DWORD dwProcessId);
extern BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);
extern BOOL AddAceToDesktop(HDESK hdesk, PSID psid);
extern BOOL GetAccountSid(
    LPCSTR SystemName,          // where to lookup account
    LPCSTR AccountName,         // account of interest
    PSID *Sid                   // resultant buffer containing SID
);

inline std::wstring A2W(const std::string& str) {
  int length_wide = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.length(), NULL, 0);
  wchar_t *string_wide = static_cast<wchar_t*>(_alloca(length_wide * sizeof(wchar_t)));
  MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.length(), string_wide, length_wide);
  std::wstring result(string_wide, length_wide);
  return result;
}

inline std::string W2A(const std::wstring& str) {
  int length_ansi = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.length(), NULL, 0, NULL, NULL);
  char* string_ansi = static_cast<char*>(_alloca(length_ansi));
  WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.length(), string_ansi, length_ansi, NULL, NULL);
  std::string result(string_ansi, length_ansi);
  return result;
}

extern int suspend_or_resume_threads(DWORD pid, DWORD threadid, bool resume);
extern void chdir_to_data_dir();
extern bool is_remote_desktop();

#endif // _WIN_UTIL_
