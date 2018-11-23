rem @echo off
for /f "tokens=2-4 delims=/ "  %%a in ("%date%") do (set MM=%%a& set DD=%%b& set YYYY=%%c)
set build_date=%YYYY%-%MM%-%DD%
set git_rev=%APPVEYOR_REPO_COMMIT:~0,8%

rem Default values because %bintray_deploy% is currently unused
set pkg_name=master
set git_rev=%APPVEYOR_REPO_COMMIT:~0,8%
set pkg_version=master_%build_date%_%git_rev%

echo Default values:
echo pkg_name: %pkg_name%
echo git_rev: %pkg_name%
echo pkg_version: %pkg_name%
echo bintray_deploy: %bintray_deploy%

echo APPVEYOR_PULL_REQUEST_NUMBER: %APPVEYOR_PULL_REQUEST_NUMBER%
echo APPVEYOR_SCHEDULED_BUILD: %APPVEYOR_SCHEDULED_BUILD%
echo APPVEYOR_PULL_REQUEST_HEAD_COMMIT: %APPVEYOR_PULL_REQUEST_HEAD_COMMIT%
echo APPVEYOR_REPO_COMMIT: %APPVEYOR_REPO_COMMIT%
echo APPVEYOR_REPO_COMMIT_TIMESTAMP: %APPVEYOR_REPO_COMMIT_TIMESTAMP%
echo APPVEYOR_BUILD_WORKER_IMAGE: %APPVEYOR_BUILD_WORKER_IMAGE%


if defined APPVEYOR_PULL_REQUEST_NUMBER (
    echo Pull request path chosen
    set pkg_name=pull-requests
    set git_rev=%APPVEYOR_PULL_REQUEST_HEAD_COMMIT:~0,8%
    set pkg_version=PR%APPVEYOR_PULL_REQUEST_NUMBER%_%build_date%_%git_rev%
    set bintray_deploy=True
    echo pkg_name: %pkg_name%
    echo git_rev: %pkg_name%
    echo pkg_version: %pkg_name%
    echo bintray_deploy: %bintray_deploy%
)

if defined APPVEYOR_SCHEDULED_BUILD (
    if "%APPVEYOR_SCHEDULED_BUILD%" == "True" (
        echo Scheduled build path chosen
        set pkg_name=weekly
        set pkg_version=weekly_%build_date%_%git_rev%
        set bintray_deploy=True
        echo pkg_name: %pkg_name%
        echo git_rev: %pkg_name%
        echo pkg_version: %pkg_name%
        echo bintray_deploy: %bintray_deploy%
    )
)
echo finished echo variables
