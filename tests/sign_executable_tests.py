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

import filecmp
import os
import subprocess
import sys
import testset

class IntegrationTests:
    def __init__(self, sign_executable, crypt_prog):
        self.sign_executable = sign_executable
        self.crypt_prog = crypt_prog
        self.result = True
        self.result &= self.test_sign_executable()
        self.result &= self.test_sign_executable_with_invalid_key()
        self.result &= self.test_sign_executable_with_invalid_file()

    def _clean_up(self, files):
        for file in files:
            if os.path.exists(file):
                os.remove(file)

    def _run_app(self, app, args):
        proc = subprocess.Popen((app + " " + args).split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        result, err = proc.communicate()
        exit_code = proc.wait()
        return result, err, exit_code

    def _genkey(self, bits, private, public):
        _, _, exit_code = self._run_app(self.crypt_prog, "-genkey " + bits + " " + private + " " + public)
        return exit_code == 0

    def _sign_executable(self, file, private):
        signature, _, exit_code = self._run_app(self.sign_executable, file + " " + private)
        return exit_code == 0, signature

    def _verify_file(self, file, signature, public):
        _, _, exit_code = self._run_app(self.crypt_prog, "-verify " + file + " " + signature + " " + public)
        return exit_code == 0

    def test_sign_executable(self):
        ts = testset.TestSet("Test sign_executable")

        file_to_sign = sys.argv[0]  # Use the current script as the file to sign
        ts.expect_true(self._genkey("1024", "private.key", "public.key"), "Key pair generated successfully")
        result, signature = self._sign_executable(file_to_sign, "private.key")
        ts.expect_true(result, "Executable signed successfully")
        with open("file_to_sign.sig", "wb") as sig_file:
            sig_file.write(signature)
        ts.expect_true(self._verify_file(file_to_sign, "file_to_sign.sig", "public.key"), "Signature verified successfully")
        self._clean_up(["private.key", "public.key", "file_to_sign.sig"])

        return ts.result()

    def test_sign_executable_with_invalid_key(self):
        ts = testset.TestSet("Test sign_executable with invalid key")

        file_to_sign = sys.argv[0]  # Use the current script as the file to sign
        ts.expect_true(self._genkey("1024", "private.key", "public.key"), "Key pair generated successfully")
        result, signature = self._sign_executable(file_to_sign, "invalid_private.key")
        ts.expect_false(result, "Signing with invalid key should fail")
        self._clean_up(["private.key", "public.key"])

        return ts.result()

    def test_sign_executable_with_invalid_file(self):
        ts = testset.TestSet("Test sign_executable with invalid file")

        ts.expect_true(self._genkey("1024", "private.key", "public.key"), "Key pair generated successfully")
        result, signature = self._sign_executable("non_existent_file", "private.key")
        ts.expect_false(result, "Signing a non-existent file should fail")
        self._clean_up(["private.key", "public.key"])

        return ts.result()

if __name__ == "__main__":
    if len(sys.argv) == 3:
        sign_executable = sys.argv[1]
        crypt_prog = sys.argv[2]
    else:
        print("Usage: python sign_executable_tests.py <sign_executable> <crypt_prog>")
        sys.exit(1)
    if not IntegrationTests(sign_executable, crypt_prog).result:
        sys.exit(1)
    sys.exit(0)
