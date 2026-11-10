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

class TestSet:
    def __init__(self, name):
        self.name = name
        self.passed = 0
        self.failed = 0
        print(f"Running test set [{self.name}]...")
        self.testset_start = datetime.datetime.now()

    def __del__(self):
        testset_end = datetime.datetime.now()
        str = f"Test set [{self.name}]: passed [{self.passed}], failed [{self.failed}]. ({testset_end - self.testset_start})"
        if self.failed == 0:
            self._print_success(str)
        else:
            self._print_failure(str)

    def _print_success_report(self, test_name):
        self.passed += 1
        self._print_success(f"Passed [{test_name}]")

    def _print_success(self, message):
        print(f"\033[92m{message}\033[0m")

    def _print_failure(self, message):
        print(f"\033[91m{message}\033[0m")

    def result(self):
        return self.failed == 0

    def expect_true(self, condition, test_name):
        if not condition:
            self.failed += 1
            self._print_failure(f"Failed [{test_name}]: expected True, got False")
            return False
        self._print_success_report(test_name)
        return True

    def expect_false(self, condition, test_name):
        if condition:
            self.failed += 1
            self._print_failure(f"Failed [{test_name}]: expected False, got True")
            return False
        self._print_success_report(test_name)
        return True

    def expect_equal(self, expected, actual, test_name):
        if expected != actual:
            self.failed += 1
            self._print_failure(f"Failed [{test_name}]: expected [{expected}], got [{actual}]")
            return False
        self._print_success_report(test_name)
        return True

    def expect_not_equal(self, a, b, test_name):
        if a == b:
            self.failed += 1
            self._print_failure(f"Failed [{test_name}]: expected {a} != {b}")
            return False
        self._print_success_report(test_name)
