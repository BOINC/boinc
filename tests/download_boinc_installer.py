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

"""
Download BOINC installer for Windows based on platform and version type.

Usage:
    python3 download_boinc_installer.py <platform> <type>

    platform: x64 or arm64
    type: release or alpha

Examples:
    python3 download_boinc_installer.py x64 release
    python3 download_boinc_installer.py arm64 alpha
"""

import sys
import urllib.request
import xml.etree.ElementTree as ET


def download_file(url, filename):
    """Download a file from URL and save it with the given filename."""
    print(f"Downloading {url}...")
    urllib.request.urlretrieve(url, filename)
    print(f"Successfully downloaded to {filename}")


def get_download_url(xml_content, platform, version_type):
    """
    Parse XML and extract download URL based on platform and version type.

    Args:
        xml_content: XML content as string
        platform: 'x64' or 'arm64'
        version_type: 'release' or 'alpha'

    Returns:
        URL string or None if not found
    """
    root = ET.fromstring(xml_content)

    # Map input parameters to XML values
    platform_map = {
        'x64': 'Windows Intel-compatible 64-bit',
        'arm64': 'Windows ARM 64-bit'
    }

    version_map = {
        'release': 'Recommended version',
        'alpha': 'Development version'
    }

    target_platform = platform_map.get(platform)
    target_version = version_map.get(version_type)

    if not target_platform or not target_version:
        print(f"Error: Invalid platform '{platform}' or type '{version_type}'")
        return None

    # Find matching version entry
    for version in root.findall('version'):
        platform_elem = version.find('platform')
        description_elem = version.find('description')
        url_elem = version.find('url')

        if (platform_elem is not None and platform_elem.text == target_platform and
            description_elem is not None and description_elem.text == target_version and
            url_elem is not None):
            return url_elem.text

    return None


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 download_boinc_installer.py <platform> <type>")
        print("  platform: x64 or arm64")
        print("  type: release or alpha")
        sys.exit(1)

    platform = sys.argv[1].lower()
    version_type = sys.argv[2].lower()

    if platform not in ['x64', 'arm64']:
        print(f"Error: Invalid platform '{platform}'. Use 'x64' or 'arm64'")
        sys.exit(1)

    if version_type not in ['release', 'alpha']:
        print(f"Error: Invalid type '{version_type}'. Use 'release' or 'alpha'")
        sys.exit(1)

    xml_url = 'https://boinc.berkeley.edu/download_all.php?xml=1'

    print(f"Fetching version information from {xml_url}...")
    try:
        with urllib.request.urlopen(xml_url) as response:
            xml_content = response.read().decode('utf-8')
    except Exception as e:
        print(f"Error downloading XML: {e}")
        sys.exit(1)

    download_url = get_download_url(xml_content, platform, version_type)

    if not download_url:
        if version_type == 'alpha':
            print(f"Development version for {platform} is not available. This is normal.")
            sys.exit(0)
        else:
            print(f"Error: Could not find download URL for {platform} {version_type}")
            sys.exit(1)

    output_filename = f"{platform}_{version_type}.exe"

    try:
        download_file(download_url, output_filename)
        print(f"Download completed successfully: {output_filename}")
    except Exception as e:
        print(f"Error downloading file: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
