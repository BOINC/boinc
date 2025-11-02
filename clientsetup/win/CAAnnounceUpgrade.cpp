// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include "stdafx.h"
#include "boinccas.h"
#include "CAAnnounceUpgrade.h"

CAAnnounceUpgrade::CAAnnounceUpgrade(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, _T("CAAnnounceUpgrade"),
        _T("Announce the new BOINC version to all components.")) {}

UINT CAAnnounceUpgrade::OnExecution() {
    return SetUpgradeParameters();
}

UINT __stdcall AnnounceUpgrade(MSIHANDLE hInstall) {
    return CAAnnounceUpgrade(hInstall).Execute();
}
