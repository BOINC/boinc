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
import pathlib
import sys

def help():
    print('Usage:')
    print('python source-code-check.py <dir>')

def check(directory, bytes_to_check, exclude_dirs, exclude_extensions, exclude_files):
    if not os.path.isdir(directory):
        return None

    files_with_errors = []

    for root, dirs, files in os.walk(directory, topdown=True):
        dirs[:] = [d for d in dirs if os.path.join(root, d) not in exclude_dirs]
        for filename in files:
            if (pathlib.Path(filename).suffix in exclude_extensions):
                continue
            file = os.path.join(root, filename)
            if (os.path.islink(file) or file in exclude_files):
                continue
            with open(file, "rb") as f:
                print('Checking file: ' + file)
                byte = f.read(1)
                while byte:
                    if byte in bytes_to_check:
                        files_with_errors.append(file)
                        break
                    byte = f.read(1)
    return files_with_errors


if (len(sys.argv) != 2):
    help()
    sys.exit(1)

directory = sys.argv[1]

exclude_dirs = [
    os.path.join(directory, ".git"),
    os.path.join(directory, "3rdParty", "android"),
    os.path.join(directory, "3rdParty", "buildCache"),
    os.path.join(directory, "3rdParty", "linux"),
    os.path.join(directory, "3rdParty", "Windows"),
    os.path.join(directory, "3rdParty", "vcpkg_ports", "ports"),
    os.path.join(directory, "android", "BOINC", ".gradle"),
    os.path.join(directory, "android", "BOINC", "app", "build"),
    os.path.join(directory, "android", "BOINC", "app", "src", "main", "assets"),
    os.path.join(directory, "android", "BOINC", "app", "src", "main", "res"),
    os.path.join(directory, "doc", "manpages"),
    os.path.join(directory, "drupal", "sites", "all", "libraries"),
    os.path.join(directory, "drupal", "sites", "all", "themes"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "modules", "contrib"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "themes"),
    os.path.join(directory, "fastlane"),
    os.path.join(directory, "samples", "example_app", "bin"),
]

exclude_files = [
    os.path.join(directory, "aclocal.m4"),
    os.path.join(directory, "android", "BOINC", "gradlew"),
    os.path.join(directory, "client", "boinc_client"),
    os.path.join(directory, "client", "boinccmd"),
    os.path.join(directory, "client", "boinc"),
    os.path.join(directory, "client", "switcher"),
    os.path.join(directory, "clientgui", "BOINCBaseView.cpp"),
    os.path.join(directory, "clientscr", "progress", "simt"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "modules", "boinc_solr_search", "boinc_solr_comments", "README.txt"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "modules", "boinc_solr_search", "boinc_solr_comments", "INSTALL.txt"),
    os.path.join(directory, "html", "inc", "GeoIP.dat"),
    os.path.join(directory, "mac_installer", "BOINC.pmproj"),
    os.path.join(directory, "mac_build", "boinc.xcodeproj", "project.pbxproj"),
    os.path.join(directory, "stage", "usr", "local", "bin", "boinc_client"),
    os.path.join(directory, "stage", "usr", "local", "bin", "boinccmd"),
    os.path.join(directory, "stage", "usr", "local", "bin", "boinc"),
    os.path.join(directory, "stage", "usr", "local", "bin", "switcher"),
]

exclude_extensions = [
    ".a",
    ".bmp",
    ".cub",
    ".dll",
    ".exe",
    ".gif",
    ".icns",
    ".ico",
    ".jar",
    ".jpg",
    ".lib",
    ".mo",
    ".nib",
    ".o",
    ".odp",
    ".pdf",
    ".pdn",
    ".png",
    ".po",
    ".psd",
    ".rgb",
    ".so",
    ".tif",
    ".tiff",
    ".tlb",
    ".ttf",
    ".zip",
]

files_with_errors = check(directory, [b"\xC2"], exclude_dirs, exclude_extensions, exclude_files)

if files_with_errors:
    print("Found files with errors:")
    for file in files_with_errors:
        print(file)
    sys.exit(1)

sys.exit(0)
