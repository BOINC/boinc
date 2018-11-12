REM @echo off

echo Start build

set dependencies_path=%1
set revision_lst_path=%dependencies_path%\revision.lst
set platform=%2
set configuration=%3

curl -o %TEMP%\revision.lst https://s3-us-west-2.amazonaws.com/boinc-win-dependencies/latest/revision.lst
set /p last_revision=<%TEMP%\revision.lst

if exist %revision_lst_path% (
  set /p temp_saved_revision=<%dependencies_path%\revision.lst
  set saved_revision=%temp_saved_revision%

  if "%last_revision%" == "%saved_revision%" (
    echo Dependencies are up-to-date
    goto :EOF
  )

if exist %dependencies_path% (
  rd /s /q %dependencies_path%
)

curl -o %TEMP%\boinc_dependencies.zip https://s3-us-west-2.amazonaws.com/boinc-win-dependencies/latest/boinc_depends_win_vs2013_%platform%_%configuration%.zip
7z x %TEMP%\boinc_dependencies.zip -o%dependencies_path%\..\

echo | set /p dummy="%last_revision%" > %revision_lst_path%

goto :EOF