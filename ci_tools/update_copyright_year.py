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

import datetime
import os
import pathlib
import re
import sys

def help():
    print('Usage:')
    print('python update_copyright_year.py <dir>')

def update_dlgabout(filename, year=datetime.date.today().year):
    print('Checking file: ' + filename)
    with open(filename, "rb") as f:
        lines = f.read().splitlines()
    count = 0
    changed = False
    match_found = False
    for line in lines:
        count += 1
        match = re.search(b'.*\(C\)\s\d{4}-\d{4}.*', line)
        if match:
            match_found = True
            updated_line = re.sub(b'-(\d{4})', b'-' + str.encode(str(year)), line)
            if updated_line != line:
                lines[count-1] = updated_line
                changed = True
    if not match_found:
        print('There was no copyright line found in the file')
        return False
    if changed:
        print('Updating file: ' + filename + '...')
        with open(filename, "wb") as f:
            f.write(str.encode(os.linesep).join(lines))
            f.write(str.encode(os.linesep))
    return True

def update_rc_files(directory, year=datetime.date.today().year):
    if not os.path.isdir(directory):
        return None
    dlg_about = os.path.join(directory, "clientgui", "DlgAbout.cpp")
    for root, dirs, files in os.walk(directory, topdown=True):
        dirs[:] = [d for d in dirs if os.path.join(root, d) not in exclude_dirs]
        for filename in files:
            src_file = os.path.join(root, filename)
            if src_file == dlg_about:
                if not update_dlgabout(os.path.join(root, filename)):
                    return False
                continue
            if (pathlib.Path(filename).suffix != '.rc'):
                continue
            if (os.path.islink(src_file)):
                continue
            with open(src_file, "rb") as f:
                print('Checking file: ' + src_file)
                lines = f.read().splitlines()
            count = 0
            changed = False
            for line in lines:
                count += 1
                match = re.search(b'VALUE \"LegalCopyright\", \".*\d{4}-\d{4}.*\"', line)
                if match:
                    updated_line = re.sub(b'-(\d{4})', b'-' + str.encode(str(year)), line)
                    if updated_line != line:
                        lines[count-1] = updated_line
                        changed = True
            if changed:
                print('Updating file: ' + src_file + '...')
                with open(src_file, "wb") as f:
                    f.write(str.encode(os.linesep).join(lines))
                    f.write(str.encode(os.linesep))
    return True


if (len(sys.argv) < 2):
    help()
    sys.exit(1)

directory = sys.argv[1]

exclude_dirs = [
    os.path.join(directory, "3rdParty")
]

if not update_rc_files(directory):
    sys.exit(1)

sys.exit(0)
