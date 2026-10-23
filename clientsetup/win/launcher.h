// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#ifdef _UNICODE
#define tstring_view std::wstring_view
#else
// we use this when building installer_setup project only
// a little bit ugly but omits code duplication
#include <windows.h>
#include <string_view>
#include <wil/resource.h>
#include "win_util.h"
#define tstring_view std::string_view
#define _T(x) x
#endif

class Launcher {
public:
    Launcher() = delete;
    ~Launcher() = delete;
    Launcher operator=(const Launcher&) = delete;
    Launcher(const Launcher&) = delete;

    static HRESULT CreateProcessWithExplorerIL(tstring_view szCmdLine);
};
