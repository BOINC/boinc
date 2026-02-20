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

        constexpr std::array<std::wstring_view, 12> filesToDelete = {
            _T("boinc.exe"),
            _T("boincmgr.exe"),
            _T("boinccmd.exe"),
            _T("boinc.dll"),
            _T("libcurl.dll"),
            _T("libeay32.dll"),
            _T("ssleay32.dll"),
            _T("zlib1.dll"),
            _T("dbghelp.dll"),
            _T("dbghelp95.dll"),
            _T("srcsrv.dll"),
            _T("symsrv.dll")
        };

        for (auto file : filesToDelete) {
            DeleteFile(
                (strInstallDirectory + _T("\\") + file.data()).c_str());
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall CleanupOldBinaries(MSIHANDLE hInstall) {
    return CACleanupOldBinaries(hInstall).Execute();
}
