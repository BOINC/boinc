
set BASEDIR=%1..\..\build

DEL /S /Q %BASEDIR%\src 2> NUL
DEL /S /Q %BASEDIR%\src_screen 2> NUL

robocopy %BASEDIR% %BASEDIR%\src_screen boincscr.exe
move %BASEDIR%\src_screen\boincscr.exe %BASEDIR%\src_screen\boinc.scr

robocopy %BASEDIR% %BASEDIR%\src /XD src /XF boincscr.exe

heat.exe dir "%BASEDIR%\src" -dr BOINCBIN -platform x64 -srd -gg -cg BINARYFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceBinDir -sw -o "%1boinc_binary_files.wxs"

heat.exe dir "%BASEDIR%\src_screen" -dr BOINCSCREENBIN -platform x64 -srd -gg -cg SCREENFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceScreenDir -sw -o "%1boinc_screensaver_files.wxs"

heat.exe dir "%1redist" -dr BOINCDATA -platform x64 -srd -gg -cg DATAFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceDataDir -sw -o "%1boinc_data_files.wxs"
