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
Option Explicit

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

Const msiMessageStatusError        = -1 
Const msiMessageStatusNone         = 0 
Const msiMessageStatusOk           = 1 
Const msiMessageStatusCancel       = 2 
Const msiMessageStatusAbort        = 3 
Const msiMessageStatusRetry        = 4 
Const msiMessageStatusIgnore       = 5 
Const msiMessageStatusYes          = 6 
Const msiMessageStatusNo           = 7 

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
Function PopulateServiceAccount()
    On Error Resume Next

	Dim oNetwork
    Dim oRecord
	Dim strInitialServiceUsername
	Dim strInitialServiceDomain
	Dim strDomainUsername
	Dim strUserName
	Dim strUserDomain

	Set oNetwork = CreateObject("WScript.Network")
    Set oRecord = Installer.CreateRecord(2)

    strInitialServiceUsername = Property("SERVICE_USERNAME")
    If ( Len(strInitialServiceUsername) = 0 ) Then
        strUserName = oNetwork.UserName
        If ( Err.Number <> 0 ) Then
		    oRecord.IntegerData(1) = Err.Number
		    oRecord.StringData(2) = Err.Description
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
        
            PopulateServiceAccount = msiDoActionStatusFailure
            Exit Function
        End If
    End If
    
    strInitialServiceDomain = Property("SERVICE_DOMAIN")
    If ( Len(strInitialServiceDomain) = 0 ) Then
        strUserDomain = oNetwork.UserDomain
        If ( Err.Number <> 0 ) Then
		    oRecord.IntegerData(1) = Err.Number
		    oRecord.StringData(2) = Err.Description
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
        
            PopulateServiceAccount = msiDoActionStatusFailure
            Exit Function
        End If
    End If
        
    Property("SERVICE_DOMAINUSERNAME") = strUserDomain & "\" & strUserName

    Set oNetwork = Nothing

    PopulateServiceAccount = msiDoActionStatusSuccess
End Function


''
'' Validate the service username and domain name being passed
''   into the execution phase
''
Function ValidateServiceAccount()
    On Error Resume Next

    Dim oRecord
	Dim strInitialServiceUsername
	Dim strInitialServiceDomain
	Dim strInitialServiceDomainUsername
	Dim strInitialServicePassword

    Set oRecord = Installer.CreateRecord(2)

    strInitialServiceUsername = Property("SERVICE_USERNAME")
    strInitialServiceDomain = Property("SERVICE_DOMAIN")
    strInitialServicePassword = Property("SERVICE_PASSWORD")
    strInitialServiceDomainUsername = Property("SERVICE_DOMAINUSERNAME")
    
    If ( Property("SETUPTYPE") <> "ServiceGUI" ) Then 
	    If ( Len(strInitialServicePassword) = 0 ) Then
		    oRecord.IntegerData(1) = 25006
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
	
	        ValidateServiceAccount = msiDoActionStatusFailure
	        Exit Function
	    End If
	Else
	    If ( Len(strInitialServicePassword) = 0 ) Then
            If ( (Not(Instr(strInitialServiceDomainUsername, "LOCALSYSTEM"))) And (Not(Instr(strInitialServiceDomainUsername, "NetworkService"))) ) Then
			    oRecord.IntegerData(1) = 25006
		        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
		
		        ValidateServiceAccount = msiDoActionStatusFailure
		        Exit Function
            End If
	    End If
	End If


    If ( Len(strInitialServiceDomainUsername) = 0 ) Then
        If ( (Len(strInitialServiceUsername) = 0) Or (Len(strInitialServiceDomain) = 0) ) Then
		    oRecord.IntegerData(1) = 25007
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
	
	        ValidateServiceAccount = msiDoActionStatusFailure
	        Exit Function
        End If
    Else
        Property("SERVICE_DOMAIN") = Left( strInitialServiceDomainUsername, InStr( strInitialServiceDomainUsername, "\" ) - 1 )
        If ( Err.Number <> 0 ) Then
		    oRecord.IntegerData(1) = Err.Number
		    oRecord.StringData(2) = Err.Description
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
        
            ValidateServiceAccount = msiDoActionStatusFailure
            Exit Function
        End If

        Property("SERVICE_USERNAME") = Right( strInitialServiceDomainUsername, (Len( strInitialServiceDomainUsername ) - (InStr( strInitialServiceDomainUsername, "\" ))) )
        If ( Err.Number <> 0 ) Then
		    oRecord.IntegerData(1) = Err.Number
		    oRecord.StringData(2) = Err.Description
	        Message msiMessageTypeFatalExit Or vbCritical Or vbOKOnly, oRecord
        
            ValidateServiceAccount = msiDoActionStatusFailure
            Exit Function
        End If
    End If

    ValidateServiceAccount = msiDoActionStatusSuccess
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
        If ( strSetupType = "Single" Or strSetupType = "Service" ) Then
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


''
'' Verify with the user that it is okay to grant the 'Logon As a Service'
''   right for the selected user account
''
Function VerifyServiceExecutionRight()
    On Error Resume Next

    Dim oRecordRightPrompt
    Dim oRecordBDCPrompt
    Dim iFunctionReturnValue
    Dim strGrantValue

    Set oRecordRightPrompt = Installer.CreateRecord(2)
    Set oRecordBDCPrompt = Installer.CreateRecord(2)

    oRecordRightPrompt.IntegerData(1) = 25008
    oRecordBDCPrompt.IntegerData(1) = 25009

    strGrantValue = "0"

    iFunctionReturnValue = Message(msiMessageTypeUser Or vbExclamation Or vbYesNo, oRecordRightPrompt)
    If ( msiMessageStatusYes = iFunctionReturnValue ) Then
        strGrantValue = "1"    
    End If
    
    ''    
    '' Check if we are running on Windows NT 4.0 and we are either
    ''   a primary domain controller or backup domain controller
    ''
    If ( 400 = Property("VersionNT") ) Then
        If ( 2 = Property("MsiNTProductType") ) Then
		    Message msiMessageTypeUser Or vbExclamation Or vbOKOnly, oRecordBDCPrompt
    	 	strGrantValue = "0"    
        End If
    End If  

    Property("SERVICE_GRANTEXECUTIONRIGHT") = strGrantValue    
	VerifyServiceExecutionRight = msiDoActionStatusSuccess
End Function

