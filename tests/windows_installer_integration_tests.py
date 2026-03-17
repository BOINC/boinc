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

import argparse
import os
import pathlib
import subprocess
import sys
import winreg
import xml.etree.ElementTree as ET
import testset as testset
import testhelper as testhelper

class IntegrationTests:
    def __init__(self, installation_type="normal", test_type="install"):
        self.testhelper = testhelper.TestHelper()
        self.installation_type = installation_type
        self.test_type = test_type
        self.result = True
        self.result &= self.test_version()
        self.result &= self.test_files_exist()
        self.result &= self.test_registry_records_exist()
        self.result &= self.test_login_token_file()
        if self.installation_type == "service":
            self.result &= self.test_service_exists()
            self.result &= self.test_service_users_exist()
            self.result &= self.test_service_groups_and_memberships()
        if self.installation_type == "test_acct_mgr_login":
            self.result &= self.test_acct_mgr_login()
        if self.installation_type == "test_client_auth_file":
            self.result &= self.test_client_auth_file()
            if self.test_type == "install":
                self.result &= self.test_boinc_master_user_only_exists()
        if self.installation_type == "test_proj_init_file":
            self.result &= self.test_proj_init_file()

    def _get_test_executable_file_path(self, filename):
        return pathlib.Path("C:\\Program Files\\BOINC\\") / filename

    def _get_test_data_file_path(self, filename):
        return pathlib.Path("C:\\ProgramData\\BOINC\\") / filename

    def _get_file_version(self, filename):
        return self.testhelper.get_version_from_string(os.popen(("\"{app}\" --version").format(app=self._get_test_executable_file_path(filename))).read().strip())

    def _get_value_from_registry(self, key, value):
        try:
            hive, subkey = key.split("\\", 1)
            hives = {
                "HKEY_CLASSES_ROOT": winreg.HKEY_CLASSES_ROOT,
                "HKEY_CURRENT_USER": winreg.HKEY_CURRENT_USER,
                "HKEY_LOCAL_MACHINE": winreg.HKEY_LOCAL_MACHINE,
                "HKEY_USERS": winreg.HKEY_USERS,
                "HKEY_CURRENT_CONFIG": winreg.HKEY_CURRENT_CONFIG
            }
            with winreg.OpenKey(hives[hive], subkey) as key:
                result, _ = winreg.QueryValueEx(key, value)
                return result
        except Exception as e:
            print(f"Error accessing registry: {e}")
            return None

    def _check_service_exists(self, service_name):
        try:
            result = subprocess.run(
                ["sc", "query", service_name],
                capture_output=True,
                text=True,
                check=False
            )
            return result.returncode == 0
        except Exception as e:
            print(f"Error checking service: {e}")
            return False

    def _check_user_exists(self, username):
        try:
            result = subprocess.run(
                ["net", "user", username],
                capture_output=True,
                text=True,
                check=False
            )
            return result.returncode == 0
        except Exception as e:
            print(f"Error checking user: {e}")
            return False

    def _check_group_exists(self, groupname, username=None):
        try:
            result = subprocess.run(
                ["net", "localgroup", groupname],
                capture_output=True,
                text=True,
                check=False
            )
            if username is not None:
                if result.returncode != 0:
                    return False
                return username in result.stdout
            # Just check if the group exists
            return result.returncode == 0
        except Exception as e:
            print(f"Error checking group: {e}")
            return False

    def test_version(self):
        ts = testset.TestSet("Test version is correct")
        current_version = self.testhelper.get_current_version_number()
        ts.expect_not_equal("", current_version, "Test current version could be read from the 'version.h' file")
        ts.expect_equal(current_version, self._get_file_version("boinc.exe"), "Test 'boinc.exe' version is correctly set")
        ts.expect_equal(current_version, self._get_file_version("boinccmd.exe"), "Test 'boinccmd.exe' version is correctly set")
        ts.expect_equal(current_version, self._get_value_from_registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup\\", "UpgradingTo"), "Test 'UpgradingTo' registry record exists and is set correctly")
        return ts.result()

    def test_files_exist(self):
        ts = testset.TestSet("Test files exist")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinc.exe")), "Test 'boinc.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinccmd.exe")), "Test 'boinccmd.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boincmgr.exe")), "Test 'boincmgr.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boincscr.exe")), "Test 'boincscr.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_executable_file_path("boinctray.exe")), "Test 'boinctray.exe' file exists in 'C:\\Program Files\\BOINC\\'")
        ts.expect_true(os.path.exists(self._get_test_data_file_path("all_projects_list.xml")), "Test 'all_projects_list.xml' file exists in 'C:\\ProgramData\\BOINC\\'")
        return ts.result()

    def test_registry_records_exist(self):
        ts = testset.TestSet("Test registry records exist")
        ts.expect_equal("C:\\ProgramData\\BOINC\\", self._get_value_from_registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup\\", "DATADIR"), "Test 'DATADIR' registry record exists and is set correctly")
        ts.expect_equal("C:\\Program Files\\BOINC\\", self._get_value_from_registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup\\", "INSTALLDIR"), "Test 'INSTALLDIR' registry record exists and is set correctly")
        return ts.result()

    def test_service_exists(self):
        ts = testset.TestSet("Test BOINC service exists")
        ts.expect_true(self._check_service_exists("boinc"), "Test 'boinc' service exists")
        return ts.result()

    def test_service_users_exist(self):
        ts = testset.TestSet("Test BOINC service users exist")
        ts.expect_true(self._check_user_exists("boinc_master"), "Test 'boinc_master' user exists")
        ts.expect_true(self._check_user_exists("boinc_project"), "Test 'boinc_project' user exists")
        return ts.result()

    def test_boinc_master_user_only_exists(self):
        ts = testset.TestSet("Test 'boinc_master' user only exists")
        ts.expect_true(self._check_user_exists("boinc_master"), "Test 'boinc_master' user exists")
        ts.expect_false(self._check_user_exists("boinc_project"), "Test 'boinc_project' user does not exist")
        return ts.result()

    def test_service_groups_and_memberships(self):
        ts = testset.TestSet("Test BOINC service groups and memberships")
        ts.expect_true(self._check_group_exists("boinc_admins"), "Test 'boinc_admins' group exists")
        ts.expect_true(self._check_group_exists("boinc_projects"), "Test 'boinc_projects' group exists")
        ts.expect_true(self._check_group_exists("boinc_users"), "Test 'boinc_users' group exists")
        ts.expect_true(self._check_group_exists("boinc_admins", "boinc_master"), "Test 'boinc_master' user is a member of 'boinc_admins' group")
        ts.expect_true(self._check_group_exists("boinc_projects", "boinc_project"), "Test 'boinc_project' user is a member of 'boinc_projects' group")
        return ts.result()

    def test_acct_mgr_login(self):
        ts = testset.TestSet("Test account manager login file")
        acct_mgr_file = self._get_test_data_file_path("acct_mgr_login.xml")
        ts.expect_true(os.path.exists(acct_mgr_file), "Test 'acct_mgr_login.xml' file exists in 'C:\\ProgramData\\BOINC\\'")

        if os.path.exists(acct_mgr_file):
            try:
                tree = ET.parse(acct_mgr_file)
                root = tree.getroot()

                ts.expect_equal("acct_mgr_login", root.tag, "Test root element is 'acct_mgr_login'")

                login_element = root.find("login")
                password_hash_element = root.find("password_hash")

                ts.expect_true(login_element is not None, "Test 'login' element exists")
                ts.expect_true(password_hash_element is not None, "Test 'password_hash' element exists")

                if login_element is not None:
                    ts.expect_equal("test", login_element.text, "Test 'login' element contains 'test'")

                if password_hash_element is not None:
                    ts.expect_equal("0123456789abcdef", password_hash_element.text, "Test 'password_hash' element contains '0123456789abcdef'")

            except ET.ParseError as e:
                ts.expect_true(False, f"Test XML file is well-formed (Parse error: {e})")
            except Exception as e:
                ts.expect_true(False, f"Test XML file can be processed (Error: {e})")

        return ts.result()

    def test_client_auth_file(self):
        ts = testset.TestSet("Test client auth file")
        client_auth_file = self._get_test_data_file_path("client_auth.xml")
        ts.expect_true(os.path.exists(client_auth_file), "Test 'client_auth.xml' file exists in 'C:\\ProgramData\\BOINC\\'")

        if os.path.exists(client_auth_file):
            try:
                tree = ET.parse(client_auth_file)
                root = tree.getroot()

                ts.expect_equal("client_authorization", root.tag, "Test root element is 'client_authorization'")

                boinc_project_element = root.find("boinc_project")
                ts.expect_true(boinc_project_element is not None, "Test 'boinc_project' element exists")

                if boinc_project_element is not None:
                    username_element = boinc_project_element.find("username")
                    password_element = boinc_project_element.find("password")

                    ts.expect_true(username_element is not None, "Test 'username' element exists")
                    ts.expect_true(password_element is not None, "Test 'password' element exists")

                    if username_element is not None:
                        ts.expect_equal("test_user", username_element.text, "Test 'username' element contains 'test_user'")

                    if password_element is not None:
                        ts.expect_equal("cXdlcnR5MTIzNDU2IUAjJCVe", password_element.text.strip(), "Test 'password' element contains 'cXdlcnR5MTIzNDU2IUAjJCVe'")

            except ET.ParseError as e:
                ts.expect_true(False, f"Test XML file is well-formed (Parse error: {e})")
            except Exception as e:
                ts.expect_true(False, f"Test XML file can be processed (Error: {e})")

        return ts.result()

    def test_login_token_file(self):
        ts = testset.TestSet("Test login token file")
        login_token_file = self._get_test_data_file_path("login_token.txt")
        ts.expect_true(os.path.exists(login_token_file), "Test 'login_token.txt' file exists in 'C:\\ProgramData\\BOINC\\'")

        if os.path.exists(login_token_file):
            try:
                with open(login_token_file, 'r') as f:
                    content = f.read()
                    ts.expect_equal("installer_setup.exe", content.strip(), "Test 'login_token.txt' contains 'installer_setup.exe' line")
            except Exception as e:
                ts.expect_true(False, f"Test 'login_token.txt' can be read (Error: {e})")

        return ts.result()

    def test_proj_init_file(self):
        ts = testset.TestSet("Test project init file")
        project_init_file = self._get_test_data_file_path("project_init.xml")
        ts.expect_true(os.path.exists(project_init_file), "Test 'project_init.xml' file exists in 'C:\\ProgramData\\BOINC\\'")

        if os.path.exists(project_init_file):
            try:
                tree = ET.parse(project_init_file)
                root = tree.getroot()

                ts.expect_equal("project_init", root.tag, "Test root element is 'project_init'")

                url_element = root.find("url")
                name_element = root.find("name")
                account_key_element = root.find("account_key")
                embedded_element = root.find("embedded")

                ts.expect_true(url_element is not None, "Test 'url' element exists")
                ts.expect_true(name_element is not None, "Test 'name' element exists")
                ts.expect_true(account_key_element is not None, "Test 'account_key' element exists")
                ts.expect_true(embedded_element is not None, "Test 'embedded' element exists")

                if url_element is not None:
                    ts.expect_equal("https://test.com", url_element.text, "Test 'url' element contains 'https://test.com'")

                if name_element is not None:
                    ts.expect_equal("test", name_element.text, "Test 'name' element contains 'test'")

                if account_key_element is not None:
                    ts.expect_equal("abcdef1234567890", account_key_element.text, "Test 'account_key' element contains 'abcdef1234567890'")

                if embedded_element is not None:
                    ts.expect_equal("0", embedded_element.text, "Test 'embedded' element contains '0'")

            except ET.ParseError as e:
                ts.expect_true(False, f"Test XML file is well-formed (Parse error: {e})")
            except Exception as e:
                ts.expect_true(False, f"Test XML file can be processed (Error: {e})")

        return ts.result()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="BOINC Windows Installer Integration Tests")
    parser.add_argument(
        "--installation-type",
        type=str,
        default="normal",
        help="Installation type: 'normal' (default), 'service', 'test_acct_mgr_login', 'test_client_auth_file', or 'test_proj_init_file'"
    )
    parser.add_argument(
        "--type",
        type=str,
        default="install",
        help="Test type: 'install' (default) or 'upgrade_from_alpha' or 'upgrade_from_stable'"
    )
    args = parser.parse_args()

    if not IntegrationTests(installation_type=args.installation_type, test_type=args.type).result:
        sys.exit(1)
    sys.exit(0)
