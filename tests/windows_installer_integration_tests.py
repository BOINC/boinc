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
import testset as testset
import testhelper as testhelper

class IntegrationTests:
    def __init__(self):
        self.testhelper = testhelper.TestHelper()
        self.result = True
        self.result &= self.test_version()
        self.result &= self.test_files_exist()

    def _get_test_executable_file_path(self, filename):
        return pathlib.Path("C:\\Program Files\\BOINC\\") / filename

    def _get_test_data_file_path(self, filename):
        return pathlib.Path("C:\\ProgramData\\BOINC\\") / filename

    def _get_file_version(self, filename):
        return self.testhelper.get_version_from_string(os.popen(("\"{app}\" --version").format(app=self._get_test_executable_file_path(filename))).read().strip())

    def test_version(self):
        ts = testset.TestSet("Test version is correct")
        current_version = self.testhelper.get_current_version_number()
        ts.expect_not_equal("", current_version, "Test current version could be read from the 'version.h' file")
        ts.expect_equal(current_version, self._get_file_version("boinc.exe"), "Test 'boinc.exe' version is correctly set")
        ts.expect_equal(current_version, self._get_file_version("boinccmd.exe"), "Test 'boinccmd.exe' version is correctly set")
        return ts.result()

    def test_files_exist(self):
        ts = testset.TestSet("Test files exist")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinc.exe")), "Test 'boinc.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinccmd.exe")), "Test 'boinccmd.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boincmgr.exe")), "Test 'boincmgr.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boincscr.exe")), "Test 'boincscr.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boincsvcctrl.exe")), "Test 'boincsvcctrl.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinctray.exe")), "Test 'boinctray.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_data_file_path("all_projects_list.xml")), "Test 'all_projects_list.xml' file exists in 'C:\\ProgramData\\BOINC\\'")
        return ts.result()

if __name__ == "__main__":
    if not IntegrationTests().result:
        sys.exit(1)
    sys.exit(0)
