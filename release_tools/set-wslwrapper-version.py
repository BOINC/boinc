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

def set_vcxproj(version):
    for vcxproj in ['win_build/wsl_wrapper.vcxproj']:
        with open(vcxproj, 'r') as f:
            lines = f.readlines()
        with open(vcxproj, 'w') as f:
            for line in lines:
                if line.startswith('    <TargetVersion>'):
                    line = f'    <TargetVersion>{version}</TargetVersion>\n'
                f.write(line)

if (len(sys.argv) != 2):
    print('Usage: set-wslwrapper-version.py VERSION')
    exit(1)

version = sys.argv[1]

print(f'Setting wsl_wrapper version to {version}...')

set_vcxproj(version)

print('Done.')
