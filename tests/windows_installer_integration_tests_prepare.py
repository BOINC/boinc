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
Windows Installer Integration Tests Preparation Script

This script prepares the Windows environment for installer integration tests.
Currently supports creating test users on Windows.
"""

import argparse
import platform
import subprocess
import sys


def check_windows():
    """Verify that the script is running on Windows."""
    if platform.system() != 'Windows':
        print("Error: This script must be run on Windows.", file=sys.stderr)
        sys.exit(1)


def create_windows_user(username, password):
    """
    Create a new Windows user account.

    Args:
        username: The username for the new account
        password: The password for the new account

    Returns:
        bool: True if user was created successfully, False otherwise
    """
    try:
        # Use net user command to create a new local user account
        cmd = ['net', 'user', username, password, '/add', '/y']
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False
        )

        if result.returncode == 0:
            print(f"Successfully created user: {username}")
            return True
        else:
            error_msg = result.stderr.strip() or result.stdout.strip() or "Unknown error occurred"
            print(f"Error creating user: {error_msg}", file=sys.stderr)
            return False

    except Exception as e:
        print(f"Exception while creating user: {e}", file=sys.stderr)
        return False


def main():
    """Main entry point for the script."""
    parser = argparse.ArgumentParser(
        description='Prepare Windows environment for installer integration tests'
    )

    parser.add_argument(
        '--create-user',
        action='store_true',
        help='Create a new Windows user account'
    )

    parser.add_argument(
        '--username',
        type=str,
        help='Username for the new account (required with --create-user)'
    )

    parser.add_argument(
        '--password',
        type=str,
        help='Password for the new account (required with --create-user)'
    )

    args = parser.parse_args()

    # Check if running on Windows
    check_windows()

    # Validate arguments
    if args.create_user:
        if not args.username or not args.password:
            parser.error('--create-user requires both --username and --password')

        # Create the user
        success = create_windows_user(args.username, args.password)
        sys.exit(0 if success else 1)
    else:
        parser.print_help()
        sys.exit(0)


if __name__ == '__main__':
    main()
