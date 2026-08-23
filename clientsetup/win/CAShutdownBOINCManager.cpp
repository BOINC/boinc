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

#include <TlHelp32.h>

#include "boinccas.h"

class CAShutdownBOINCManager : public BOINCCABase {
public:
    virtual ~CAShutdownBOINCManager() = default;
    explicit CAShutdownBOINCManager(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAShutdownBOINCManager"),
            _T("Shutting down running instances of BOINC Manager")) {
    }
private:
    DWORD findProcessByName(const std::wstring& processName) {
        const auto snapshot =
            CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }
        wil::unique_handle snapshotHandle(snapshot);

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        if (!Process32First(snapshot, &processEntry)) {
            return 0;
        }
        do {
            if (processName == processEntry.szExeFile) {
                return processEntry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processEntry));
        return 0;
    }

    UINT OnExecution() override final {
        TerminateProcessEx(_T("boincmgr.exe"), false);
        auto terminateResult = false;
        for (auto attempt = 0u; attempt <= 5u; ++attempt) {
            if (findProcessByName(_T("boincmgr.exe")) == 0) {
                terminateResult = true;
                break;
            }
            Sleep(1000);
        }

        if (!terminateResult) {
            LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
                _T("One or more BOINC Manager applications "
                    "could not be closed, terminating process(s)."));
            return ERROR_INSTALL_FAILURE;
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall ShutdownBOINCManager(MSIHANDLE hInstall) {
    return CAShutdownBOINCManager(hInstall).Execute();
}
