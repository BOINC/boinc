
set BASEDIR=%1..\..\build
set PLATFORM=%2

DEL /S /Q %BASEDIR%\src 2> NUL
DEL /S /Q %BASEDIR%\src_screen 2> NUL

DEL /S /Q %BASEDIR%\installer* 2> NUL
DEL /S /Q %BASEDIR%\*_windows_*.exe 2> NUL
DEL /S /Q %BASEDIR%\en-us 2> NUL
DEL /S /Q %BASEDIR%\boinc_*.wxs 2> NUL
DEL /S /Q %BASEDIR%\boinc_*.msi 2> NUL
DEL /S /Q %BASEDIR%\boinc_*.wixpdb 2> NUL

robocopy %BASEDIR% %BASEDIR%\src /XD src /XD src_screen /XD prerequisites /XD en-us /XF boinc.scr /XF boinc.exe /E

if NOT "%PLATFORM%" == "ARM64" (
    robocopy %BASEDIR% %BASEDIR%\src_screen boinc.scr
) ELSE (
    MKDIR %BASEDIR%\src_screen
)

heat.exe dir "%BASEDIR%\src" -dr BOINCBIN -platform x64 -srd -gg -cg BINARYFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceBinDir -sw -o "%1boinc_binary_files.wxs"

heat.exe dir "%BASEDIR%\src_screen" -dr BOINCSCREENBIN -platform x64 -srd -gg -cg SCREENFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceScreenDir -sw -o "%1boinc_screensaver_files.wxs"

heat.exe dir "%1redist" -dr BOINCDATA -platform x64 -srd -gg -cg DATAFILES -indent 3 -nologo -projectname boinc -ke -suid -template fragment -var var.SourceDataDir -sw -o "%1boinc_data_files.wxs"
