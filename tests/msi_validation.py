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

import os
import sys

def main():
    if len(sys.argv) != 4:
        print("Usage: msi_validation.py <msival2_path> <boinc_msi_path> <darice_cub_path>")
        sys.exit(1)

    msival2_path = sys.argv[1]
    if not os.path.exists(msival2_path):
        print(f"'{msival2_path}' not found")
        sys.exit(1)

    msi_path = sys.argv[2]
    if not os.path.exists(msi_path):
        print(f"'{msi_path}' not found")
        sys.exit(1)

    cub_path = sys.argv[3]
    if not os.path.exists(cub_path):
        print(f"'{cub_path}' not found")
        sys.exit(1)

    ignore_list = [
        "ICE          Type       Description",
        "ICE07        WARNING   '_BOINCScreensaver_LiberationSans_Regular.ttf' is a Font and must be installed to the FontsFolder. Current Install Directory: 'INSTALLDIR'",
        "ICE30        WARNING   The target file 'BOINC.EXE|boinc.exe' might be installed in '[ProgramFiles64Folder]\\BOINC\\' by two different conditionalized components on an SFN system: 'BOINCServiceConfig' and '_BOINC'. If the conditions are not mutually exclusive, this will break the component reference counting system.",
        "ICE30        WARNING   The target file 'BOINC.EXE|boinc.exe' might be installed in '[ProgramFiles64Folder]\\BOINC\\' by two different conditionalized components on an LFN system: 'BOINCServiceConfig' and '_BOINC'. If the conditions are not mutually exclusive, this will break the component reference counting system.",
        "ICE43        ERROR     Component _BOINCManagerStartMenu has non-advertised shortcuts. It should use a registry key under HKCU as its KeyPath, not a file.",
        "ICE57        ERROR     Component '_ScreensaverEnableNT' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE57        ERROR     Component '_BOINCManagerStartup' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE57        ERROR     Component '_BOINCManagerStartMenu' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE61        WARNING   This product should remove only older versions of itself. The Maximum version is not less than the current product.",
    ]
    output = os.popen(f'"{msival2_path}" "{msi_path}" "{cub_path}" -f').read()
    error_found = False
    for line in output.splitlines():
        if line == '' or any(ignore in line for ignore in ignore_list):
            continue
        error_found = True
        print(line)

    if error_found:
        print("Validation failed")
        sys.exit(1)

    print("Validation succeeded")
    sys.exit(0)

if __name__ == "__main__":
    main()
