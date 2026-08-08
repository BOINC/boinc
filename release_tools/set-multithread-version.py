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

import sys

def set_configure_ac(version):
    with open('configure.ac', 'r') as f:
        lines = f.readlines()
    with open('configure.ac', 'w') as f:
        for line in lines:
            if line.startswith('MULTITHREAD_RELEASE='):
                line = f'MULTITHREAD_RELEASE={version}\n'
            f.write(line)

def set_version_h(version):
    with open('version.h', 'r') as f:
        lines = f.readlines()
    with open('version.h', 'w') as f:
        for line in lines:
            if line.startswith('#define MULTITHREAD_RELEASE'):
                line = f'#define MULTITHREAD_RELEASE {version}\n'
            f.write(line)

def set_vcxproj(version):
    for vcxproj in ['win_build/example_app_multi_thread.vcxproj']:
        with open(vcxproj, 'r') as f:
            lines = f.readlines()
        with open(vcxproj, 'w') as f:
            for line in lines:
                if line.startswith('    <TargetVersion>'):
                    line = f'    <TargetVersion>{version}</TargetVersion>\n'
                f.write(line)

if (len(sys.argv) != 2):
    print('Usage: set-multithread-version.py VERSION')
    exit(1)

version = sys.argv[1]

print(f'Setting multithread version to {version}...')

set_configure_ac(version)
set_version_h(version)
set_vcxproj(version)

print('Done.')
