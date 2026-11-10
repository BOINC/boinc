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
//
// Implementation based on: DCOM Permission Configuration Sample
// Read the license below
//
/*++

DCOM Permission Configuration Sample
Copyright (c) 1996, Microsoft Corporation. All rights reserved.

Module Name:

    dcomperm.h

Abstract:

    Include file for DCOM Permission Configuration sample

Author:

    Michael Nelson

Environment:

    Windows NT

--*/

#pragma once

class DCOMPermissionConfig
{
public:
    DCOMPermissionConfig() = delete;
    ~DCOMPermissionConfig() = delete;
    DCOMPermissionConfig operator=(const DCOMPermissionConfig&) = delete;
    DCOMPermissionConfig(const DCOMPermissionConfig&) = delete;

    static DWORD ChangeAppIDAccessACL(std::wstring_view AppID,
        std::wstring_view Principal);

    static DWORD ChangeAppIDLaunchACL(std::wstring_view AppID,
        std::wstring_view Principal);
};
