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
import json

def main():
    if len(sys.argv) != 5:
        print("Usage: create_signing_metadata.py <output_file> <endpoint> <code_signing_account_name> <certificate_profile_name>")
        sys.exit(1)

    output_file = sys.argv[1]
    metadata = {
        "Endpoint": sys.argv[2],
        "CodeSigningAccountName": sys.argv[3],
        "CertificateProfileName": sys.argv[4]
    }

    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(metadata, f, ensure_ascii=False, indent=4)

if __name__ == "__main__":
    main()
