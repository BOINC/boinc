#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2018 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

## support script to put all build artefacts into a defined location
## BOINC_TYPE should always be consistent with content in .travis.yml and appveyor.yml
## Change artefacts in each prepare_*() function below.
## Don't hardlink files because this can be run on a filesystem without hardlinks
## On error always exit non-zero so the deploy script does not run

# check working directory because the script needs to be called like: ./deploy/prepare_deployment.sh
if [ ! -d "deploy" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

ROOTDIR=$(pwd)
# main funtion is at the end

cp_if_exists() {
    if [ -e "${1}" ]; then
        cp -f "${1}" "${2}"
    fi
}

prepare_7z_archive() {
    if [[ $(ls -A "${TARGET_DIR}" | wc -l) -eq 0 ]]; then
        echo "Directory '$TARGET_DIR' is empty";
        exit 1;
    fi
    cd "${TARGET_DIR}"
    7z a "${TYPE}.7z" '-x!*.7z' '*'  &>/dev/null
    if [ $? -gt 1 ]; then # an exit code of 1 is still a success says 7z
        cd ${ROOTDIR}
        echo "error while creating 7z archive; files not uploaded"
        exit 1
    fi
}

prepare_client() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists client/boinc "${TARGET_DIR}"
    cp_if_exists client/boinccmd "${TARGET_DIR}"
    cp_if_exists client/switcher "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_apps() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists samples/condor/boinc_gahp "${TARGET_DIR}"
    cp_if_exists samples/example_app/uc2 "${TARGET_DIR}"
    cp_if_exists samples/example_app/ucn "${TARGET_DIR}"
    cp_if_exists samples/example_app/uc2_graphics "${TARGET_DIR}"
    cp_if_exists samples/example_app/slide_show "${TARGET_DIR}"
    cp_if_exists samples/multi_thread/multi_thread "${TARGET_DIR}"
    cp_if_exists samples/sleeper/sleeper "${TARGET_DIR}"
    cp_if_exists samples/vboxmonitor/vboxmonitor "${TARGET_DIR}"
    cp_if_exists samples/vboxwrapper/vboxwrapper "${TARGET_DIR}"
    cp_if_exists samples/worker/worker "${TARGET_DIR}"
    cp_if_exists samples/wrapper/wrapper "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_manager() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists clientgui/boincmgr "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_apps_mingw() {
    mkdir -p "${TARGET_DIR}"
}

prepare_osx() {
    mkdir -p "${TARGET_DIR}"
}

prepare_android() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/debug/app-debug.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/release/app-release-unsigned.apk "${TARGET_DIR}"
    prepare_7z_archive
}

ROOTDIR=$(pwd)
if [[ $# -eq 0 || $# -gt 2 ]]; then
    echo "Usage: $0 BOINC_TYPE [TARGET_DIR]"
    echo "BOINC_TYPE : [client | apps | manager | apps-mingw | manager-osx | manager-android]"
    echo "TARGET_DIR : relative path where binaries should be copied to (default: deploy/BOINC_TYPE/)"
    exit 1
fi

TYPE="$1"
TARGET_DIR="${2:-deploy/$TYPE/}"

case $TYPE in
    client)
        prepare_client
    ;;
    apps)
        prepare_apps
    ;;
    manager)
        prepare_manager
    ;;
    apps-mingw)
        prepare_apps_mingw
    ;;
    manager-osx)
        prepare_osx
    ;;
    manager-android)
        prepare_android
    ;;
    *)
        echo "unrecognized BOINC_TYPE $key"
        exit 1
    ;;
esac
