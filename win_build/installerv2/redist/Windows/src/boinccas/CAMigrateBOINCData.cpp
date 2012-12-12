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

#include "stdafx.h"
#include "boinccas.h"
#include "CAMigrateBOINCData.h"
#include "dirops.h"

#define CUSTOMACTION_NAME               _T("CAMigrateBOINCData")
#define CUSTOMACTION_PROGRESSTITLE      _T("Migrate application data to the data directory.")


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCData::CAMigrateBOINCData(MSIHANDLE hMSIHandle) :
    BOINCCABase(hMSIHandle, CUSTOMACTION_NAME, CUSTOMACTION_PROGRESSTITLE)
{}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
CAMigrateBOINCData::~CAMigrateBOINCData()
{
    BOINCCABase::~BOINCCABase();
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CAMigrateBOINCData::OnInstall()
{
    tstring     strCustomActionData;
    tstring     strMigratingDirectory;
    tstring     strInstallDirectory;
    tstring     strDataDirectory;
    tstring     strMessage;
    ULONGLONG   ullFileSize = 0;
    ULONGLONG   ullDirectorySize = 0;
    ULONGLONG   ullBytesTransfered = 0;
    UINT        uiReturnValue = -1;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnInstall -- Function Begin")
    );

    // CustomActionData is a pipe seperated string which contains whether
    //   this interation of the installer is migrating the BOINC data, the
    //   installer directory, and the data directory.
    //
    // Ex: <MigrationStatus>|<InstallDirectory>|<DataDrectory>
    //
    uiReturnValue = GetProperty( _T("CustomActionData"), strCustomActionData );
    if ( uiReturnValue ) return uiReturnValue;

    strMigratingDirectory = 
        strCustomActionData.substr(0, strCustomActionData.find(_T("|")));

    strMessage = _T("CAMigrateBOINCData::OnInstall -- strMigratingDirectory: '");
    strMessage += strMigratingDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    strInstallDirectory = 
        strCustomActionData.substr(
            strCustomActionData.find(_T("|")) + 1,
            strCustomActionData.find(_T("|"), (strCustomActionData.find(_T("|")) + 1)) - 5
        );

    strMessage = _T("CAMigrateBOINCData::OnInstall -- strInstallDirectory: '");
    strMessage += strInstallDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    strDataDirectory = 
        strCustomActionData.substr(strCustomActionData.rfind(_T("|")) + 1, strCustomActionData.length() - strCustomActionData.rfind(_T("|")) - 1);

    strMessage = _T("CAMigrateBOINCData::OnInstall -- strDataDirectory: '");
    strMessage += strDataDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );


    // Are we migrating data?
    if ( _T("TRUE") == strMigratingDirectory )
    {
        // Determine how we should setup the progress bar.
        GetFileDirectorySizes( strInstallDirectory, ullFileSize, ullDirectorySize );

        // Reset the progress bar
        MsiRecordSetInteger(m_phProgressRec, 1, 0);
        MsiRecordSetInteger(m_phProgressRec, 2, (INT)((ullDirectorySize/1024)/1024));
        MsiRecordSetInteger(m_phProgressRec, 3, 0);
        MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);

        // Tell the installer to use explicit progress messages.
        MsiRecordSetInteger(m_phProgressRec, 1, 1);
        MsiRecordSetInteger(m_phProgressRec, 2, 1);
        MsiRecordSetInteger(m_phProgressRec, 3, 0);
        MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);

        // Migrate the data files
        if (!MoveFiles( strInstallDirectory, strDataDirectory, ullBytesTransfered ))
        {
            LogMessage(
                INSTALLMESSAGE_FATALEXIT,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Failed to migrate BOINC data files to the data directory.")
                );
            return ERROR_INSTALL_FAILURE;
        }
    }

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnInstall -- Function End")
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CAMigrateBOINCData::OnRollback()
{
    tstring     strCustomActionData;
    tstring     strMigratingDirectory;
    tstring     strInstallDirectory;
    tstring     strDataDirectory;
    tstring     strMessage;
    ULONGLONG   ullFileSize = 0;
    ULONGLONG   ullDirectorySize = 0;
    ULONGLONG   ullBytesTransfered = 0;
    UINT        uiReturnValue = -1;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnRollback -- Function Begin")
    );

    // CustomActionData is a pipe seperated string which contains whether
    //   this interation of the installer is migrating the BOINC data, the
    //   installer directory, and the data directory.
    //
    // Ex: <MigrationStatus>|<InstallDirectory>|<DataDrectory>
    //
    uiReturnValue = GetProperty( _T("CustomActionData"), strCustomActionData );
    if ( uiReturnValue ) return uiReturnValue;

    strMigratingDirectory = 
        strCustomActionData.substr(0, strCustomActionData.find(_T("|")));

    strMessage = _T("CAMigrateBOINCData::OnRollback -- strMigratingDirectory: '");
    strMessage += strMigratingDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    strInstallDirectory = 
        strCustomActionData.substr(
            strCustomActionData.find(_T("|")) + 1,
            strCustomActionData.find(_T("|"), (strCustomActionData.find(_T("|")) + 1)) - 5
        );

    strMessage = _T("CAMigrateBOINCData::OnRollback -- strInstallDirectory: '");
    strMessage += strInstallDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    strDataDirectory = 
        strCustomActionData.substr(strCustomActionData.rfind(_T("|")) + 1, strCustomActionData.length() - strCustomActionData.rfind(_T("|")) - 1);

    strMessage = _T("CAMigrateBOINCData::OnRollback -- strDataDirectory: '");
    strMessage += strDataDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );


    // Are we migrating data?
    if ( _T("TRUE") == strMigratingDirectory )
    {
        // Determine how we should setup the progress bar.
        GetFileDirectorySizes( strInstallDirectory, ullFileSize, ullDirectorySize );

        // Reset the progress bar
        MsiRecordSetInteger(m_phProgressRec, 1, 0);
        MsiRecordSetInteger(m_phProgressRec, 2, (INT)((ullDirectorySize/1024)/1024));
        MsiRecordSetInteger(m_phProgressRec, 3, 0);
        MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);

        // Tell the installer to use explicit progress messages.
        MsiRecordSetInteger(m_phProgressRec, 1, 1);
        MsiRecordSetInteger(m_phProgressRec, 2, 1);
        MsiRecordSetInteger(m_phProgressRec, 3, 0);
        MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);

        // Migrate the data files back to the original location
        MoveFiles( strDataDirectory, strInstallDirectory, ullBytesTransfered );
    }

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnRollback -- Function End")
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT CAMigrateBOINCData::OnExecution()
{
    tstring      strCustomActionData;
    tstring      strCurrentInstallDirectory;
    tstring      strFutureInstallDirectory;
    tstring      strCurrentDataDirectory;
    tstring      strFutureDataDirectory;
    tstring      strMigration;
    tstring      strMigrationVersion;
    tstring      strMigrationDirectory;
    tstring      strDestinationClientStateFile;
    tstring      strRemove;
    tstring      strProductVersion;
    tstring      strWindowsDirectory;
    tstring      strWindowsSystemDirectory;
    tstring      strProgramFilesDirectory;
    tstring      strSystemDrive;
    tstring      strVersionWindows64;
    struct _stat buf;
    ULONGLONG    ullFileSize = 0;
    ULONGLONG    ullDirectorySize = 0;
    ULONGLONG    ullFreeDiskSpace = 0;
    UINT         uiReturnValue = -1;

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnExecution -- Function Begin")
    );


    GetRegistryValue( _T("INSTALLDIR"), strCurrentInstallDirectory );

    uiReturnValue = GetProperty( _T("INSTALLDIR"), strFutureInstallDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("DATADIR"), strFutureDataDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetRegistryValue( _T("MIGRATION"), strMigration );
    uiReturnValue = GetRegistryValue( _T("MIGRATIONVERSION"), strMigrationVersion );
    uiReturnValue = GetRegistryValue( _T("MIGRATIONDIR"), strMigrationDirectory );

    uiReturnValue = GetProperty( _T("REMOVE"), strRemove );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("ProductVersion"), strProductVersion );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("VersionNT64"), strVersionWindows64 );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("WindowsFolder"), strWindowsDirectory );
    if ( uiReturnValue ) return uiReturnValue;

    uiReturnValue = GetProperty( _T("WindowsVolume"), strSystemDrive );
    if ( uiReturnValue ) return uiReturnValue;

    if (strVersionWindows64.length() > 0)
    {
        uiReturnValue = GetProperty( _T("System64Folder"), strWindowsSystemDirectory );
        if ( uiReturnValue ) return uiReturnValue;

        uiReturnValue = GetProperty( _T("ProgramFiles64Folder"), strProgramFilesDirectory );
        if ( uiReturnValue ) return uiReturnValue;
    }
    else
    {
        uiReturnValue = GetProperty( _T("SystemFolder"), strWindowsSystemDirectory );
        if ( uiReturnValue ) return uiReturnValue;

        uiReturnValue = GetProperty( _T("ProgramFilesFolder"), strProgramFilesDirectory );
        if ( uiReturnValue ) return uiReturnValue;
    }


    // If the REMOVE property is specified, then we are uninstalling BOINC
    if (strRemove.length())
    {
        // If we successfully migrated, and the user is now uninstalling, they
        //   are probably going back to a 6.x client, so don't migrate the
        //   data back to the 5.x location.
        //
        strMigration = _T("FALSE");
        strCustomActionData += strMigration + _T("|");
        strCustomActionData += strMigrationDirectory + _T("|");
        strCustomActionData += strFutureDataDirectory;
    }
    else
    {
        strDestinationClientStateFile = strFutureDataDirectory + _T("\\client_state.xml");

        // Perform some basic sanity tests to see if we need to migrate
        //   anything.
        BOOL bClientStateExists =
            (BOOL)(0 == _stat(strDestinationClientStateFile.c_str(), &buf));
        BOOL bInstallDataSameDirectory = 
            (BOOL)(strFutureInstallDirectory == strFutureDataDirectory);
        BOOL bDataDirExistsWithinInstallDir = 
            (BOOL)(tstring::npos != strFutureDataDirectory.find(strFutureInstallDirectory));
        BOOL bInstallDirWindowsDirSame = 
            (BOOL)(strFutureInstallDirectory == strWindowsDirectory);
        BOOL bDataDirWindowsDirSame = 
            (BOOL)(strFutureDataDirectory == strWindowsDirectory);
        BOOL bInstallDirSystemDriveSame = 
            (BOOL)(strFutureInstallDirectory == strSystemDrive);
        BOOL bDataDirSystemDriveSame = 
            (BOOL)(strFutureDataDirectory == strSystemDrive);
        BOOL bInstallDirWindowsSystemDirSame = 
            (BOOL)(strFutureInstallDirectory == strWindowsSystemDirectory);
        BOOL bDataDirWindowsSystemDirSame = 
            (BOOL)(strFutureDataDirectory == strWindowsSystemDirectory);
        BOOL bInstallDirProgramFilesDirSame = 
            (BOOL)(strFutureInstallDirectory == strProgramFilesDirectory);
        BOOL bDataDirProgramFilesDirSame = 
            (BOOL)(strFutureDataDirectory == strProgramFilesDirectory);

        if      ( bClientStateExists )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data files already exists, skipping migration.")
            );
        }
        else if ( bInstallDataSameDirectory ) 
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Install directory and data directory are the same, skipping migration.")
            );
        }
        else if ( bDataDirExistsWithinInstallDir )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data drectory exists within the install directory, skipping migration.")
            );
        }
        else if ( bInstallDirWindowsDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Install directory is the same as the Windows directory, skipping migration.")
            );
        }
        else if ( bDataDirWindowsDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data directory is the same as the Windows directory, skipping migration.")
            );
        }
        else if ( bInstallDirSystemDriveSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Install directory is the same as the system drive, skipping migration.")
            );
        }
        else if ( bDataDirSystemDriveSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data directory is the same as the system drive, skipping migration.")
            );
        }
        else if ( bInstallDirWindowsSystemDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Install directory is the same as the Windows system directory, skipping migration.")
            );
        }
        else if ( bDataDirWindowsSystemDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data directory is the same as the Windows system directory, skipping migration.")
            );
        }
        else if ( bInstallDirProgramFilesDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Install directory is the same as the program files directory, skipping migration.")
            );
        }
        else if ( bDataDirProgramFilesDirSame )
        {
            strMigration = _T("FALSE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data directory is the same as the program files directory, skipping migration.")
            );
        }
        else
        {
            strMigration = _T("TRUE");
            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                _T("Data files do NOT exist, performing migration.")
            );

            strMigrationDirectory = strCurrentInstallDirectory + _T("\\client_state.xml");
            if (0 == _stat(strMigrationDirectory.c_str(), &buf)) {
                strMigrationDirectory = strCurrentInstallDirectory;
            } else {
                strMigrationDirectory = strFutureInstallDirectory;
            }

            if ( GetFileDirectorySizes( strMigrationDirectory, ullFileSize, ullDirectorySize ) )
            {
                // The total amount of disk space required depends on whether or not
                //   the files in the original location are on the same volume as the
                //   destination. So do a quick check to see if we have enough disk
                //   space.
                if (!GetFreeDiskSpace(strFutureDataDirectory, ullFreeDiskSpace))
                {
                    // If the destination directory doesn't exist, try the parent
                    //   directory
                    tstring strBuffer = tstring(strFutureDataDirectory + _T("..\\"));
                    if (!GetFreeDiskSpace(strBuffer, ullFreeDiskSpace))
                    {
                        // If the parent directory doesn't exist, just choose
                        //   the default volume. Something is better than nothing
                        GetFreeDiskSpace(tstring(""), ullFreeDiskSpace);
                    }
                }


                // Are we on the same volume?
                if (strMigrationDirectory.substr(0, 2) == strFutureDataDirectory.substr(0, 2))
                {
                    // We only need the amount of free space as the largest file
                    //   that is going to be transfered.
                    if (ullFileSize > ullFreeDiskSpace)
                    {
                        LogMessage(
                            INSTALLMESSAGE_INFO,
                            NULL, 
                            NULL,
                            NULL,
                            NULL,
                            _T("Not enough free disk space is available to migrate BOINC's data files to\n"
                               "the new data directory. Please free up some disk space or migrate the files\n"
                               "manually. (ullFileSize > ullFreeDiskSpace)")
                        );
                        return ERROR_INSTALL_FAILURE;
                    }
                }
                else
                {
                    // We only need the amount of free space of the directory
                    //   that is going to be transfered.
                    if (ullDirectorySize > ullFreeDiskSpace)
                    {
                        LogMessage(
                            INSTALLMESSAGE_INFO,
                            NULL, 
                            NULL,
                            NULL,
                            NULL,
                            _T("Not enough free disk space is available to migrate BOINC's data files to\n"
                               "the new data directory. Please free up some disk space or migrate the files\n"
                               "manually. (ullDirectorySize > ullFreeDiskSpace)")
                        );
                        return ERROR_INSTALL_FAILURE;
                    }
                }
            }
        }

        // Contruct a '|' delimited string to pass along to the install script
        //   and rollback script parts of this custom action.
        strCustomActionData += strMigration + _T("|");
        strCustomActionData += strMigrationDirectory + _T("|");
        strCustomActionData += strFutureDataDirectory;
    }

    SetRegistryValue( _T("MIGRATION"), strMigration );
    SetRegistryValue( _T("MIGRATIONDIR"), strMigrationDirectory );
    SetProperty( _T("CAMigrateBOINCDataInstall"), strCustomActionData );
    SetProperty( _T("CAMigrateBOINCDataRollback"), strCustomActionData );

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        _T("CAMigrateBOINCData::OnExecution -- Function End")
    );

    return ERROR_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
BOOL CAMigrateBOINCData::GetFileDirectorySizes( tstring strDirectory, ULONGLONG& ullFileSize, ULONGLONG& ullDirectorySize )
{
    WIN32_FIND_DATA ffData;
    HANDLE          hFind;
    tstring         csPathMask;
    tstring         csFullPath;
    tstring         csNewFullPath;
    
    if ( _T("\\") != strDirectory.substr(strDirectory.length() - 1, 1) )
    {
        strDirectory += _T("\\");
    }
    csPathMask = strDirectory + _T("*.*");

    hFind = FindFirstFile(csPathMask.c_str(), &ffData);

    if (hFind == INVALID_HANDLE_VALUE){
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("CAMigrateBOINCData::GetFileDirectorySizes -- Invalid handle")
        );
        return FALSE;
    }
    
    // Calculating Sizes
    while (hFind && FindNextFile(hFind, &ffData)) 
    {
        if( !(ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
        {
            csFullPath  = strDirectory;
            csFullPath += ffData.cFileName;

            // Add current file size to the overall directory size
            ullDirectorySize += ((ffData.nFileSizeHigh * MAXDWORD) + ffData.nFileSizeLow);

            // If this file size is bigger than the one we know about, store it
            //   for later use.
            if (ullFileSize < ((ffData.nFileSizeHigh * MAXDWORD) + ffData.nFileSizeLow)) {
                ullFileSize = ((ffData.nFileSizeHigh * MAXDWORD) + ffData.nFileSizeLow);
            }
        }
        else // it is a directory
        { 
            csNewFullPath  = strDirectory;
            csNewFullPath += ffData.cFileName;
            csNewFullPath += _T("\\");

            if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
                (_tcscmp(ffData.cFileName, _T("..")) != 0) ) 
            {
                GetFileDirectorySizes(csNewFullPath, ullFileSize, ullDirectorySize);
            }
        }
    }

    FindClose(hFind);
    return TRUE;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
typedef BOOL (CALLBACK* MyGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
BOOL CAMigrateBOINCData::GetFreeDiskSpace( tstring strDirectory, ULONGLONG& ullFreeSpace )
{
    ULARGE_INTEGER          TotalNumberOfFreeBytes;
    ULARGE_INTEGER          TotalNumberOfBytes;
    ULARGE_INTEGER          TotalNumberOfBytesFreeToCaller;
    DWORD                   dwSectPerClust = NULL;
    DWORD                   dwBytesPerSect = NULL;
    DWORD                   dwFreeClusters = NULL;
    DWORD                   dwTotalClusters = NULL;
    BOOL                    bReturnValue = FALSE;
    MyGetDiskFreeSpaceEx    pGetDiskFreeSpaceEx = NULL;
    TCHAR                   szMessage[2048];

#ifdef _UNICODE
    pGetDiskFreeSpaceEx = (MyGetDiskFreeSpaceEx)GetProcAddress(
        GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExW"
    );
#else
    pGetDiskFreeSpaceEx = (MyGetDiskFreeSpaceEx)GetProcAddress(
        GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA"
    );
#endif

    _sntprintf(
        szMessage, 
        sizeof(szMessage),
        _T("GetFreeDiskSpace Directory Location: '%s'"),
        strDirectory.c_str()
    );
    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        szMessage
    );

    if (pGetDiskFreeSpaceEx) {

        if (0 == strDirectory.length())
        {
            bReturnValue = pGetDiskFreeSpaceEx(
                NULL,
                &TotalNumberOfBytesFreeToCaller,
                &TotalNumberOfBytes,
                &TotalNumberOfFreeBytes
            );
        }
        else
        {
            bReturnValue = pGetDiskFreeSpaceEx(
                strDirectory.c_str(),
                &TotalNumberOfBytesFreeToCaller,
                &TotalNumberOfBytes,
                &TotalNumberOfFreeBytes
            );
        }

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("GetDiskFreeSpaceEx Return Value: '%d'"),
            bReturnValue
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("GetDiskFreeSpaceEx GetLastError: '%d'"),
            GetLastError()
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("TotalNumberOfFreeBytes: '%I64u'"),
            TotalNumberOfFreeBytes.QuadPart
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        ullFreeSpace = TotalNumberOfFreeBytes.QuadPart;

    }
    else
    {

        if (0 == strDirectory.length())
        {
            bReturnValue = GetDiskFreeSpace(
                NULL,
                &dwSectPerClust,
                &dwBytesPerSect,
                &dwFreeClusters,
                &dwTotalClusters
            );
        }
        else
        {
            bReturnValue = GetDiskFreeSpace(
                strDirectory.c_str(),
                &dwSectPerClust,
                &dwBytesPerSect,
                &dwFreeClusters,
                &dwTotalClusters
            );
        }

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("GetDiskFreeSpace Return Value: '%d'"),
            bReturnValue
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        _sntprintf(
            szMessage, 
            sizeof(szMessage),
            _T("GetDiskFreeSpace GetLastError: '%d'"),
            GetLastError()
        );
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            szMessage
        );

        ullFreeSpace = dwFreeClusters * dwSectPerClust * dwBytesPerSect;
    }

    return bReturnValue;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
BOOL CAMigrateBOINCData::MoveFiles( tstring strSourceDirectory, tstring strDestinationDirectory, ULONGLONG& ullBytesTransfered )
{
    BOOL bRet = TRUE;
    WIN32_FIND_DATA ffData;
    HANDLE hFind;
    tstring csPathMask;
    tstring csFullPath;
    tstring csNewFullPath;
    tstring strMessage;

    strMessage  = _T("CAMigrateBOINCData::MoveFiles -- Directory: '");
    strMessage += strSourceDirectory;
    strMessage += _T("'");

    LogMessage(
        INSTALLMESSAGE_INFO,
        NULL, 
        NULL,
        NULL,
        NULL,
        strMessage.c_str()
    );

    // Create the destination cirectory if needed.
    //
    CreateDirectory(strDestinationDirectory.c_str(), NULL);
    
    if ( _T("\\") != strSourceDirectory.substr(strSourceDirectory.length() - 1, 1) )
    {
        strSourceDirectory       += _T("\\");
    }
    if ( _T("\\") != strDestinationDirectory.substr(strDestinationDirectory.length() - 1, 1) )
    {
        strDestinationDirectory  += _T("\\");
    }
    csPathMask                = strSourceDirectory + _T("*.*");
        
    hFind = FindFirstFile(csPathMask.c_str(), &ffData);

    if (hFind == INVALID_HANDLE_VALUE){
        LogMessage(
            INSTALLMESSAGE_INFO,
            NULL, 
            NULL,
            NULL,
            NULL,
            _T("CAMigrateBOINCData::MoveFiles -- Invalid handle")
        );
        return FALSE;
    }
    

    // Copying all the files
    while (hFind && FindNextFile(hFind, &ffData)) 
    {
        csFullPath    = strSourceDirectory      + ffData.cFileName;
        csNewFullPath = strDestinationDirectory + ffData.cFileName;

        RemoveReadOnly(csFullPath);
        RemoveReadOnly(csNewFullPath);

        if( !(ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
        {
            strMessage  = _T("CAMigrateBOINCData::MoveFiles -- Copy File: '");
            strMessage += csFullPath.c_str();
            strMessage += _T("' to '");
            strMessage += csNewFullPath.c_str();
            strMessage += _T("'");

            LogMessage(
                INSTALLMESSAGE_INFO,
                NULL, 
                NULL,
                NULL,
                NULL,
                strMessage.c_str()
            );

            if( !CopyFile(csFullPath.c_str(), csNewFullPath.c_str(), FALSE) ) 
            {
                LogMessage(
                    INSTALLMESSAGE_INFO,
                    NULL, 
                    NULL,
                    NULL,
                    GetLastError(),
                    _T("CAMigrateBOINCData::MoveFiles -- Failed to copy original file")
                );
                bRet = FALSE;
            }
            else
            {
                ullBytesTransfered += ((ffData.nFileSizeHigh * MAXDWORD) + ffData.nFileSizeLow);

                // Specify that an update of the progress bar's position in
                //   this case means to move it forward by one increment.
                MsiRecordSetInteger(m_phProgressRec, 1, 2);
                MsiRecordSetInteger(m_phProgressRec, 2, (INT)((ullBytesTransfered/1024)/1024));
                MsiRecordSetInteger(m_phProgressRec, 3, 0);
                MsiProcessMessage(m_hMSIHandle, INSTALLMESSAGE_PROGRESS, m_phProgressRec);
                Sleep(0);

                // Delete the original file when it has been successfully
                //   copied
                if( !DeleteFile(csFullPath.c_str()) ) 
                {
                    LogMessage(
                        INSTALLMESSAGE_INFO,
                        NULL, 
                        NULL,
                        NULL,
                        GetLastError(),
                        _T("CAMigrateBOINCData::MoveFiles -- Failed to delete original file")
                    );
                }
            }
        }
        else // it is a directory -> Copying recursivly
        { 
            if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
                (_tcscmp(ffData.cFileName, _T("..")) != 0) && 
                (!IsDirectoryExcluded(tstring(ffData.cFileName)))) 
            {
                if( !MoveFiles(csFullPath, csNewFullPath, ullBytesTransfered) )
                {
                    LogMessage(
                        INSTALLMESSAGE_INFO,
                        NULL, 
                        NULL,
                        NULL,
                        NULL,
                        _T("CAMigrateBOINCData::MoveFiles -- Failed to copy drectory")
                    );
                    LogMessage(
                        INSTALLMESSAGE_INFO,
                        NULL, 
                        NULL,
                        NULL,
                        NULL,
                        csNewFullPath.c_str()
                    );
                    bRet = FALSE;
                }
            }
        }
    }

    FindClose(hFind);

    RemoveReadOnly(strSourceDirectory);
	RemoveDirectory(strSourceDirectory.c_str());

    return bRet;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
BOOL CAMigrateBOINCData::IsDirectoryExcluded( tstring strTargetDirectory )
{
    DowncaseString(strTargetDirectory);
    if (StartsWith(strTargetDirectory, tstring("locale"))) return TRUE;
    if (StartsWith(strTargetDirectory, tstring("skins"))) return TRUE;
    return FALSE;
}


/////////////////////////////////////////////////////////////////////
// 
// Function:    MigrateBOINCData
//
// Description: 
//
/////////////////////////////////////////////////////////////////////
UINT __stdcall MigrateBOINCData(MSIHANDLE hInstall)
{
    UINT uiReturnValue = 0;

    CAMigrateBOINCData* pCA = new CAMigrateBOINCData(hInstall);
    uiReturnValue = pCA->Execute();
    delete pCA;

    return uiReturnValue;
}


const char *BOINC_RCSID_8dca879ada="$Id: CAMigrateBOINCData.cpp 11773 2007-01-05 08:49:02Z rwalton $";
