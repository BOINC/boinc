setlocal EnableDelayedExpansion
rem @echo off

if not exist "deploy\win_apps" mkdir deploy\win_apps
copy "win_build\Build\%platform%\%configuration%\htmlgfx*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\wrapper*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\vboxwrapper*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\boincsim.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\slide_show.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\example*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\worker*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\sleeper*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\boinclog.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\boincsim.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\multi_thread*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\slide_show.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\test*.exe" "deploy\win_apps\"
copy "win_build\Build\%platform%\%configuration%\wrappture*.exe" "deploy\win_apps\"
cd deploy\win_apps
7z a win_apps.7z *.exe
cd ..\..

if not exist "deploy\win_client" mkdir deploy\win_client
copy "win_build\Build\%platform%\%configuration%\boinc.exe" "deploy\win_client\"
copy "win_build\Build\%platform%\%configuration%\boincsvcctrl.exe" "deploy\win_client\"
copy "win_build\Build\%platform%\%configuration%\boinccmd.exe" "deploy\win_client\"
copy "win_build\Build\%platform%\%configuration%\boincscr.exe" "deploy\win_client\"
copy "win_build\Build\%platform%\%configuration%\boinc.scr" "deploy\win_client\"
cd deploy\win_client
7z a win_client.7z *.exe *.scr
cd ..\..

if not exist "deploy\win_manager" mkdir deploy\win_manager
copy "win_build\Build\%platform%\%configuration%\boinctray.exe" "deploy\win_manager\"
copy "win_build\Build\%platform%\%configuration%\boincmgr.exe" "deploy\win_manager\"
cd deploy\win_manager
7z a win_manager.7z *.exe
cd ..\..
