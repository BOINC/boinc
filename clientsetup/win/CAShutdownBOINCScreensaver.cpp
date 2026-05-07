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

#include "stdafx.h"
#include "boinccas.h"
#include "terminate.h"

#define CUSTOMACTION_NAME               
#define CUSTOMACTION_PROGRESSTITLE      

class CAShutdownBOINCScreensaver : public BOINCCABase {
public:
    virtual ~CAShutdownBOINCScreensaver() = default;
    explicit CAShutdownBOINCScreensaver(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAShutdownBOINCScreensaver"),
            _T("Shutting down running instances of BOINC Screensaver")) {
    }

    UINT OnExecution() override final {
        TerminateProcessEx(_T("boinc.scr"));
        return ERROR_SUCCESS;
    }
};

UINT __stdcall ShutdownBOINCScreensaver(MSIHANDLE hInstall) {
    return CAShutdownBOINCScreensaver(hInstall).Execute();
}
