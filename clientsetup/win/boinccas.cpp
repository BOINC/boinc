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
#include "terminate.h"
#include "dcomperm.h"
#include "lsaprivs.h"
#include "launcher.h"

BOINCCABase::BOINCCABase(MSIHANDLE hMSIHandle, tstring strActionName,
    tstring strProgressTitle) : m_hMSIHandle(hMSIHandle),
    m_strActionName(std::move(strActionName)),
    m_strProgressTitle(std::move(strProgressTitle)) {
}

UINT BOINCCABase::Execute() {
    const auto uiReturnValue = OnInitialize();
    if (uiReturnValue) {
        return uiReturnValue;
    }

    if (MsiGetMode(m_hMSIHandle, MSIRUNMODE_SCHEDULED) ||
        MsiGetMode(m_hMSIHandle, MSIRUNMODE_COMMIT) ||
        MsiGetMode(m_hMSIHandle, MSIRUNMODE_ROLLBACK)) {
        return uiReturnValue;
    }
    return OnExecution();
}

static auto IsVersionNewer(const tstring& v1, const tstring& v2) {
    int v1_maj = 0, v1_min = 0, v1_rel = 0;
    int v2_maj = 0, v2_min = 0, v2_rel = 0;

    swscanf_s(v1.c_str(), _T("%d.%d.%d"), &v1_maj, &v1_min, &v1_rel);
    swscanf_s(v2.c_str(), _T("%d.%d.%d"), &v2_maj, &v2_min, &v2_rel);

    if (v1_maj > v2_maj) {
        return true;
    }
    if (v1_maj < v2_maj) {
        return false;
    }
    if (v1_min > v2_min) {
        return true;
    }
    if (v1_min < v2_min) {
        return false;
    }
    if (v1_rel > v2_rel) {
        return true;
    }

    return false;
}

bool BOINCCABase::IsUpgrading() {
    tstring strCurrentProductVersion;
    auto uiReturnValue = GetProperty(_T("ProductVersion"),
        strCurrentProductVersion);
    if (uiReturnValue) {
        return FALSE;
    }

    tstring strRegistryProductVersion;
    uiReturnValue = GetRegistryValue(_T("UpgradingTo"),
        strRegistryProductVersion);
    if (uiReturnValue) {
        return FALSE;
    }

    return IsVersionNewer(strRegistryProductVersion, strCurrentProductVersion);
}

UINT BOINCCABase::OnInitialize() {
    PMSIHANDLE m_phActionStartRec = MsiCreateRecord(3);

    MsiRecordSetString(m_phActionStartRec, 1, m_strActionName.data());
    MsiRecordSetString(m_phActionStartRec, 2, m_strProgressTitle.data());
    MsiRecordSetString(m_phActionStartRec, 3, _T("[1]"));

    auto uiReturnValue = MsiProcessMessage(m_hMSIHandle,
        INSTALLMESSAGE_ACTIONSTART, m_phActionStartRec);
    if (uiReturnValue == IDCANCEL) {
        return ERROR_INSTALL_USEREXIT;
    }

    // Give the UI a chance to refresh.
    Sleep(0);

    m_phActionDataRec = MsiCreateRecord(3);
    PMSIHANDLE m_phProgressRec = MsiCreateRecord(3);

    MsiRecordSetInteger(m_phProgressRec, 1, 1);
    MsiRecordSetInteger(m_phProgressRec, 2, 1);
    MsiRecordSetInteger(m_phProgressRec, 3, 0);
    uiReturnValue = MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS,
        m_phProgressRec);
    if (uiReturnValue == IDCANCEL) {
        return ERROR_INSTALL_USEREXIT;
    }

    m_phLogInfoRec = MsiCreateRecord(3);

    MsiRecordSetString(m_phLogInfoRec, 0,
        _T("Custom Message : Action Name: [1] Description: [2] "
            "Error Code: [3] "));
    MsiRecordSetString(m_phLogInfoRec, 1, m_strActionName.data());

    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, _T("Starting Custom Action"));

    return ERROR_SUCCESS;
}

UINT BOINCCABase::GetRegistryValue(const tstring& strName,
    tstring& strValue, bool bDisplayValue) {
    HKEY hkSetupHive;
    auto lReturnValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
        0, KEY_READ, &hkSetupHive);
    if (lReturnValue != ERROR_SUCCESS) {
        return ERROR_INSTALL_FAILURE;
    }
    wil::unique_hkey hKeyDeleter(hkSetupHive);

    DWORD dwSize = 0;
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, nullptr, &dwSize);
    if (lReturnValue != ERROR_SUCCESS) {
        return ERROR_INSTALL_FAILURE;
    }

    strValue.resize(dwSize / sizeof(TCHAR));
    lReturnValue = RegQueryValueEx(hkSetupHive, strName.data(), nullptr,
        nullptr, reinterpret_cast<LPBYTE>(strValue.data()), &dwSize);

    if (lReturnValue != ERROR_SUCCESS) {
        return ERROR_INSTALL_FAILURE;
    }

    if (strValue.back() == _T('\0')) {
        strValue.pop_back();
    }

    tstring strMessage = _T("Successfully retrieved registry value '");
    strMessage += strName;
    strMessage += _T("' with a value of '");
    if (bDisplayValue) {
        strMessage += strValue;
    }
    else {
        strMessage += _T("<Value Hidden>");
    }
    strMessage += _T("'");

    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);
    return ERROR_SUCCESS;
}

UINT BOINCCABase::SetRegistryValue(const tstring& strName,
    const tstring& strValue, bool bDisplayValue) {
    HKEY hkSetupHive;
    auto lReturnValue = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr,
        &hkSetupHive, nullptr);
    if (lReturnValue != ERROR_SUCCESS) {
        return ERROR_INSTALL_FAILURE;
    }
    wil::unique_hkey hkeyDeleter(hkSetupHive);

    lReturnValue = RegSetValueEx(hkSetupHive, strName.data(), 0, REG_SZ,
        reinterpret_cast<const BYTE*>(strValue.data()),
        static_cast<DWORD>((strValue.size() + 1) * sizeof(TCHAR)));

    if (lReturnValue != ERROR_SUCCESS) {
        return ERROR_INSTALL_FAILURE;
    }

    tstring strMessage = _T("Successfully set registry value '");
    strMessage += strName;
    strMessage += _T("' to a value of '");
    if (bDisplayValue) {
        strMessage += strValue;
    }
    else {
        strMessage += _T("<Value Hidden>");
    }
    strMessage += _T("'");

    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);

    return ERROR_SUCCESS;
}

UINT BOINCCABase::GetProperty(const tstring& strPropertyName,
    tstring& strPropertyValue, bool bDisplayValue) {
    DWORD dwCharacterCount = 0;
    auto uiReturnValue = MsiGetProperty(m_hMSIHandle, strPropertyName.data(),
        _T(""), &dwCharacterCount);
    if (uiReturnValue == ERROR_INVALID_HANDLE ||
        uiReturnValue == ERROR_INVALID_PARAMETER) {
        tstring strMessage = _T("Failed to get '");
        strMessage += strPropertyName;
        strMessage += _T("'");

        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);
        return ERROR_INSTALL_FAILURE;
    }

    strPropertyValue.resize(dwCharacterCount++);
    uiReturnValue = MsiGetProperty(m_hMSIHandle, strPropertyName.data(),
        strPropertyValue.data(), &dwCharacterCount);
    if (uiReturnValue == ERROR_INVALID_HANDLE ||
        uiReturnValue == ERROR_INVALID_PARAMETER) {
        tstring strMessage = _T("Failed to get '");
        strMessage += strPropertyName;
        strMessage += _T("' after allocating the buffer");

        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);
        return ERROR_INSTALL_FAILURE;
    }

    tstring strMessage = _T("Successfully retrieved '");
    strMessage += strPropertyName;
    strMessage += _T("' with a value of '");
    if (bDisplayValue) {
        strMessage += strPropertyValue;
    }
    else {
        strMessage += _T("<Value Hidden>");
    }
    strMessage += _T("'");

    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);

    return ERROR_SUCCESS;
}

UINT BOINCCABase::SetProperty(const tstring& strPropertyName,
    const tstring& strPropertyValue, bool bDisplayValue) {
    const auto uiReturnValue = MsiSetProperty(m_hMSIHandle,
        strPropertyName.data(), strPropertyValue.data());
    if (uiReturnValue == ERROR_FUNCTION_FAILED ||
        uiReturnValue == ERROR_INVALID_HANDLE ||
        uiReturnValue == ERROR_INVALID_PARAMETER) {
        tstring strMessage = _T("Failed to set '");
        strMessage += strPropertyName;
        strMessage += _T("' to a value of '");
        if (bDisplayValue) {
            strMessage += strPropertyValue;
        }
        else {
            strMessage += _T("<Value Hidden>");
        }
        strMessage += _T("'");
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);
        return ERROR_INSTALL_FAILURE;
    }

    tstring strMessage = _T("Successfully set '");
    strMessage += strPropertyName;
    strMessage += _T("' to a value of '");
    if (bDisplayValue) {
        strMessage += strPropertyValue;
    }
    else {
        strMessage += _T("<Value Hidden>");
    }
    strMessage += _T("'");
    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);

    return ERROR_SUCCESS;
}

UINT BOINCCABase::GetComponentKeyFilename(const tstring& strComponentName,
    tstring& strComponentKeyFilename) {
    PMSIHANDLE hDatabase = MsiGetActiveDatabase(m_hMSIHandle);
    if (!hDatabase) {
        return ERROR_INSTALL_FAILURE;
    }

    constexpr auto strQuery =
        _T("SELECT `KeyPath` FROM `Component` WHERE `Component`= ?");

    PMSIHANDLE hView;
    auto uiReturnValue = MsiDatabaseOpenView(hDatabase, strQuery, &hView);
    if (uiReturnValue == ERROR_BAD_QUERY_SYNTAX) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiDatabaseOpenView reports an invalid query was issued"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiDatabaseOpenView reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }

    wil::unique_any<MSIHANDLE, decltype(&MsiViewClose), MsiViewClose>
        hViewCloser(hView);

    PMSIHANDLE hRecComponentName = MsiCreateRecord(1);
    uiReturnValue =
        MsiRecordSetString(hRecComponentName, 1, strComponentName.data());
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordSetString reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_PARAMETER) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordSetString reports an invalid parameter was used"));
        return ERROR_INSTALL_FAILURE;
    }

    // Execute the query
    uiReturnValue = MsiViewExecute(hView, hRecComponentName);
    if (uiReturnValue == ERROR_FUNCTION_FAILED) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewExecute failed to execute the view"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewExecute reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }

    PMSIHANDLE hRec;
    uiReturnValue = MsiViewFetch(hView, &hRec);
    if (uiReturnValue == ERROR_FUNCTION_FAILED) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewFetch: An error occurred during fetching"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewFetch reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_HANDLE_STATE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewFetch reports the handle was in an invalid state"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_NO_MORE_ITEMS) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiViewFetch reports no valid items fetched"));
        return ERROR_INSTALL_FAILURE;
    }

    DWORD dwBufferSize = 0;
    uiReturnValue = MsiRecordGetString(hRec, 1, _T(""), &dwBufferSize);
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordGetString reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_PARAMETER) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordGetString reports an invalid parameter was used"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue != ERROR_MORE_DATA) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordGetString failed to get the required buffer size"));
        return ERROR_INSTALL_FAILURE;
    }

    strComponentKeyFilename.resize(dwBufferSize++);
    uiReturnValue = MsiRecordGetString(hRec, 1,
        strComponentKeyFilename.data(), &dwBufferSize);
    if (uiReturnValue == ERROR_INVALID_HANDLE) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordGetString reports an invalid handle was used"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiReturnValue == ERROR_INVALID_PARAMETER) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("MsiRecordGetString reports an invalid parameter was used"));
        return ERROR_INSTALL_FAILURE;
    }

    // strComponentKeyFilename format is [strComponentName]_[filename]
    // Remove the component name from the string
    strComponentKeyFilename =
        strComponentKeyFilename.substr(strComponentName.size() + 1);

    tstring strMessage = _T("The key filename for component '");
    strMessage += strComponentName;
    strMessage += _T("' is '");
    strMessage += strComponentKeyFilename;
    strMessage += _T("'");

    LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0, strMessage);

    return ERROR_SUCCESS;
}

UINT BOINCCABase::DisplayMessage(UINT uiPushButtonStyle, UINT uiIconStyle,
    const tstring& strMessage) {
    tstring uiLevel;
    const auto uiReturnValue = GetProperty(_T("UILevel"), uiLevel);
    if (uiReturnValue != ERROR_SUCCESS) {
        LogMessage(INSTALLMESSAGE_INFO, 0, 0, 0,
            _T("Failed to get UILevel property"));
        return ERROR_INSTALL_FAILURE;
    }
    if (uiLevel == _T("2")) {
        return ERROR_SUCCESS;
    }
    return ::MessageBox(nullptr, strMessage.data(), _T("Installer Message"),
        uiPushButtonStyle | uiIconStyle |
        MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION
    );
}

UINT BOINCCABase::LogProgress(const tstring& strProgress) {
    MsiRecordSetString(m_phActionDataRec, 2, strProgress.data());

    const auto uiReturnValue = MsiProcessMessage(m_hMSIHandle,
        INSTALLMESSAGE_ACTIONDATA, m_phActionDataRec);
    if (uiReturnValue == IDCANCEL)
        return ERROR_INSTALL_USEREXIT;

    // Give the UI a chance to refresh.
    Sleep(0);

    return ERROR_SUCCESS;
}

UINT BOINCCABase::LogMessage(UINT uiInstallMessageType,
    UINT uiPushButtonStyle, UINT uiIconStyle, UINT uiErrorCode,
    const tstring& strMessage) {
    if (uiInstallMessageType == INSTALLMESSAGE_INFO) {
        // Send informational message to the log file
        MsiRecordSetString(m_phLogInfoRec, 2, strMessage.data());
        MsiRecordSetInteger(m_phLogInfoRec, 3, uiErrorCode);

        return MsiProcessMessage(m_hMSIHandle,
            static_cast<INSTALLMESSAGE>(uiInstallMessageType), m_phLogInfoRec);
    }
    else {
        // Display a dialog and send error message to log file
        PMSIHANDLE phLogErrorRec = MsiCreateRecord(2);

        MsiRecordSetString(phLogErrorRec, 0, _T("[1]"));
        MsiRecordSetString(phLogErrorRec, 1, strMessage.data());

        // Return value to indicate which button is
        // pushed on message box
        return MsiProcessMessage(m_hMSIHandle,
            static_cast<INSTALLMESSAGE>(
                uiInstallMessageType | uiPushButtonStyle | uiIconStyle),
            phLogErrorRec);
    }
}

UINT BOINCCABase::RebootWhenFinished() {
    SetProperty(_T("RETURN_REBOOTREQUESTED"), _T("1"));
    return MsiSetMode(m_hMSIHandle, MSIRUNMODE_REBOOTATEND, TRUE);
}

bool BOINCCABase::localGroupExists(const tstring& groupName) {
    LOCALGROUP_INFO_0* info = nullptr;
    const auto rc = NetLocalGroupGetInfo(nullptr, groupName.c_str(), 0,
        reinterpret_cast<LPBYTE*>(&info));
    if (info) {
        NetApiBufferFree(info);
    }
    return rc == NERR_Success;
}

bool BOINCCABase::RecursiveSetPermissions(const tstring& path, PACL pACL) {
    const auto csPath = path + _T("\\");
    const auto csPathMask = csPath + _T("*.*");

    WIN32_FIND_DATA ffData;
    auto hFind = FindFirstFile(csPathMask.c_str(), &ffData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    while (hFind && FindNextFile(hFind, &ffData)) {
        if ((_tcscmp(ffData.cFileName, _T(".")) != 0) &&
            (_tcscmp(ffData.cFileName, _T("..")) != 0))
        {
            auto csFullPath = csPath + ffData.cFileName;

            SetNamedSecurityInfo(csFullPath.data(), SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION, nullptr, nullptr, pACL, nullptr);

            if (ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                RecursiveSetPermissions(csFullPath, pACL);
            }
        }
    }

    if (hFind) {
        FindClose(hFind);
    }

    return true;
}

void BOINCCABase::TerminateProcessEx(const tstring& strProcessName,
    bool bRecursive) {
    TerminateProcess::TerminateProcessEx(strProcessName, bRecursive);
}

DWORD BOINCCABase::ChangeAppIDAccessACL(const tstring& AppID,
    const tstring& Principal) {
    return DCOMPermissionConfig::ChangeAppIDAccessACL(AppID, Principal);
}

DWORD BOINCCABase::ChangeAppIDLaunchACL(const tstring& AppID,
    const tstring& Principal) {
    return DCOMPermissionConfig::ChangeAppIDLaunchACL(AppID, Principal);
}

bool BOINCCABase::GetAccountSid(const tstring& AccountName, PSID* Sid) {
    return LsaPrivs::GetAccountSid(AccountName, Sid);
}

bool BOINCCABase::GrantUserRight(PSID psidAccountSid,
    const tstring& pszUserRight, bool bEnable) {
    return LsaPrivs::GrantUserRight(psidAccountSid, pszUserRight, bEnable);
}

HRESULT BOINCCABase::CreateProcessWithExplorerIL(const tstring& szCmdLine) {
    return Launcher::CreateProcessWithExplorerIL(szCmdLine);
}
