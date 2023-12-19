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

#ifndef BOINC_WIN_UTIL_H
#define BOINC_WIN_UTIL_H

extern BOOL TerminateProcessById(DWORD dwProcessId);
extern void chdir_to_data_dir();

extern std::wstring boinc_ascii_to_wide(const std::string& str);
extern std::string boinc_wide_to_ascii(const std::wstring& str);

extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize ...
);

#endif
