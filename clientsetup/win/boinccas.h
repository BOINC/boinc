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

#pragma once

class BOINCCABase
{
public:
    explicit BOINCCABase(
        MSIHANDLE hMSIHandle,
        tstring_view strActionName,
        tstring_view strProgressTitle
    );
    virtual ~BOINCCABase() = default;

    UINT Execute();
protected:
    virtual UINT OnExecution() = 0;

    bool IsUpgrading();
    UINT GetRegistryValue(
        tstring_view strName, tstring& strValue, bool bDisplayValue = true
    );
    UINT SetRegistryValue(
        tstring_view strName, tstring_view strValue, bool bDisplayValue = true
    );
    UINT GetProperty(
        tstring_view strName, tstring& strValue, bool bDisplayValue = true
    );
    UINT SetProperty(
        tstring_view strName, tstring_view strValue, bool bDisplayValue = true
    );
    UINT GetComponentKeyFilename(
        tstring_view strComponentName, tstring&strComponentKeyFilename
    );
    UINT DisplayMessage(
        UINT uiPushButtonStyle, UINT uiIconStyle, tstring_view strMessage
    );
    UINT LogProgress(tstring_view strProgress);
    UINT LogMessage(
        UINT uiInstallMessageType, UINT uiPushButtonStyle, UINT uiIconStyle,
        UINT uiErrorCode, tstring_view strMessage
    );
    UINT RebootWhenFinished();
    bool localGroupExists(tstring_view groupName);
    bool RecursiveSetPermissions(tstring_view path, PACL pACL);
    void TerminateProcessEx(
        tstring_view strProcessName, bool bRecursive = true
    );
    DWORD ChangeAppIDAccessACL(tstring_view AppID, tstring_view Principal);
    DWORD ChangeAppIDLaunchACL(tstring_view AppID, tstring_view Principal);
    bool GetAccountSid(tstring_view AccountName, PSID* Sid);
    bool GrantUserRight(
        PSID psidAccountSid, tstring_view pszUserRight, bool bEnable
    );
    HRESULT CreateProcessWithExplorerIL(tstring_view szCmdLine);
private:
    UINT OnInitialize();

    MSIHANDLE m_hMSIHandle;
    PMSIHANDLE m_phActionDataRec = 0;
    PMSIHANDLE m_phLogInfoRec = 0;

    tstring_view m_strActionName;
    tstring_view m_strProgressTitle;
};
