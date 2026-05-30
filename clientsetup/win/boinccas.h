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
    explicit BOINCCABase(MSIHANDLE hMSIHandle, tstring strActionName,
        tstring strProgressTitle);
    virtual ~BOINCCABase() = default;

    UINT Execute();
protected:
    virtual UINT OnExecution() = 0;

    bool IsUpgrading();
    UINT GetRegistryValue(const tstring& strName, tstring& strValue,
        bool bDisplayValue = true);
    UINT SetRegistryValue(const tstring& strName, const tstring& strValue,
        bool bDisplayValue = true);
    UINT GetProperty(const tstring& strPropertyName,
        tstring& strPropertyValue, bool bDisplayValue = true);
    UINT SetProperty(const tstring& strPropertyName,
        const tstring& strPropertyValue, bool bDisplayValue = true);
    UINT GetComponentKeyFilename(const tstring& strComponentName,
        tstring&strComponentKeyFilename);
    UINT DisplayMessage(UINT uiPushButtonStyle, UINT uiIconStyle,
        const tstring& strMessage);
    UINT LogProgress(const tstring& strProgress);
    UINT LogMessage(UINT uiInstallMessageType, UINT uiPushButtonStyle,
        UINT uiIconStyle, UINT uiErrorCode, const tstring& strMessage);
    UINT RebootWhenFinished();
    bool localGroupExists(const tstring& groupName);
    bool RecursiveSetPermissions(const tstring& path, PACL pACL);
    void TerminateProcessEx(const tstring& strProcessName,
        bool bRecursive = true);
    DWORD ChangeAppIDAccessACL(const tstring& AppID,
        const tstring& Principal);
    DWORD ChangeAppIDLaunchACL(const tstring& AppID,
        const tstring& Principal);
    bool GetAccountSid(const tstring& AccountName, PSID* Sid);
    bool GrantUserRight(PSID psidAccountSid,
        const tstring& pszUserRight, bool bEnable);
    HRESULT CreateProcessWithExplorerIL(const tstring& szCmdLine);
private:
    UINT OnInitialize();

    MSIHANDLE m_hMSIHandle;
    PMSIHANDLE m_phActionDataRec = 0;
    PMSIHANDLE m_phLogInfoRec = 0;

    tstring m_strActionName;
    tstring m_strProgressTitle;
};
