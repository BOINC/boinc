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
'' Portions created by the SETI@home project are Copyright (C) 2002
'' University of California at Berkeley. All Rights Reserved. 
'' 
'' Contributor(s):
''

''
'' This script is called right after all the files have been copied to the
'' destination folder and before the BOINC service is started.  We basically
'' copy all the account files from the source path to the destination folder.
''

Function Accounts_CopyFiles()

	Dim fso
	Dim szInstallDir
	Dim szAccountsLocation
	Dim iReturnValue
	
    On Error Resume Next

	Set fso = CreateObject("Scripting.FileSystemObject")
	szInstallDir = Property("INSTALLDIR")
	szAccountsLocation = Property("ACCOUNTS_LOCATION")
    
	If (Not( IsEmpty(szAccountsLocation) )) And (Not( IsNull(szAccountsLocation) )) Then

 	   	'' Append the wildcard search to the end of the account file location
  	   	'' string
    	szAccountsLocation = szAccountsLocation + "\*.xml"

    	fso.CopyFile szAccountsLocation, szInstallDir
    	If ErrHandler("Copy Account Files Failed ") Then
    	    iReturnValue = IDCANCEL
    	Else
    	    iReturnValue = IDOK
    	End If

	End If

    '' return success	
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
    'MsgBox(Str)
End Sub
