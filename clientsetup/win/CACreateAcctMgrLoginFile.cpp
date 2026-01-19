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
#include "CACreateAcctMgrLoginFile.h"

CACreateAcctMgrLoginFile::CACreateAcctMgrLoginFile(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, _T("CACreateAcctMgrLoginFile"),
        _T("Store account manager initialization data")) {
}

UINT CACreateAcctMgrLoginFile::OnExecution() {
    tstring strDataDirectory;
    auto uiReturnValue = GetProperty(_T("DATADIR"), strDataDirectory);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }
    if (strDataDirectory.empty()) {
        return ERROR_INSTALL_FAILURE;
    }
    if (!std::filesystem::exists(strDataDirectory)) {
        return ERROR_INSTALL_FAILURE;
    }

    tstring login;
    uiReturnValue = GetProperty(_T("ACCTMGR_LOGIN"), login);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    if (login.empty()) {
        return ERROR_SUCCESS;
    }

    tstring pwdHash;
    uiReturnValue = GetProperty(_T("ACCTMGR_PASSWORDHASH"), pwdHash);
    if (uiReturnValue != ERROR_SUCCESS) {
        return uiReturnValue;
    }

    // The project_init.xml file is stored in the data directory.
    //
    const auto strAcctMgrLoginFile =
        strDataDirectory + _T("\\acct_mgr_login.xml");
    std::wofstream fAcctMgrLoginFile(strAcctMgrLoginFile);
    fAcctMgrLoginFile <<
        _T("<acct_mgr_login>\n") <<
        _T("    <login>") << login << _T("</login>\n") <<
        _T("    <password_hash>") << pwdHash << _T("</password_hash>\n") <<
        _T("</acct_mgr_login>\n");
    fAcctMgrLoginFile.close();

    return ERROR_SUCCESS;
}

UINT __stdcall CreateAcctMgrLoginFile(MSIHANDLE hInstall) {
    return CACreateAcctMgrLoginFile(hInstall).Execute();
}
