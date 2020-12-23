setlocal EnableDelayedExpansion
rem @echo off

for /f "tokens=2-4 delims=/ "  %%a in ("%date%") do (set MM=%%a& set DD=%%b& set YYYY=%%c)
set build_date=%YYYY%-%MM%-%DD%

set pkg_name=custom
rem GITHUB_SHA might not be available here, better use "git rev-parse --short=8 HEAD" instead
set git_rev=%GITHUB_SHA:~0,8%
set pkg_version=custom_%build_date%_!git_rev!
set pkg_version_desc=Custom build created on %build_date%
if not defined GITHUB_ACTIONS (
   set GITHUB_ACTIONS=False
)
set CI_RUN=%GITHUB_ACTIONS%

if "%CI_RUN%" == "True" (
    if "%GITHUB_EVENT_NAME%" == "pull_request" (
        set pkg_name=pull-requests
        set git_rev=%PULL_REQUEST_SHA:~0,8%
        set pkg_version=PR%PULL_REQUEST%_%build_date%_!git_rev!
        set pkg_version_desc=CI build created from PR #%PULL_REQUEST% on %build_date%
    )
    if "%GITHUB_EVENT_NAME%" == "schedule" (
        set pkg_name=weekly
        set git_rev=%GITHUB_SHA:~0,8%
        set pkg_version=weekly_%build_date%_!git_rev!
        set pkg_version_desc=Weekly CI build created on %build_date%
    )
    if "%GITHUB_EVENT_NAME%" == "push" (
        set pkg_name=master
        set git_rev=%GITHUB_SHA:~0,8%
        set pkg_version=master_%build_date%_!git_rev!
        set pkg_version_desc=Custom build created on %build_date%
    )
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

if not defined pkg_desc (
    set pkg_desc=Automated CI build of BOINC components
)

echo Creating package %pkg_name%...
set data={\"name\": \"!pkg_name!\", \"desc\": \"!pkg_desc!\", \"desc_url\": \"auto\", \"website_url\": [\"%WEBSITE_URL%\"], \"vcs_url\": [\"%VCS_URL%\"], \"issue_tracker_url\": [\"%ISSUE_TRACKER_URL%\"], \"licenses\": [\"LGPL-3.0\"]}
%CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%"

echo Creating version !pkg_version!...
set data={\"name\": \"!pkg_version!\", \"desc\": \"!pkg_version_desc!\"}
%CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/versions"

if exist "deploy\win_apps\win_apps.7z" (
    echo Uploading and publishing "deploy\win_apps\win_apps.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win_apps\win_apps.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win_apps_!pkg_version!.7z?publish=1&override=1"
)

if exist "deploy\win_client\win_client.7z" (
    echo Uploading and publishing "deploy\win_client\win_client.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win_client\win_client.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win_client_!pkg_version!.7z?publish=1&override=1"
)

if exist "deploy\win_manager\win_manager.7z" (
    echo Uploading and publishing "deploy\win_manager\win_manager.7z"
    %CURL% -H Content-Type:application/octet-stream -T "deploy\win_manager\win_manager.7z" "%API%/content/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/!pkg_version!/win_manager_!pkg_version!.7z?publish=1&override=1"
)

rem if defined APPVEYOR_JOB_ID (
rem     echo Adding AppVeyor log to release notes...
rem     set BUILD_LOG=https://ci.appveyor.com/api/buildjobs/%APPVEYOR_JOB_ID%/log
rem     set data='{"bintray": {"syntax": "markdown", "content": "'%BUILD_LOG%'"}}'
rem     %CURL% -H Content-Type:application/json -X POST -d "%data%" "%API%/packages/%BINTRAY_REPO_OWNER%/%BINTRAY_REPO%/!pkg_name!/versions/!pkg_version!/release_notes"
rem )

:EOF
