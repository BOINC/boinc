@echo off
set dependencies_git_path=%1
set dependencies_path=%2
set revision_lst_path=%dependencies_path%\revision.lst
set platform=%3
set configuration=%4

if exist %revision_lst_path% (
  set /p temp_saved_revision=<%dependencies_path%\revision.lst
  set saved_revision=%temp_saved_revision%

  for /f %%i in ('git ls-remote %dependencies_git_path% HEAD') do (
    set last_revision=%%i
    if "%last_revision%" == "%saved_revision%" (
       echo Dependencies are up-to-date
       goto :EOF
    )
    goto :PROCESS
  )
)

:PROCESS

if exist %dependencies_path% (
  rd /s /q %dependencies_path%
)

git clone -q --branch=master %dependencies_git_path% %dependencies_path%
  
rd /s /q %dependencies_path%\.git

if "%platform%" == "Win32" (
  rd /s /q %dependencies_path%\curl\mswin\x64
  rd /s /q %dependencies_path%\freetype\mswin\x64
  rd /s /q %dependencies_path%\ftgl\mswin\x64
  rd /s /q %dependencies_path%\openssl\mswin\x64
  rd /s /q %dependencies_path%\sqlite3\mswin\x64
  rd /s /q %dependencies_path%\wxwidgets\mswin\x64
  rd /s /q %dependencies_path%\zlib\mswin\x64
) else (
  rd /s /q %dependencies_path%\curl\mswin\Win32
  rd /s /q %dependencies_path%\freetype\mswin\Win32
  rd /s /q %dependencies_path%\ftgl\mswin\Win32
  rd /s /q %dependencies_path%\openssl\mswin\Win32
  rd /s /q %dependencies_path%\sqlite3\mswin\Win32
  rd /s /q %dependencies_path%\wxwidgets\mswin\Win32
  rd /s /q %dependencies_path%\zlib\mswin\Win32
)
  
if "%configuration%" == "Debug" (
  rd /s /q %dependencies_path%\curl\mswin\%platform%\Release
  rd /s /q %dependencies_path%\freetype\mswin\%platform%\Release
  rd /s /q %dependencies_path%\ftgl\mswin\%platform%\Release
  rd /s /q %dependencies_path%\openssl\mswin\%platform%\Release
  rd /s /q %dependencies_path%\sqlite3\mswin\%platform%\Release
  rd /s /q %dependencies_path%\wxwidgets\mswin\%platform%\Release
  rd /s /q %dependencies_path%\zlib\mswin\%platform%\Release
) else (
  rd /s /q %dependencies_path%\curl\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\freetype\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\ftgl\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\openssl\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\sqlite3\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\wxwidgets\mswin\%platform%\Debug
  rd /s /q %dependencies_path%\zlib\mswin\%platform%\Debug
)

echo | set /p dummy="%last_revision%" > %revision_lst_path%

goto :EOF