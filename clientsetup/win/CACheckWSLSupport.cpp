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

#include "boinccas.h"

typedef VOID(WINAPI* RtlGetNtVersionNumbersPtr)(LPDWORD lpMajorVersion,
    LPDWORD lpMinorVersion, LPDWORD lpBuildNumber);

class CACheckWSLSupport : public BOINCCABase {
public:
    virtual ~CACheckWSLSupport() = default;
    explicit CACheckWSLSupport(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CACheckWSLSupport"),
            _T("Check WSL Support")) {}
private:
    UINT OnExecution() override final {
        auto hNTDllLib = GetModuleHandle(_T("ntdll.dll"));
        if (hNTDllLib == nullptr) {
            return ERROR_INSTALL_FAILURE;
        }

        auto rtlGetVersionNumbers =
            reinterpret_cast<RtlGetNtVersionNumbersPtr>(
            GetProcAddress(hNTDllLib, "RtlGetNtVersionNumbers"));

        if (!rtlGetVersionNumbers) {
            return ERROR_INSTALL_FAILURE;
        }

        DWORD major, minor, build;
        rtlGetVersionNumbers(&major, &minor, &build);
        DWORD buildNumber = build & 0xFFFF;

        // WSL is supported on Windows 10 version 2004 (build 19041) and later
        // or version 1903 (build 18362) and later.
        // Windows 11 is fully supported (major version 10 build 22000).

        auto isWSLSupported = false;
        if (major > 10 || (major == 10 && buildNumber >= 18362)) {
            isWSLSupported = true;
        }

        if (isWSLSupported) {
            SetProperty(_T("INSTALLWSLIMAGEINSTALLER"), _T("1"));
            tstring strLaunchBOINCWslImageInstaller;
            const auto returnValue = GetProperty(_T("LAUNCHWSLIMAGEINSTALLER"),
                strLaunchBOINCWslImageInstaller);
            if (returnValue) {
                return returnValue;
            }
            if (!strLaunchBOINCWslImageInstaller.empty() &&
                strLaunchBOINCWslImageInstaller != _T("1")) {
                SetProperty(_T("LAUNCHWSLIMAGEINSTALLER"), _T(""));
            }
            else {
                SetProperty(_T("LAUNCHWSLIMAGEINSTALLER"), _T("1"));
            }
        }
        else {
            SetProperty(_T("INSTALLWSLIMAGEINSTALLER"), _T(""));
            SetProperty(_T("LAUNCHWSLIMAGEINSTALLER"), _T(""));
        }

        return ERROR_SUCCESS;
    }
};

UINT __stdcall CheckWSLSupport(MSIHANDLE hInstall) {
    return CACheckWSLSupport(hInstall).Execute();
}
