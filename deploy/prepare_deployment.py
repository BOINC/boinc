#!/usr/bin/env python3

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
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
# You should have received a of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import sys

linux_client_list = [
    './client/boinc',
    './client/boinccmd',
    './client/scripts/boinc-client.service',
    './client/scripts/boinc-client',
    './client/scripts/boinc.bash',
    './client/scripts/boinc-client.conf',
    './packages/deb/*',
    './packages/generic/36x11-common_xhost-boinc',
    'locale/*/*.mo',
    './win_build/installerv2/redist/all_projects_list.xml'
]

linux_apps_list = [
    './samples/condor/boinc_gahp',
    './samples/example_app/uc2',
    './samples/example_app/ucn',
    './samples/example_app/uc2_graphics',
    './samples/example_app/slide_show',
    './samples/multi_thread/multi_thread*pc-linux-gnu',
    './samples/sleeper/sleeper',
    './samples/vboxmonitor/vboxmonitor',
    './samples/vboxwrapper/vboxwrapper*pc-linux-gnu',
    './samples/worker/worker*pc-linux-gnu',
    './samples/wrapper/wrapper*pc-linux-gnu',
    './samples/openclapp/openclapp',
    './samples/wrappture/wrappture_example',
    './samples/wrappture/fermi',
    './samples/sporadic/sporadic',
    './samples/docker_wrapper/docker_wrapper*pc-linux-gnu',
]

linux_manager_list = [
    './clientgui/boincmgr',
    './clientgui/skins',
    './clientgui/res/boinc.desktop',
    './clientgui/res/boinc.png',
    './clientgui/res/boinc.svg',
    'locale/*/*.mo',
]

mingw_apps_list = [
    './lib/wrapper.exe'
]

mingw_apps_vcpkg_list = [
    './samples/condor/boinc_gahp.exe',
    './samples/example_app/uc2.exe',
    './samples/example_app/ucn.exe',
    './samples/example_app/uc2_graphics.exe',
    './samples/example_app/slide_show.exe',
    './samples/multi_thread/multi_thread*.exe',
    './samples/sleeper/sleeper.exe',
    './samples/worker/worker*.exe',
    './samples/wrapper/wrapper*.exe',
    './samples/wrappture/wrappture_example.exe',
    './samples/wrappture/fermi.exe',
    './samples/sporadic/sporadic.exe',
    './samples/wsl_wrapper/wsl_wrapper.exe',
    './samples/docker_wrapper/docker_wrapper*.exe',
]

android_apps_list = [
    # boinc_gahp
    './samples/condor/android_armv6_boinc_gahp',
    './samples/condor/android_arm_boinc_gahp',
    './samples/condor/android_arm64_boinc_gahp',
    './samples/condor/android_x86_boinc_gahp',
    './samples/condor/android_x86_64_boinc_gahp',
    # uc2
    './samples/example_app/android_armv6_uc2',
    './samples/example_app/android_arm_uc2',
    './samples/example_app/android_arm64_uc2',
    './samples/example_app/android_x86_uc2',
    './samples/example_app/android_x86_64_uc2',
    # ucn
    './samples/example_app/android_armv6_ucn',
    './samples/example_app/android_arm_ucn',
    './samples/example_app/android_arm64_ucn',
    './samples/example_app/android_x86_ucn',
    './samples/example_app/android_x86_64_ucn',
    # multi_thread
    './samples/multi_thread/android_armv6_multi_thread',
    './samples/multi_thread/android_arm_multi_thread',
    './samples/multi_thread/android_arm64_multi_thread',
    './samples/multi_thread/android_x86_multi_thread',
    './samples/multi_thread/android_x86_64_multi_thread',
    # sleeper
    './samples/sleeper/android_armv6_sleeper',
    './samples/sleeper/android_arm_sleeper',
    './samples/sleeper/android_arm64_sleeper',
    './samples/sleeper/android_x86_sleeper',
    './samples/sleeper/android_x86_64_sleeper',
    # worker
    './samples/worker/android_armv6_worker',
    './samples/worker/android_arm_worker',
    './samples/worker/android_arm64_worker',
    './samples/worker/android_x86_worker',
    './samples/worker/android_x86_64_worker',
    # wrapper
    './samples/wrapper/android_armv6_wrapper',
    './samples/wrapper/android_arm_wrapper',
    './samples/wrapper/android_arm64_wrapper',
    './samples/wrapper/android_x86_wrapper',
    './samples/wrapper/android_x86_64_wrapper',
    # wrappture_example
    './samples/wrappture/android_armv6_wrappture_example',
    './samples/wrappture/android_arm_wrappture_example',
    './samples/wrappture/android_arm64_wrappture_example',
    './samples/wrappture/android_x86_wrappture_example',
    './samples/wrappture/android_x86_64_wrappture_example',
    # fermi
    './samples/wrappture/android_armv6_fermi',
    './samples/wrappture/android_arm_fermi',
    './samples/wrappture/android_arm64_fermi',
    './samples/wrappture/android_x86_fermi',
    './samples/wrappture/android_x86_64_fermi',
    # sporadic
    './samples/sporadic/android_armv6_sporadic',
    './samples/sporadic/android_arm_sporadic',
    './samples/sporadic/android_arm64_sporadic',
    './samples/sporadic/android_x86_sporadic',
    './samples/sporadic/android_x86_64_sporadic'
]

windows_apps_list = [
    './win_build/Build/x64/Release/htmlgfx*.exe',
    './win_build/Build/x64/Release/wrapper*.exe',
    './win_build/Build/x64/Release/vboxwrapper*.exe',
    './win_build/Build/x64/Release/boincsim.exe',
    './win_build/Build/x64/Release/slide_show.exe',
    './win_build/Build/x64/Release/example*.exe',
    './win_build/Build/x64/Release/worker*.exe',
    './win_build/Build/x64/Release/sleeper*.exe',
    './win_build/Build/x64/Release/boinclog.exe',
    './win_build/Build/x64/Release/multi_thread*.exe',
    './win_build/Build/x64/Release/test*.exe',
    './win_build/Build/x64/Release/wrappture*.exe',
    './win_build/Build/x64/Release/crypt_prog.exe',
    './win_build/Build/x64/Release/wsl_wrapper.exe',
    './win_build/Build/x64/Release/docker_wrapper*.exe',
    './win_build/Build/x64/Release/cudart*.dll',
    './win_build/Build/ARM64/Release/htmlgfx*.exe',
    './win_build/Build/ARM64/Release/wrapper*.exe',
    './win_build/Build/ARM64/Release/vboxwrapper*.exe',
    './win_build/Build/ARM64/Release/boincsim.exe',
    './win_build/Build/ARM64/Release/slide_show.exe',
    './win_build/Build/ARM64/Release/example*.exe',
    './win_build/Build/ARM64/Release/worker*.exe',
    './win_build/Build/ARM64/Release/sleeper*.exe',
    './win_build/Build/ARM64/Release/boinclog.exe',
    './win_build/Build/ARM64/Release/multi_thread*.exe',
    './win_build/Build/ARM64/Release/test*.exe',
    './win_build/Build/ARM64/Release/wrappture*.exe',
    './win_build/Build/ARM64/Release/crypt_prog.exe',
    './win_build/Build/ARM64/Release/wsl_wrapper.exe',
    './win_build/Build/ARM64/Release/docker_wrapper*.exe',
]

windows_client_list = [
    './win_build/Build/x64/Release/boinc.exe',
    './win_build/Build/x64/Release/boincsvcctrl.exe',
    './win_build/Build/x64/Release/boinccmd.exe',
    './win_build/Build/x64/Release/boincscr.exe',
    './win_build/Build/x64/Release/boinc.scr',
    './win_build/Build/ARM64/Release/boinc.exe',
    './win_build/Build/ARM64/Release/boincsvcctrl.exe',
    './win_build/Build/ARM64/Release/boinccmd.exe',
    './win_build/Build/ARM64/Release/boincscr.exe',
    './win_build/Build/ARM64/Release/boinc.scr',
    './curl/ca-bundle.crt'
]

windows_manager_list = [
    './win_build/Build/x64/Release/boinctray.exe',
    './win_build/Build/x64/Release/boincmgr.exe',
    './win_build/Build/ARM64/Release/boinctray.exe',
    './win_build/Build/ARM64/Release/boincmgr.exe',
    './clientgui/skins',
    'locale/*/*.mo',
]

windows_installer_list = [
    './win_build/Build/x64/Release/boinccas.dll',
    './win_build/Build/ARM64/Release/boinccas.dll',
    './win_build/Build/x64/Release/installer_icon.exe',
    './win_build/Build/ARM64/Release/installer_icon.exe',
    './win_build/Build/x64/Release/installer.exe',
    './win_build/Build/ARM64/Release/installer.exe',
    './win_build/Build/x64/Release/boinc.msi',
    './win_build/Build/ARM64/Release/boinc.msi',
    './win_build/Build/x64/Release/installer_setup.exe',
    './win_build/Build/ARM64/Release/installer_setup.exe',
]

macos_manager_list = [
    'mac_build/build/Deployment/AddRemoveUser',
    'mac_build/build/Deployment/BOINC\ Installer.app',
    'mac_build/build/Deployment/BOINCManager.app',
    'mac_build/build/Deployment/BOINCSaver.saver',
    'mac_build/build/Deployment/BOINC_Finish_Install.app',
    'mac_build/build/Deployment/PostInstall.app',
    'mac_build/build/Deployment/SetUpSecurity',
    'mac_build/build/Deployment/SetVersion',
    'mac_build/build/Deployment/Uninstall\ BOINC.app',
    'mac_build/build/Deployment/boinc',
    'mac_build/build/Deployment/boinccmd',
    'mac_build/build/Deployment/boincscr',
    'mac_build/build/Deployment/gfx_cleanup',
    'mac_build/build/Deployment/gfx_switcher',
    'mac_build/build/Deployment/setprojectgrp',
    'mac_build/build/Deployment/switcher',
    'mac_build/build/Deployment/detect_rosetta_cpu',
    'mac_build/build/Deployment/Run_Podman',
]

macos_apps_list = [
    'samples/docker_wrapper/build/Deployment/docker_wrapper',
    'samples/mac_build/build/Deployment/UC2-apple-darwin',
    'samples/mac_build/build/Deployment/UC2_graphics-apple-darwin',
    'samples/mac_build/build/Deployment/slide_show-apple-darwin',
    'samples/vboxwrapper/build/Deployment/vboxwrapper',
]

macos_makefile_apps_list = [
    'samples/wrapper/wrapper',
    'samples/vboxwrapper/vboxwrapper',
]

macos_apps_x86_64_list = [
    'samples/docker_wrapper/docker_wrapper_x86_64',
    'samples/example_app/x86_64/uc2',
    'samples/example_app/uc2_x86_64',
    'samples/example_app/x86_64/uc2_graphics',
    'samples/example_app/uc2_graphics_x86_64',
    'samples/example_app/x86_64/slide_show',
    'samples/example_app/slide_show_x86_64',
    'samples/openclapp/openclapp_x86_64',
]

macos_apps_arm64_list = [
    'samples/docker_wrapper/docker_wrapper_arm64',
    'samples/example_app/arm64/uc2',
    'samples/example_app/uc2_arm64',
    'samples/example_app/arm64/uc2_graphics',
    'samples/example_app/uc2_graphics_arm64',
    'samples/example_app/arm64/slide_show',
    'samples/example_app/slide_show_arm64',
    'samples/openclapp/openclapp_arm64',
]

logs_list = [
    'config.log',
    '3rdParty/linux/vcpkg/buildtrees/*.log',
    '3rdParty/osx/vcpkg/buildtrees/*.log',
    '3rdParty/android/vcpkg/buildtrees/*.log',
    '3rdParty/mingw/vcpkg/buildtrees/*.log',
    '3rdParty/Windows/vcpkg/buildtrees/*.log',
    'parts/boinc/build/3rdParty/linux/vcpkg/buildtrees/*.log',
    'android/BOINC/app/build/reports/',
    'mac_build/xcodebuild_*.log',
    'build/*.log',
]

def prepare_7z_archive(archive_name, target_directory, files_list):
    os.makedirs(target_directory, exist_ok=True)
    archive_path = os.path.join(target_directory, archive_name + '.7z')
    command = '7z a -t7z -r -mx=9 -xr!*.dSYM -xr!Makefile -xr!Makefile.* ' + archive_path + ' ' + " ".join(files_list)
    os.system(command)

def help():
    print('Usage: python preprare_deployment.py BOINC_TYPE')
    print('BOINC_TYPE : [' + " | ".join(boinc_types.keys()) + ']')

def prepare_linux_client(target_directory):
    prepare_7z_archive('linux_client', target_directory, linux_client_list)

def prepare_linux_client_vcpkg(target_directory):
    prepare_7z_archive('linux_client-vcpkg', target_directory, linux_client_list)

def prepare_linux_client_vcpkg_arm64(target_directory):
    prepare_7z_archive('linux_client-vcpkg-arm64', target_directory, linux_client_list)

def prepare_linux_apps(target_directory):
    prepare_7z_archive('linux_apps', target_directory, linux_apps_list)

def prepare_linux_apps_arm64(target_directory):
    prepare_7z_archive('linux_apps-arm64', target_directory, linux_apps_list)

def prepare_linux_apps_vcpkg_arm64(target_directory):
    prepare_7z_archive('linux_apps-vcpkg-arm64', target_directory, linux_apps_list)

def prepare_linux_apps_vcpkg(target_directory):
    prepare_7z_archive('linux_apps-vcpkg', target_directory, linux_apps_list)

def prepare_linux_manager(target_directory):
    prepare_7z_archive('linux_manager', target_directory, linux_manager_list)

def prepare_linux_manager_with_webview(target_directory):
    prepare_7z_archive('linux_manager-with-webview', target_directory, linux_manager_list)

def prepare_linux_manager_with_webview_vcpkg(target_directory):
    prepare_7z_archive('linux_manager-with-webview-vcpkg', target_directory, linux_manager_list)

def prepare_linux_manager_with_webview_vcpkg_arm64(target_directory):
    prepare_7z_archive('linux_manager-with-webview-vcpkg-arm64', target_directory, linux_manager_list)

def prepare_linux_manager_without_webview(target_directory):
    prepare_7z_archive('linux_manager-without-webview', target_directory, linux_manager_list)

def prepare_win_apps_mingw(target_directory):
    prepare_7z_archive('win_apps-mingw', target_directory, mingw_apps_list)

def prepare_win_apps_mingw_vcpkg(target_directory):
    prepare_7z_archive('win_apps-mingw-vcpkg', target_directory, mingw_apps_vcpkg_list)

def prepare_android_apps(target_directory):
    prepare_7z_archive('android_apps', target_directory, android_apps_list)

def prepare_win_apps(target_directory):
    prepare_7z_archive('win_apps', target_directory, windows_apps_list)

def prepare_win_client(target_directory):
    prepare_7z_archive('win_client', target_directory, windows_client_list)

def prepare_win_manager(target_directory):
    prepare_7z_archive('win_manager', target_directory, windows_manager_list)

def prepare_win_installer(target_directory):
    prepare_7z_archive('win_installer', target_directory, windows_installer_list)

def prepare_macos_apps(target_directory):
    prepare_7z_archive('macos_manager', target_directory, macos_manager_list)
    prepare_7z_archive('macos_apps', target_directory, macos_apps_list)

def prepare_macos_makefile_apps(target_directory):
    prepare_7z_archive('macos_apps', target_directory, macos_makefile_apps_list)
    prepare_7z_archive('macos_apps_x86_64', target_directory, macos_apps_x86_64_list)
    prepare_7z_archive('macos_apps_arm64', target_directory, macos_apps_arm64_list)

def prepare_logs(target_directory):
    prepare_7z_archive('logs', target_directory, logs_list)

boinc_types = {
    'android_apps': prepare_android_apps,
    'linux_apps': prepare_linux_apps,
    'linux_apps-arm64': prepare_linux_apps_arm64,
    'linux_apps-vcpkg': prepare_linux_apps_vcpkg,
    'linux_apps-vcpkg-arm64': prepare_linux_apps_vcpkg_arm64,
    'linux_client': prepare_linux_client,
    'linux_client-vcpkg': prepare_linux_client_vcpkg,
    'linux_client-vcpkg-arm64': prepare_linux_client_vcpkg_arm64,
    'linux_manager': prepare_linux_manager,
    'linux_manager-with-webview': prepare_linux_manager_with_webview,
    'linux_manager-with-webview-vcpkg': prepare_linux_manager_with_webview_vcpkg,
    'linux_manager-with-webview-vcpkg-arm64': prepare_linux_manager_with_webview_vcpkg_arm64,
    'linux_manager-without-webview': prepare_linux_manager_without_webview,
    'logs': prepare_logs,
    'macos_manager': prepare_macos_apps,
    'macos_samples-makefile': prepare_macos_makefile_apps,
    'win_apps': prepare_win_apps,
    'win_apps-mingw': prepare_win_apps_mingw,
    'win_apps-mingw-vcpkg': prepare_win_apps_mingw_vcpkg,
    'win_client': prepare_win_client,
    'win_installer': prepare_win_installer,
    'win_manager': prepare_win_manager,
}

if (len(sys.argv) != 2):
    help()
    sys.exit(1)

boinc_type = sys.argv[1]
target_dir = 'deploy'

if (boinc_type not in boinc_types):
    print('Unknown BOINC_TYPE: ' + boinc_type)
    help()
    sys.exit(1)

boinc_types[boinc_type](target_dir)
