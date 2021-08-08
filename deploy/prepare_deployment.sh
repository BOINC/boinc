#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2021 University of California
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
## BOINC_TYPE should always be consistent with content in CI configuration files
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
    cp_if_exists samples/openclapp/openclapp "${TARGET_DIR}"
    cp_if_exists samples/wrappture/wrappture_example "${TARGET_DIR}"
    cp_if_exists samples/wrappture/fermi "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_manager() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists clientgui/boincmgr "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_apps_mingw() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists lib/wrapper.exe "${TARGET_DIR}"
    prepare_7z_archive
}

prepare_osx() {
    mkdir -p "${TARGET_DIR}"
}

prepare_android_manager() {
    mkdir -p "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/debug/app-debug.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/release/app-release-unsigned.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/xiaomi_debug/app-xiaomi_debug.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/xiaomi_release/app-xiaomi_release-unsigned.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/armv6_debug/app-armv6_debug.apk "${TARGET_DIR}"
    cp_if_exists android/BOINC/app/build/outputs/apk/armv6_release/app-armv6_release-unsigned.apk "${TARGET_DIR}"

    prepare_7z_archive
}

prepare_android_apps() {
    mkdir -p "${TARGET_DIR}"
    
    # boinc_gahp
    cp_if_exists samples/condor/android_armv6_boinc_gahp  "${TARGET_DIR}"
    cp_if_exists samples/condor/android_arm_boinc_gahp    "${TARGET_DIR}"
    cp_if_exists samples/condor/android_arm64_boinc_gahp  "${TARGET_DIR}"
    cp_if_exists samples/condor/android_x86_boinc_gahp    "${TARGET_DIR}"
    cp_if_exists samples/condor/android_x86_64_boinc_gahp "${TARGET_DIR}"

    # uc2
    cp_if_exists samples/example_app/android_armv6_uc2  "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_arm_uc2    "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_arm64_uc2  "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_x86_uc2    "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_x86_64_uc2 "${TARGET_DIR}"

    # ucn
    cp_if_exists samples/example_app/android_armv6_ucn  "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_arm_ucn    "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_arm64_ucn  "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_x86_ucn    "${TARGET_DIR}"
    cp_if_exists samples/example_app/android_x86_64_ucn "${TARGET_DIR}"

    # multi_thread
    cp_if_exists samples/multi_thread/android_armv6_multi_thread  "${TARGET_DIR}"
    cp_if_exists samples/multi_thread/android_arm_multi_thread    "${TARGET_DIR}"
    cp_if_exists samples/multi_thread/android_arm64_multi_thread  "${TARGET_DIR}"
    cp_if_exists samples/multi_thread/android_x86_multi_thread    "${TARGET_DIR}"
    cp_if_exists samples/multi_thread/android_x86_64_multi_thread "${TARGET_DIR}"

    # sleeper
    cp_if_exists samples/sleeper/android_armv6_sleeper  "${TARGET_DIR}"
    cp_if_exists samples/sleeper/android_arm_sleeper    "${TARGET_DIR}"
    cp_if_exists samples/sleeper/android_arm64_sleeper  "${TARGET_DIR}"
    cp_if_exists samples/sleeper/android_x86_sleeper    "${TARGET_DIR}"
    cp_if_exists samples/sleeper/android_x86_64_sleeper "${TARGET_DIR}"

    # worker
    cp_if_exists samples/worker/android_armv6_worker  "${TARGET_DIR}"
    cp_if_exists samples/worker/android_arm_worker    "${TARGET_DIR}"
    cp_if_exists samples/worker/android_arm64_worker  "${TARGET_DIR}"
    cp_if_exists samples/worker/android_x86_worker    "${TARGET_DIR}"
    cp_if_exists samples/worker/android_x86_64_worker "${TARGET_DIR}"

    # wrapper
    cp_if_exists samples/wrapper/android_armv6_wrapper  "${TARGET_DIR}"
    cp_if_exists samples/wrapper/android_arm_wrapper    "${TARGET_DIR}"
    cp_if_exists samples/wrapper/android_arm64_wrapper  "${TARGET_DIR}"
    cp_if_exists samples/wrapper/android_x86_wrapper    "${TARGET_DIR}"
    cp_if_exists samples/wrapper/android_x86_64_wrapper "${TARGET_DIR}"

    # wrappture_example
    cp_if_exists samples/wrappture/android_armv6_wrappture_example  "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_arm_wrappture_example    "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_arm64_wrappture_example  "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_x86_wrappture_example    "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_x86_64_wrappture_example "${TARGET_DIR}"

    # fermi
    cp_if_exists samples/wrappture/android_armv6_fermi  "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_arm_fermi    "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_arm64_fermi  "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_x86_fermi    "${TARGET_DIR}"
    cp_if_exists samples/wrappture/android_x86_64_fermi "${TARGET_DIR}"

    prepare_7z_archive
}

ROOTDIR=$(pwd)
if [[ $# -eq 0 || $# -gt 2 ]]; then
    echo "Usage: $0 BOINC_TYPE [TARGET_DIR]"
    echo "BOINC_TYPE : [linux_client | linux_apps | linux_manager-with-webview | linux_manager-without-webview | win_apps-mingw | osx_manager | android_manager]"
    echo "TARGET_DIR : relative path where binaries should be copied to (default: deploy/BOINC_TYPE/)"
    exit 1
fi

TYPE="$1"
TARGET_DIR="${2:-deploy/$TYPE/}"

case $TYPE in
    linux_client)
        prepare_client
    ;;
    linux_apps)
        prepare_apps
    ;;
    linux_client-vcpkg)
        prepare_client
    ;;
    linux_apps-vcpkg)
        prepare_apps
    ;;
    linux_manager-with-webview)
        prepare_manager
    ;;
    linux_manager-without-webview)
        prepare_manager
    ;;
    win_apps-mingw)
        prepare_apps_mingw
    ;;
    osx_manager)
        prepare_osx
    ;;
    android_manager)
        prepare_android_manager
    ;;
    android_apps)
        prepare_android_apps
    ;;
    android_manager-vcpkg)
        prepare_android_manager
    ;;
    android_apps-vcpkg)
        prepare_android_apps
    ;;
    *)
        echo "unrecognized BOINC_TYPE $key"
        exit 1
    ;;
esac
