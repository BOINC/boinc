'' $Id$
''
'' The contents of this file are subject to the BOINC Public License
'' Version 1.0 (the "License"); you may not use this file except in
'' compliance with the License. You may obtain a copy of the License at
'' http://boinc.berkeley.edu/license_1.0.txt
'' 
'' Software distributed under the License is distributed on an "AS IS"
'' basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
'' License for the specific language governing rights and limitations
'' under the License. 
'' 
'' The Original Code is the Berkeley Open Infrastructure for Network Computing. 
'' 
'' The Initial Developer of the Original Code is the SETI@home project.
'' Portions created by the SETI@home project are Copyright (C) 2004
'' University of California at Berkeley. All Rights Reserved. 
'' 
'' Contributor(s):
''

Const msiMessageTypeFatalExit      = &H00000000
Const msiMessageTypeError          = &H01000000 
Const msiMessageTypeWarning        = &H02000000 
Const msiMessageTypeUser           = &H03000000 
Const msiMessageTypeInfo           = &H04000000 
Const msiMessageTypeFilesInUse     = &H05000000 
Const msiMessageTypeResolveSource  = &H06000000 
Const msiMessageTypeOutOfDiskSpace = &H07000000 
Const msiMessageTypeActionStart    = &H08000000 
Const msiMessageTypeActionData     = &H09000000
Const msiMessageTypeProgress       = &H0A000000
Const msiMessageTypeCommonData     = &H0B000000 

Const msiDoActionStatusNoAction    = 0
Const msiDoActionStatusSuccess     = 1
Const msiDoActionStatusUserExit    = 2
Const msiDoActionStatusFailure     = 3
Const msiDoActionStatusSuspend     = 4
Const msiDoActionStatusFinished    = 5
 

''
'' This function is called right after all the files have been copied to the
'' destination folder and before the BOINC service is started.  We basically
'' copy all the account files from the source path to the destination path.
''
Function CopyAccountFiles()
    On Error Resume Next

	Dim oFSO
    Dim oRecord
	Dim strInstallDir
	Dim strAccountsLocation
	Dim iReturnValue
	
	Set oFSO = CreateObject("Scripting.FileSystemObject")
    Set oRecord = Installer.CreateRecord(1)
	strInstallDir = Property("INSTALLDIR")
	strAccountsLocation = Property("ACCOUNTS_LOCATION")
    
	If (Not( IsEmpty(strAccountsLocation) )) And (Not( IsNull(strAccountsLocation) )) Then

 	   	'' Append the wildcard search to the end of the account file location
  	   	'' string
    	strAccountsLocation = strAccountsLocation + "\*.xml"

    	oFSO.CopyFile strAccountsLocation, strInstallDir
    	If (Err.Number = 0) Then
		    oRecord.IntegerData(1) = 25003
		    Message msiMessageTypeInfo, oRecord

    	    CopyAccountFiles = msiDoActionStatusSuccess
    	    Exit Function
    	Else
		    oRecord.IntegerData(1) = 25004
		    Message msiMessageTypeError, oRecord
		End If
	End If

    Set oFSO = Nothing
    Set oRecord = Nothing
    
	CopyAccountFiles = msiDoActionStatusFailure
End Function


''
'' Detect the previous version of BOINC if it was installed with the old installer
''
Function DetectOldInstaller()
    On Error Resume Next

	Dim oShell
    Dim oRecord
	Dim strUninstallValue

	Set oShell = CreateObject("WScript.Shell")
    Set oRecord = Installer.CreateRecord(1)

    strUninstallValue = oShell.RegRead("HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\BOINC\UninstallString")
	If (Err.Number <> 0) Then
        oRecord.IntegerData(1) = 25000
        Message msiMessageTypeInfo, oRecord

	    DetectOldInstaller = msiDoActionStatusSuccess
	    Exit Function
	Else
	    oRecord.IntegerData(1) = 25001
        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
	End If

    Set oShell = Nothing
    Set oRecord = Nothing

    DetectOldInstaller = msiDoActionStatusFailure 
End Function


''
'' Detect the username of the user executing setup, and populate
''   SERVICE_USERNAME if it is empty
''
Function DetectUsername()
    On Error Resume Next

	Dim oNetwork
	Dim strValue
	Dim strNewValue
	Dim strUserName
	Dim strUserDomain
	Dim strUserComputerName

	Set oNetwork = CreateObject("WScript.Network")

    strValue = Property("SERVICE_USERNAME")
    If ( Len(strValue) = 0 ) Then
        strUserName = oNetwork.UserName
        If ( Err.Number <> 0 ) Then
            DetectUsername = msiDoActionStatusFailure
            Exit Function
        End If
        
        strUserDomain = oNetwork.UserDomain
        If ( Err.Number <> 0 ) Then
            DetectUsername = msiDoActionStatusFailure
            Exit Function
        End If
        
        strComputerName = oNetwork.ComputerName
        If ( Err.Number <> 0 ) Then
            DetectUsername = msiDoActionStatusFailure
            Exit Function
        End If
                   
        If ( strUserDomain = strComputerName ) Then
            strNewValue = ".\" & strUserName
        Else
            strNewValue = strUserDomain & "\" & strUserName
        End If
                      
        Property("SERVICE_USERNAME") = strNewValue
    End If

    Set oNetwork = Nothing

    DetectUsername = msiDoActionStatusSuccess
End Function


''
'' Launch the BOINC Readme when requested to do so
''
Function LaunchReadme()
    On Error Resume Next

	Dim oShell
    Dim oRecord
    Dim strFileToLaunch

	Set oShell = CreateObject("WScript.Shell")
    Set oRecord = Installer.CreateRecord(0)

    oRecord.StringData(0) = Property("READMEFILETOLAUNCHATEND")
    strFileToLaunch = "notepad.exe " & FormatRecord(oRecord)

    oShell.Run(strFileToLaunch)

    Set oShell = Nothing
    Set oRecord = Nothing

	LaunchReadme = msiDoActionStatusSuccess
End Function


''
'' If we are the 'Shared' SETUPTYPE then we must change ALLUSERS = 1
''
Function ValidateSetupType()
    On Error Resume Next

    Dim strSetupType
    Dim oRecord

    Set oRecord = Installer.CreateRecord(1)

    strSetupType = Property("SETUPTYPE")
    If (Len(strSetupType) <> 0) Then
        If ( strSetupType = "Single" ) Then
            If (Property("ALLUSERS") <> "") Then
			    oRecord.IntegerData(1) = 25002
			    Message msiMessageTypeFatalExit, oRecord

		        ValidateSetupType = msiDoActionStatusFailure
				Exit Function
            End If
        Else
            If (Property("ALLUSERS") <> "1") Then
			    oRecord.IntegerData(1) = 25005
			    Message msiMessageTypeFatalExit, oRecord

		        ValidateSetupType = msiDoActionStatusFailure
				Exit Function
            End If
        End If
    End If
                                     
    Set oRecord = Nothing

	ValidateSetupType = msiDoActionStatusSuccess
End Function

