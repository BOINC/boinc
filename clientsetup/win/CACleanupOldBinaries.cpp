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

class CACleanupOldBinaries : public BOINCCABase {
public:
    virtual ~CACleanupOldBinaries() = default;

    explicit CACleanupOldBinaries(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CACleanupOldBinaries"),
            _T("Cleanup any old binaries that were left lying around from "
                "some other install.")) {
    }

    UINT OnExecution() override final {
        tstring strInstallDirectory;

        const auto uiReturnValue =
            GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (uiReturnValue != ERROR_SUCCESS) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            return ERROR_INSTALL_FAILURE;
        }

        DeleteFile(tstring(strInstallDirectory +
            _T("\\boinc.exe")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\boincmgr.exe")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\boinccmd.exe")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\boinc.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\libcurl.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\libeay32.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\ssleay32.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\zlib1.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\dbghelp.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\dbghelp95.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\srcsrv.dll")).c_str());
        DeleteFile(tstring(strInstallDirectory +
            _T("\\symsrv.dll")).c_str());

        return ERROR_SUCCESS;
    }
};

UINT __stdcall CleanupOldBinaries(MSIHANDLE hInstall) {
    return CACleanupOldBinaries(hInstall).Execute();
}
