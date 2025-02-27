# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2024 University of California
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

mingw_apps_list = [
    './lib/wrapper.exe'
]

mingw_apps_vcpkg_list = [
    './samples/condor/boinc_gahp.exe',
    './samples/example_app/uc2.exe',
    './samples/example_app/ucn.exe',
    './samples/example_app/uc2_graphics.exe',
    './samples/example_app/slide_show.exe',
    './samples/multi_thread/multi_thread.exe',
    './samples/sleeper/sleeper.exe',
    './samples/worker/worker*.exe',
    './samples/wrapper/wrapper*.exe',
    './samples/wrappture/wrappture_example.exe',
    './samples/wrappture/fermi.exe',
    './samples/sporadic/sporadic.exe',
    './samples/wsl_wrapper/wsl_wrapper.exe',
    './samples/docker_wrapper/docker_wrapper.exe',
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
    './win_build/Build/x64/Release/docker_wrapper.exe',
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
    './win_build/Build/ARM64/Release/docker_wrapper.exe',
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

wasm_client_list = [
    './client/boinc_client.wasm',
    './client/boinc_client.js',
    './client/boinc.html',
    './samples/wasm/index.html',
]

wasm_client_debug_folder_list = [
    'lib/*.cpp',
    'lib/*.h',
    'client/*.cpp',
    'client/*.h',
    'client/boinc_client.html',
    'client/boinc_client.js',
    'client/boinc_client.wasm',
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
]

macos_apps_list = [
    'zip/build/Deployment/boinc_zip_test',
    'zip/build/Deployment/testzlibconflict',
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
    'samples/example_app/x86_64/uc2',
    'samples/example_app/uc2_x86_64',
    'samples/example_app/x86_64/uc2_graphics',
    'samples/example_app/uc2_graphics_x86_64',
    'samples/example_app/x86_64/slide_show',
    'samples/example_app/slide_show_x86_64',
    'samples/openclapp/openclapp_x86_64',
]

macos_apps_arm64_list = [
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
    '3rdParty/wasm/vcpkg/buildtrees/*.log',
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

def prepare_win_apps_mingw(target_directory):
    prepare_7z_archive('win_apps-mingw', target_directory, mingw_apps_list)

def prepare_win_apps_mingw_vcpkg(target_directory):
    prepare_7z_archive('win_apps-mingw-vcpkg', target_directory, mingw_apps_vcpkg_list)

def prepare_win_apps(target_directory):
    prepare_7z_archive('win_apps', target_directory, windows_apps_list)

def prepare_win_client(target_directory):
    prepare_7z_archive('win_client', target_directory, windows_client_list)

def prepare_win_manager(target_directory):
    prepare_7z_archive('win_manager', target_directory, windows_manager_list)

def prepare_win_installer(target_directory):
    prepare_7z_archive('win_installer', target_directory, windows_installer_list)

def prepare_wasm_client(target_directory):
    prepare_7z_archive('wasm_client', target_directory, wasm_client_list)

def prepare_wasm_client_debug(target_directory):
    prepare_7z_archive('wasm_client-debug', target_directory, wasm_client_debug_folder_list)

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
    'win_apps-mingw': prepare_win_apps_mingw,
    'win_apps-mingw-vcpkg': prepare_win_apps_mingw_vcpkg,
    'win_apps': prepare_win_apps,
    'win_client': prepare_win_client,
    'win_manager': prepare_win_manager,
    'win_installer': prepare_win_installer,
    'wasm_client': prepare_wasm_client,
    'wasm_client-debug': prepare_wasm_client_debug,
    'macos_manager': prepare_macos_apps,
    'macos_samples-makefile': prepare_macos_makefile_apps,
    'logs': prepare_logs,
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
