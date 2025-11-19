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
    print('python trailing_whitespaces_check.py <dir>')

def check(directory, exclude_dirs, exclude_extensions, exclude_files, fix_errors=False):
    if not os.path.isdir(directory):
        return None

    files_with_errors = []

    for root, dirs, files in os.walk(directory, topdown=True):
        dirs[:] = [d for d in dirs if os.path.join(root, d) not in exclude_dirs]
        for filename in files:
            if (pathlib.Path(filename).suffix in exclude_extensions):
                continue
            src_file = os.path.join(root, filename)
            if (os.path.islink(src_file) or src_file in exclude_files or filename in exclude_files):
                continue
            with open(src_file, "rb") as f:
                if verbose:
                    print('Checking file: ' + src_file)
                lines = f.read().splitlines()
            count = 0
            changed = False
            for line in lines:
                count += 1
                if line.rstrip() != line:
                    if fix_errors:
                        lines[count - 1] = line.rstrip()
                        changed = True
                    else:
                        files_with_errors.append("{}:{}".format(src_file, count))
            if changed:
                print('Fixing file: ' + src_file + '...')
                with open(src_file, "wb") as f:
                    f.write(str.encode(os.linesep).join(lines))
                    f.write(str.encode(os.linesep))
    return files_with_errors


if (len(sys.argv) < 2):
    help()
    sys.exit(1)

verbose = False
fix = False
directory = sys.argv[1]

for arg in sys.argv:
    if arg == "--verbose":
        verbose = True
        continue
    if arg == "--fix":
        fix = True
        continue

exclude_dirs = [
    os.path.join(directory, ".git"),
    os.path.join(directory, ".github", "ISSUE_TEMPLATE"),
    os.path.join(directory, "3rdParty", "android"),
    os.path.join(directory, "3rdParty", "buildCache"),
    os.path.join(directory, "3rdParty", "linux"),
    os.path.join(directory, "3rdParty", "Windows"),
    os.path.join(directory, "3rdParty", "vcpkg_ports", "ports"),
    os.path.join(directory, "android", "BOINC", ".gradle"),
    os.path.join(directory, "android", "BOINC", ".idea"),
    os.path.join(directory, "android", "BOINC", "app", "build"),
    os.path.join(directory, "android", "BOINC", "app", "src", "main", "assets"),
    os.path.join(directory, "android", "BOINC", "app", "src", "main", "res"),
    os.path.join(directory, "api", "ttf"),
    os.path.join(directory, "coprocs", "NVIDIA", "include"),
    os.path.join(directory, "doc", "manpages"),
    os.path.join(directory, "drupal", "sites", "all", "libraries"),
    os.path.join(directory, "drupal", "sites", "all", "themes"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "modules", "contrib"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "modules", "boincstats", "includes", "pchart"),
    os.path.join(directory, "drupal", "sites", "default", "boinc", "themes", "boinc", "webfonts"),
    os.path.join(directory, "html", "inc", "password_compat"),
    os.path.join(directory, "html", "inc", "random_compat"),
    os.path.join(directory, "html", "inc", "ReCaptcha"),
    os.path.join(directory, "locale"),
    os.path.join(directory, "mac_build", "build"),
    os.path.join(directory, "mac_build", "boinc.xcodeproj", "project.xcworkspace"),
    os.path.join(directory, "samples", "example_app", "bin"),
    os.path.join(directory, "samples", "gfx_html"),
    os.path.join(directory, "samples", "glut"),
    os.path.join(directory, "samples", "image_libs"),
    os.path.join(directory, "samples", "jpeglib"),
    os.path.join(directory, "samples", "mac_build", "build"),
    os.path.join(directory, "samples", "mac_build", "UpperCase2.xcodeproj", "project.xcworkspace"),
    os.path.join(directory, "samples", "vboxwrapper", "build"),
    os.path.join(directory, "samples", "vboxwrapper", "vboxwrapper.xcodeproj", "project.xcworkspace"),
    os.path.join(directory, "tests", "old"),
    os.path.join(directory, "win_build", "Build"),
    os.path.join(directory, "win_build", ".vs"),
    os.path.join(directory, "win_build", "ipch"),
    os.path.join(directory, "win_build", "installerv2", "redist"),
    os.path.join(directory, "zip", "unzip"),
    os.path.join(directory, "zip", "zip"),
    os.path.join(directory, "zip", "boinc_zip.xcodeproj", "project.xcworkspace"),
]

exclude_files = [
    ".DS_Store",
    os.path.join(directory, "bolt_checkin_notes.txt"),
    os.path.join(directory, "checkin_notes"),
    os.path.join(directory, "checkin_notes_2004"),
    os.path.join(directory, "checkin_notes_2005"),
    os.path.join(directory, "checkin_notes_2006"),
    os.path.join(directory, "checkin_notes_2007"),
    os.path.join(directory, "checkin_notes_2008"),
    os.path.join(directory, "checkin_notes_2009"),
    os.path.join(directory, "checkin_notes_2010"),
    os.path.join(directory, "checkin_notes_2011"),
    os.path.join(directory, "checkin_notes_2012"),
    os.path.join(directory, "checkin_notes_samples"),
    os.path.join(directory, "checkin_notes_samples"),
    os.path.join(directory, "todo"),
    os.path.join(directory, "TODO_OLD"),
    os.path.join(directory, "aclocal.m4"),
    os.path.join(directory, "api", "MultiGPUMig.h"),
    os.path.join(directory, "api", "MultiGPUMigServer.c"),
    os.path.join(directory, "api", "MultiGPUMigServer.h"),
    os.path.join(directory, "api", "MultiGPUMigUser.c"),
    os.path.join(directory, "client", "boinc_client"),
    os.path.join(directory, "client", "boinccmd"),
    os.path.join(directory, "client", "boinc"),
    os.path.join(directory, "client", "switcher"),
    os.path.join(directory, "clientscr", "progress", "simt"),
    os.path.join(directory, "html", "inc", "GeoIP.dat"),
    os.path.join(directory, "html", "inc", "geoip.inc"),
    os.path.join(directory, "html", "inc", "htmLawed.php"),
    os.path.join(directory, "html", "inc", "recaptcha_loader.php"),
    os.path.join(directory, "samples", "vboxwrapper", "x86_64", "vboxwrapper"),
    os.path.join(directory, "samples", "wrapper", "regerror.c"),
    os.path.join(directory, "samples", "wrapper", "regexp_custom.h"),
    os.path.join(directory, "samples", "wrapper", "regexp_int.h"),
    os.path.join(directory, "samples", "wrapper", "regexp_memory.c"),
    os.path.join(directory, "samples", "wrapper", "regexp_report.c"),
    os.path.join(directory, "samples", "wrapper", "regexp.c"),
    os.path.join(directory, "samples", "wrapper", "regexp.h"),
    os.path.join(directory, "samples", "wrapper", "regmagic.h"),
    os.path.join(directory, "samples", "wrapper", "regsub.c"),
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
    ".DS_Store",
    ".exe",
    ".gch",
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
    ".pdb",
    ".pdf",
    ".pdn",
    ".png",
    ".po",
    ".psd",
    ".rgb",
    ".rtf",
    ".so",
    ".tif",
    ".tiff",
    ".tlb",
    ".ttf",
    ".zip",
]

files_with_errors = check(directory, exclude_dirs, exclude_extensions, exclude_files, fix)

if files_with_errors:
    print('Found files with errors:')
    for file in files_with_errors:
        print(file)
    print('To fix errors run locally:\n\tpython3 ci_tools/trailing_whitespaces_check.py . --fix')
    sys.exit(1)

sys.exit(0)
