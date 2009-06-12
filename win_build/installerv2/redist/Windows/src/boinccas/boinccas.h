// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef _BOINCCAS_H_
#define _BOINCCAS_H_

class BOINCCABase
{
public:

    BOINCCABase(
        MSIHANDLE          hMSIHandle, 
        const tstring      strActionName,
        const tstring      strProgressTitle
        );

    ~BOINCCABase();


    // Main Custom Action Execution Routine
    UINT Execute();


    // Overrides
    virtual UINT OnInitialize();
    virtual UINT OnCleanup();
    // Called when we are being executed from the InstallExecutionSequence
    virtual UINT OnInstall();
    virtual UINT OnRollback();
    virtual UINT OnCommit();
    // Called when we are being executed from the InstallUISequence
    virtual UINT OnExecution();


    // Registry Property Management
    UINT GetRegistryValue( 
        const tstring      strName, 
        tstring&           strValue,
        bool               bDisplayValue = true
        );

    UINT SetRegistryValue( 
        const tstring      strName, 
        const tstring      strValue,
        bool               bDisplayValue = true
        );

    // MSI Property Management
    UINT GetProperty( 
        const tstring      strPropertyName, 
        tstring&           strPropertyValue,
        bool               bDisplayValue = true
        );

    UINT SetProperty( 
        const tstring      strPropertyName, 
        const tstring      strPropertyValue,
        bool               bDisplayValue = true
        );

	// MSI Database Management
    UINT GetComponentKeyFilename(
        const tstring      strComponentName, 
        tstring&           strComponentKeyFilename
		);

    // MSI UI Dialog Management
    UINT DisplayMessage(
        const UINT         uiPushButtonStyle,       // push button style to use in message box
        const UINT         uiIconStyle,             // icon style to use in message box
        const tstring      strMessage               // message
        );

    // MSI Logging Management
    UINT LogProgress(
        const tstring      strProgress              // message to display
        );

    UINT LogMessage(
        const UINT         uiInstallMessageType,    // message type to send to Windows Installer
        const UINT         uiPushButtonStyle,       // push button sstyle to use in message box
        const UINT         uiIconStyle,             // icon style to use in message box
        const UINT         uiErrorNumber,           // number of error in Error table
        const UINT         uiErrorCode,             // the return value from an api
        const tstring      strMessage               // message
        );

    // Reboot machine when setup has completed.
    UINT RebootWhenFinished();

protected:
    MSIHANDLE   m_hMSIHandle;
	PMSIHANDLE  m_phActionStartRec;
	PMSIHANDLE  m_phActionDataRec;
    PMSIHANDLE  m_phProgressRec;
    PMSIHANDLE  m_phLogInfoRec;

    tstring     m_strActionName;
    tstring     m_strProgressTitle;
};

#endif

