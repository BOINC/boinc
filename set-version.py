# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2022 University of California
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

import os
import subprocess
import sys

def split_version(version):
    major = int(version.split('.')[0])
    minor = int(version.split('.')[1])
    release = int(version.split('.')[2])
    return major, minor, release

def is_release(minor):
    return minor % 2 == 0

def set_configure_ac(version):
    with open('configure.ac', 'r') as f:
        lines = f.readlines()
    with open('configure.ac', 'w') as f:
        for line in lines:
            if line.startswith('AC_INIT'):
                line = 'AC_INIT(BOINC, %s)\n' % (version)
            f.write(line)

def set_version_h(version):
    major, minor, release = split_version(version)
    with open('version.h', 'r') as f:
        lines = f.readlines()
    with open('version.h', 'w') as f:
        for line in lines:
            if line.startswith('#define BOINC_MAJOR_VERSION'):
                line = '#define BOINC_MAJOR_VERSION %s\n' % (major)
            elif line.startswith('#define BOINC_MINOR_VERSION'):
                line = '#define BOINC_MINOR_VERSION %s\n' % (minor)
            elif line.startswith('#define BOINC_RELEASE'):
                line = '#define BOINC_RELEASE %s\n' % (release)
            elif line.startswith('#define BOINC_VERSION_STRING'):
                line = '#define BOINC_VERSION_STRING "%s.%s.%s"\n' % (major, minor, release)
            elif line.startswith('#define PACKAGE_STRING'):
                line = '#define PACKAGE_STRING "BOINC %s.%s.%s"\n' % (major, minor, release)
            elif line.startswith('#define PACKAGE_VERSION'):
                line = '#define PACKAGE_VERSION "%s.%s.%s"\n' % (major, minor, release)
            elif line.find('#define BOINC_PRERELEASE 1') != -1:
                if is_release(minor):
                    line = '//#define BOINC_PRERELEASE 1\n'
                else:
                    line = '#define BOINC_PRERELEASE 1\n'
            f.write(line)

def set_version_log(version):
    with open('version.log', 'w') as f:
        line = '%s\n' % (version)
        f.write(line)

def set_build_gradle(version):
    _, minor, _ = split_version(version)
    if (is_release(minor)):
        return
    with open('android/BOINC/app/build.gradle', 'r') as f:
        lines = f.readlines()
    with open('android/BOINC/app/build.gradle', 'w') as f:
        for line in lines:
            if line.startswith('    def version = '):
                line = '    def version = \'%s : DEVELOPMENT\'\n' % (version)
            f.write(line)

def set_vcpkg_json(version):
    for json in ['android/vcpkg_config_apps/vcpkg.json',
                 'android/vcpkg_config_client/vcpkg.json',
                 'android/vcpkg_config_libs/vcpkg.json',
                 'lib/vcpkg.json',
                 'lib/vcpkg_config_android/vcpkg.json',
                 'linux/vcpkg_config_apps/vcpkg.json',
                 'linux/vcpkg_config_client/vcpkg.json',
                 'linux/vcpkg_config_libs/vcpkg.json',
                 'mingw/vcpkg_config_apps/vcpkg.json',
                 'win_build/vcpkg_config_msbuild_ARM64/vcpkg.json',
                 'win_build/vcpkg_config_msbuild_x64/vcpkg.json']:
        with open(json, 'r') as f:
            lines = f.readlines()
        with open(json, 'w') as f:
            for line in lines:
                if line.startswith('    "version-string":'):
                    line = '    "version-string": "%s",\n' % (version)
                f.write(line)

def set_installshield(version):
    for ism in ['win_build/installerv2/BOINCx64_vbox.ism', 'win_build/installerv2/BOINCx64.ism']:
        with open(ism, 'r') as f:
            lines = f.readlines()
        with open(ism, 'w') as f:
            for line in lines:
                if line.startswith('		<row><td>ProductVersion</td><td>'):
                    line = '		<row><td>ProductVersion</td><td>%s</td><td/></row>\n' % (version)
                f.write(line)

if (len(sys.argv) != 2):
    print('Usage: set-version.py VERSION')
    exit(1)

version = sys.argv[1]

_, minor, release = split_version(version)

if (not is_release(minor) and release != 0):
    print('ERROR: for development version release number should be 0 but it\'s set to %s' % (release))
    exit(1)

print('Setting BOINC version to %s...' % (version))

set_configure_ac(version)
set_version_h(version)
set_version_log(version)
set_build_gradle(version)
set_vcpkg_json(version)
set_installshield(version)

if (os.name == 'posix' and sys.platform != 'darwin'):
    print('Running autosetup...')
    subprocess.call('./_autosetup -c', shell=True)

print('Done.')
