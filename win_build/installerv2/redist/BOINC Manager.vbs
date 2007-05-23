set objShell = CreateObject("Wscript.Shell") 
objShell.Run """C:\Program Files\BOINC\boincmgr.exe"" /s"
'' objShell.ShellExecute "cmd.exe", "/C ""C:\Program Files\BOINC\boincmgr.exe""", , , 1