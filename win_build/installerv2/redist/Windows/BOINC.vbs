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


''
'' Detect is a previous version of BOINC was installed with the old installer
''
Function IsOldInstallDeteted()

	Dim iReturnValue
	Dim oShell
	Dim strUninstallValue

    On Error Resume Next

	Set oShell = CreateObject("WScript.Shell")
    strUninstallValue = oShell.RegRead("HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\BOINC\UninstallString")
	If Err.Number <> 0 Then
	    iReturnValue = IDOK
	Else
	    MsgBox("Setup has detected an older version of BOINC, please uninstall it to install this version")
	    iReturnValue = IDABORT 
	End If

	IsOldInstallDeteted = iReturnValue
    On Error GoTo 0
End Function

''
'' This function is called right after all the files have been copied to the
'' destination folder and before the BOINC service is started.  We basically
'' copy all the account files from the source path to the destination path.
''
Function Accounts_CopyFiles()

	Dim oFSO
	Dim strInstallDir
	Dim strAccountsLocation
	Dim iReturnValue
	
    On Error Resume Next

	Set oFSO = CreateObject("Scripting.FileSystemObject")
	strInstallDir = Property("INSTALLDIR")
	strAccountsLocation = Property("ACCOUNTS_LOCATION")
    
	If (Not( IsEmpty(strAccountsLocation) )) And (Not( IsNull(strAccountsLocation) )) Then

 	   	'' Append the wildcard search to the end of the account file location
  	   	'' string
    	strAccountsLocation = strAccountsLocation + "\*.xml"

    	oFSO.CopyFile strAccountsLocation, strInstallDir
    	If ErrHandler("Copy Account Files Failed ") Then
    	    iReturnValue = IDCANCEL
    	Else
    	    iReturnValue = IDOK
    	End If

	End If

	Accounts_CopyFiles = iReturnValue
    On Error GoTo 0
End Function


Function ErrHandler(what)
    Dim bReturnValue

    If Err.Number > 0 Then
        MsgBox( what & " Error " & Err.Number & ": " & Err.Description )
        Err.Clear
        bReturnValue = True
    Else
        bReturnValue = False
    End If

    ErrHandler = bReturnValue
End Function


Sub Print(Str)
    'strip when not debugging
    MsgBox(Str)
End Sub
