setlocal EnableDelayedExpansion
rem @echo off

if not defined bintray_deploy (
    set bintray_deploy=False
)
if !bintray_deploy! == False (
    goto :EOF
)

if not defined BINTRAY_API_KEY (
    goto :EOF
)
if [%BINTRAY_API_KEY%] == [] (
    goto :EOF
)

set API=https://api.bintray.com
if not defined BINTRAY_USER (
    set BINTRAY_USER=ChristianBeer
)
if not defined BINTRAY_REPO (
    set BINTRAY_REPO=boinc-ci
)
rem owner and user not always the same
if not defined BINTRAY_REPO_OWNER (
    set BINTRAY_REPO_OWNER=boinc
)
if not defined WEBSITE_URL (
    set WEBSITE_URL=https://boinc.berkeley.edu
)
if not defined ISSUE_TRACKER_URL (
    set ISSUE_TRACKER_URL=https://github.com/BOINC/boinc/issues
)
rem Mandatory for packages in free Bintray repos
if not defined VCS_URL (
    set VCS_URL=https://github.com/BOINC/boinc.git
) 
set CURL=curl -u%BINTRAY_USER%:%BINTRAY_API_KEY% -H Accept:application/json -w \n
rem use this for local debugging
rem set CURL=echo

if not defined pkg_name (
    set pkg_name=custom
)
if not defined pkg_desc (
    set pkg_desc=Automated CI build of BOINC components
)

echo Creating package %pkg_name%...
set data={\"name\": \"!pkg_name!\", \"desc\": \"!pkg_desc!\", \"desc_url\": \"auto\", \"website_url\": [\"%WEBSITE_URL%\"], \"vcs_url\": [\"%VCS_URL%\"], \"issue_tracker_url\": [\"%ISSUE_TRACKER_URL%\"], \"licenses\": [\"LGPL-3.0\"]}
%CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%"

for /f "tokens=2-4 delims=/ "  %%a in ("%date%") do (set MM=%%a& set DD=%%b& set YYYY=%%c)
if not defined build_date (
    set build_date=%YYYY%-%MM%-%DD%
)
if not defined git_rev (
    set git_rev=%APPVEYOR_REPO_COMMIT:~0,8%
)
if not defined pkg_version (
    set pkg_version=custom_%build_date%_!git_rev!
)
if not defined pkg_version_desc (
    set pkg_version_desc=Custom build created on %build_date%
)

echo Creating version !pkg_version!...
set data={\"name\": \"!pkg_version!\", \"desc\": \"!pkg_version_desc!\"}
%CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/versions"

if exist "deploy\win-apps\win-apps_!pkg_version!_%platform%.7z" (
    echo Uploading and publishing "deploy\win-apps\win-apps_!pkg_version!_%platform%.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win-apps\win-apps_!pkg_version!_%platform%.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win-apps_!pkg_version!_%platform%.7z?publish=1&override=1"
)

if exist "deploy\win-client\win-client_!pkg_version!_%platform%.7z" (
    echo Uploading and publishing "deploy\win-client\win-client_!pkg_version!_%platform%.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win-client\win-client_!pkg_version!_%platform%.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win-client_!pkg_version!_%platform%.7z?publish=1&override=1"
)

if exist "deploy\win-manager\win-manager_!pkg_version!_%platform%.7z" (
    echo Uploading and publishing "deploy\win-manager\win-manager_!pkg_version!_%platform%.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win-manager\win-manager_!pkg_version!_%platform%.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win-manager_!pkg_version!_%platform%.7z?publish=1&override=1"
)

rem if defined APPVEYOR_JOB_ID (
rem     echo Adding AppVeyor log to release notes...
rem     set BUILD_LOG=https://ci.appveyor.com/api/buildjobs/%APPVEYOR_JOB_ID%/log
rem     set data='{"bintray": {"syntax": "markdown", "content": "'%BUILD_LOG%'"}}'
rem     %CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/versions/!pkg_version!/release_notes"
rem )

:EOF
