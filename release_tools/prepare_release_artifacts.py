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
import subprocess
import sys
import zipfile

def rename_file(directory, old_name, new_name):
    os.rename(f"{directory}/{old_name}", f"{directory}/{new_name}")
    print(f"Renamed {old_name} to {new_name}")

def extract_zip(zip_path, extract_to):
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_to)
    print(f"Extracted {zip_path} to {extract_to}")

def extract_7z(archive_path, target_file, extract_to, sevenzip_path):
    command = [sevenzip_path, 'e', archive_path, f'-o{extract_to}', target_file]
    subprocess.run(command, check=True)
    print(f"Extracted {archive_path} to {extract_to}")

def rename_pdb(directory, filename):
    arch = filename.split('_')[2]
    arch = 'x86_64' if arch == 'x64' else arch
    rename_file(directory, filename, f"boinc_{version}_windows_{arch}.pdb.zip")

def rename_installer(directory, filename, sevenzip_path):
    arch = filename.split('_')[2]
    arch = 'x86_64' if arch == 'x64' else arch
    target_file = 'installer_setup.exe'
    extract_zip(f"{directory}/{filename}", directory)
    extract_7z(f"{directory}/win_installer.7z", target_file, directory, sevenzip_path)
    rename_file(directory, target_file, f"boinc_{version}_windows_{arch}.exe")

def rename_linux_package(directory, filename):
    codename = filename.split('_')[3]
    if codename.startswith('suse'):
        codename = codename + "_" + filename.split('_')[4]
    package_name = zipfile.ZipFile(f"{directory}/{filename}").filelist[0].filename
    extract_zip(f"{directory}/{filename}", directory)
    new_package_name = package_name.replace(".deb", f"_{codename}.deb") if package_name.endswith(".deb") else package_name.replace(".rpm", f"_{codename}.rpm")
    rename_file(directory, package_name, new_package_name)

def rename_android(directory, filename):
    apk_name = zipfile.ZipFile(f"{directory}/{filename}").filelist[0].filename
    extract_zip(f"{directory}/{filename}", directory)
    arch = "" if filename.split('_')[1] == "manager" else f"_{filename.split('_')[1]}"
    rename_file(directory, apk_name, f"boinc_{version}{arch}.apk")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: prepare_release_artifacts.py <directory> <version> <7z_path>")
        sys.exit(1)

    directory = sys.argv[1]
    version = sys.argv[2]
    sevenzip_path = sys.argv[3]

    print(f"Preparing release artifacts for version {version} in directory {directory}...")

    files = os.listdir(directory)
    for filename in files:
        file_path = os.path.join(directory, filename)
        if os.path.isfile(file_path):
            print(f"Processing file: {filename}")
            if filename.startswith("win_pdb_"):
                rename_pdb(directory, filename)
            elif filename.startswith("win_installer_"):
                rename_installer(directory, filename, sevenzip_path)
                os.remove(f"{directory}/win_installer.7z")
                os.remove(file_path)
            elif filename.startswith("linux-package_"):
                rename_linux_package(directory, filename)
                os.remove(file_path)
            elif filename.startswith("android_"):
                rename_android(directory, filename)
                os.remove(file_path)
            else:
                print(f"Skipping unrecognized file: {filename}")

    print("Release artifacts prepared successfully.")
