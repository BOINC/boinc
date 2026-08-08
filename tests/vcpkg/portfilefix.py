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

ref = sys.argv[1]
hash = sys.argv[2]
if len(sys.argv) != 3:
    print("Usage: portfilefix.py <ref> <hash>")
    sys.exit(1)

with (open("tests/vcpkg/ports/boinc/portfile.cmake", "r") as f):
    lines = f.readlines()

ref_replaced = False
hash_replaced = False

# Fix the portfile
for i, line in enumerate(lines):
    if line.startswith("    REF "):
        lines[i] = f"    REF \"{ref}\"\n"
        ref_replaced = True
    if line.startswith("    SHA512 "):
        lines[i] = f"    SHA512 {hash}\n"
        hash_replaced = True

if not ref_replaced:
    print(f"Error: REF {ref} not found in portfile.cmake")
    sys.exit(1)
if not hash_replaced:
    print(f"Error: SHA512 {hash} not found in portfile.cmake")
    sys.exit(1)

with (open("tests/vcpkg/ports/boinc/portfile.cmake", "w") as f):
    f.writelines(lines)

print(f"Updated portfile.cmake with REF {ref} and SHA512 {hash}")
