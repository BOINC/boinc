# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2024 University of California
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

class TestSet:
    def __init__(self, name):
        self.name = name
        self.passed = 0
        self.failed = 0
        print(("Running test set [{name}]...").format(name=self.name))

    def __del__(self):
        print(("Test set [{name}]: \033[92mpassed [{passed}]\033[0m, \033[91mfailed [{failed}]\033[0m.").format(name=self.name, passed=self.passed, failed=self.failed))

    def _print_success_report(self, test_name):
        self.passed += 1
        self._print_success(("Passed [{test_name}]").format(test_name=test_name))

    def _print_success(self, message):
        print(("\033[92m{message}\033[0m").format(message=message))

    def _print_failure(self, message):
        print(("\033[91m{message}\033[0m").format(message=message))

    def result(self):
        return self.failed == 0

    def expect_true(self, condition, test_name):
        if not condition:
            self.failed += 1
            self._print_failure(("Failed [{test_name}]: expected True, got False").format(test_name=test_name))
            return False
        self._print_success_report(test_name)
        return True

    def expect_false(self, condition, test_name):
        if condition:
            self.failed += 1
            self._print_failure(("Failed [{test_name}]: expected False, got True").format(test_name=test_name))
            return False
        self._print_success_report(test_name)
        return True

    def expect_equal(self, expected, actual, test_name):
        if expected != actual:
            self.failed += 1
            self._print_failure(("Failed [{test_name}]: expected [{expected}], got [{actual}]").format(test_name=test_name, expected=expected, actual=actual))
            return False
        self._print_success_report(test_name)
        return True

    def expect_not_equal(self, a, b, test_name):
        if a == b:
            self.failed += 1
            self._print_failure(("Failed [{test_name}]: expected {a} != {b}").format(test_name=test_name, a=a, b=b))
            return False
        self._print_success_report(test_name)
