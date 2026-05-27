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

class CAVerifyInstallDirectories : public BOINCCABase {
public:
    virtual ~CAVerifyInstallDirectories() = default;
    explicit CAVerifyInstallDirectories(MSIHANDLE hMSIHandle) :
        BOINCCABase(hMSIHandle, _T("CAVerifyInstallDirectories"),
            _T("Verify correctness of install and data directories")) {
    }

    UINT OnExecution() override final {
        tstring strInstallDirectory;
        auto uiReturnValue =
            GetProperty(_T("INSTALLDIR"), strInstallDirectory);
        if (uiReturnValue) {
            return uiReturnValue;
        }
        if (strInstallDirectory.empty()) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory is not set. "
                    "Please select a program directory."));
            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        tstring strDataDirectory;
        uiReturnValue = GetProperty(_T("DATADIR"), strDataDirectory);
        if (uiReturnValue) {
            return uiReturnValue;
        }
        if (strDataDirectory.empty()) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The data directory is not set. "
                    "Please select a data directory."));
            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        tstring strWindowsDirectory;
        uiReturnValue = GetProperty(_T("WindowsFolder"), strWindowsDirectory);
        if (uiReturnValue) {
            return uiReturnValue;
        }

        tstring strSystemDrive;
        uiReturnValue = GetProperty(_T("WindowsVolume"), strSystemDrive);
        if (uiReturnValue) {
            return uiReturnValue;
        }

        tstring strWindowsSystemDirectory;
        uiReturnValue = GetProperty(_T("System64Folder"),
            strWindowsSystemDirectory);
        if (uiReturnValue) {
            return uiReturnValue;
        }

        tstring strProgramFilesDirectory;
        uiReturnValue = GetProperty(_T("ProgramFiles64Folder"),
            strProgramFilesDirectory);
        if (uiReturnValue) {
            return uiReturnValue;
        }

        if (strInstallDirectory == strDataDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory and data directory must be "
                    "different. Please select a different data directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strInstallDirectory == strWindowsDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory may not be the Windows directory. "
                    "Please select a different program directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strDataDirectory == strWindowsDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The data directory may not be the Windows directory. "
                    "Please select a different data directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strInstallDirectory == strSystemDrive) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory may not be the system drive. "
                    "Please select a different program directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strDataDirectory == strSystemDrive) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The data directory may not be the system drive. "
                    "Please select a different data directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strInstallDirectory == strWindowsSystemDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory may not be the Windows system "
                    "directory. Please select a different program "
                    "directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strDataDirectory == strWindowsSystemDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The data directory may not be the Windows system "
                    "directory. Please select a different data directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strInstallDirectory == strProgramFilesDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The program directory may not be the program files "
                    "directory. Please select a different program "
                    "directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        if (strDataDirectory == strProgramFilesDirectory) {
            DisplayMessage(MB_OK, MB_ICONERROR,
                _T("The data directory may not be the program files "
                    "directory. Please select a different data directory."));

            SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("0"));
            return ERROR_INSTALL_USEREXIT;
        }

        SetProperty(_T("RETURN_VERIFYINSTALLDIRECTORIES"), _T("1"));
        return ERROR_SUCCESS;
    }
};

UINT __stdcall VerifyInstallDirectories(MSIHANDLE hInstall) {
    return CAVerifyInstallDirectories(hInstall).Execute();
}
