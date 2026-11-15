#!/usr/bin/env python3

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

def update_gl_file(filename, year):
    print('Checking file: ' + filename)
    with open(filename, "rb") as f:
        lines = f.read().splitlines()
    changed = False
    match_found = False
    match = re.search(rb'.*\d{4}.*', lines[2])
    if match:
        match_found = True
        updated_line = re.sub(rb'(\d{4})', str.encode(str(year)), lines[2])
        if updated_line != lines[2]:
            lines[2] = updated_line
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


def update_copyright_file(filename, year):
    print('Checking file: ' + filename)
    with open(filename, "rb") as f:
        lines = f.read().splitlines()
    changed = False
    match_found = False
    match = re.search(rb'.*\d{4}-\d{4}.*', lines[3])
    if match:
        match_found = True
        updated_line = re.sub(rb'-(\d{4})', b'-' + str.encode(str(year)), lines[3])
        if updated_line != lines[3]:
            lines[3] = updated_line
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

def update_dlgabout(filename, year):
    print('Checking file: ' + filename)
    with open(filename, "rb") as f:
        lines = f.read().splitlines()
    count = 0
    changed = False
    match_found = False
    for line in lines:
        count += 1
        match = re.search(rb'.*\(C\)\s\d{4}-\d{4}.*', line)
        if match:
            match_found = True
            updated_line = re.sub(rb'-(\d{4})', b'-' + str.encode(str(year)), line)
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

def update_rc_files(directory, year):
    if not os.path.isdir(directory):
        return None

    exclude_dirs = [
        os.path.join(directory, "3rdParty")
    ]

    for root, dirs, files in os.walk(directory, topdown=True):
        dirs[:] = [d for d in dirs if os.path.join(root, d) not in exclude_dirs]
        for filename in files:
            if (pathlib.Path(filename).suffix != '.rc'):
                continue
            src_file = os.path.join(root, filename)
            if (os.path.islink(src_file)):
                continue
            with open(src_file, "rb") as f:
                print('Checking file: ' + src_file)
                lines = f.read().splitlines()
            count = 0
            changed = False
            for line in lines:
                count += 1
                match = re.search(rb'VALUE \"LegalCopyright\", \".*\d{4}-\d{4}.*\"', line)
                if match:
                    updated_line = re.sub(rb'-(\d{4})', b'-' + str.encode(str(year)), line)
                    if updated_line != line:
                        lines[count-1] = updated_line
                        changed = True
            if changed:
                print('Updating file: ' + src_file + '...')
                with open(src_file, "wb") as f:
                    f.write(str.encode(os.linesep).join(lines))
                    f.write(str.encode(os.linesep))
    return True


if __name__ == "__main__":
    if (len(sys.argv) < 2):
        help()
        sys.exit(1)

    directory = sys.argv[1]
    year = datetime.date.today().year

    if not update_rc_files(directory, year):
        sys.exit(1)
    if not update_dlgabout(os.path.join(directory, "clientgui", "DlgAbout.cpp"), year):
        sys.exit(1)
    if not update_copyright_file(os.path.join(directory, "COPYRIGHT"), year):
        sys.exit(1)
    if not update_gl_file(os.path.join(directory, "gl"), year):
        sys.exit(1)
    if not update_gl_file(os.path.join(directory, "gl.py"), year):
        sys.exit(1)

    sys.exit(0)
