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
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import json
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
                line = f'AC_INIT(BOINC, {version})\n'
            f.write(line)

def set_version_h(version):
    major, minor, release = split_version(version)
    with open('version.h', 'r') as f:
        lines = f.readlines()
    with open('version.h', 'w') as f:
        for line in lines:
            if line.startswith('#define BOINC_MAJOR_VERSION'):
                line = f'#define BOINC_MAJOR_VERSION {major}\n'
            elif line.startswith('#define BOINC_MINOR_VERSION'):
                line = f'#define BOINC_MINOR_VERSION {minor}\n'
            elif line.startswith('#define BOINC_RELEASE'):
                line = f'#define BOINC_RELEASE {release}\n'
            elif line.startswith('#define BOINC_VERSION_STRING'):
                line = f'#define BOINC_VERSION_STRING "{major}.{minor}.{release}"\n'
            elif line.startswith('#define PACKAGE_STRING'):
                line = f'#define PACKAGE_STRING "BOINC {major}.{minor}.{release}"\n'
            elif line.startswith('#define PACKAGE_VERSION'):
                line = f'#define PACKAGE_VERSION "{major}.{minor}.{release}"\n'
            elif line.find('#define BOINC_PRERELEASE 1') != -1:
                if is_release(minor):
                    line = '//#define BOINC_PRERELEASE 1\n'
                else:
                    line = '#define BOINC_PRERELEASE 1\n'
            f.write(line)

def set_version_h_in(version):
    _, minor, _ = split_version(version)
    with open('version.h.in', 'r') as f:
        lines = f.readlines()
    with open('version.h.in', 'w') as f:
        for line in lines:
            if line.find('#define BOINC_PRERELEASE 1') != -1:
                if is_release(minor):
                    line = '//#define BOINC_PRERELEASE 1\n'
                else:
                    line = '#define BOINC_PRERELEASE 1\n'
            f.write(line)

def set_build_gradle(version):
    _, minor, _ = split_version(version)
    with open('android/BOINC/app/build.gradle', 'r') as f:
        lines = f.readlines()
    with open('android/BOINC/app/build.gradle', 'w') as f:
        for line in lines:
            if line.startswith('    def version = '):
                if (is_release(minor)):
                    line = f'    def version = \'{version}\'\n'
                else:
                    line = f'    def version = \'{version} : DEVELOPMENT\'\n'
            f.write(line)

def set_property_json(version):
    with open('installer/include/Property.json','r') as f:
        data = json.load(f)
    for item in data['Property']:
        if item['Property'] == 'ProductVersion':
            item['Value'] = version
            break
    with open('installer/include/Property.json','w') as f:
        json.dump(data, f, indent=4)

def set_snapcraft(version):
    with open('snap/snapcraft.yaml','r') as f:
        lines = f.readlines()
    with open('snap/snapcraft.yaml','w') as f:
        for line in lines:
            if line.startswith('version:'):
                line = f'version: "{version}"\n'
            f.write(line)

def set_snap_boinc_desktop(version):
    with open('snap/gui/boinc.desktop','r') as f:
        lines = f.readlines()
    with open('snap/gui/boinc.desktop','w') as f:
        for line in lines:
            if line.startswith('Version='):
                line = f'Version={version}\n'
            f.write(line)

if (len(sys.argv) != 2):
    print('Usage: set-client-version.py VERSION')
    exit(1)

version = sys.argv[1]

_, minor, release = split_version(version)

if (not is_release(minor) and release != 0):
    print(f'ERROR: for development version release number should be 0 but it\'s set to {release}')
    exit(1)

print(f'Setting BOINC client version to {version}...')

set_configure_ac(version)
set_version_h(version)
set_version_h_in(version)
set_build_gradle(version)
set_property_json(version)
set_snapcraft(version)
set_snap_boinc_desktop(version)

if (os.name == 'posix' and sys.platform != 'darwin'):
    print('Running autosetup...')
    subprocess.call('./_autosetup -c', shell=True)

print('Done.')
