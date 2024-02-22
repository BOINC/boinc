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

import os
import pathlib
import re
import sys
import testset

class IntegrationTests:
    def __init__(self):
        self.result = True
        self.result &= self.test_files_exist()
        self.result &= self.test_version()
        self.result &= self.test_user()
        self.result &= self.test_selected_values_from_boinc_client_service_file()

    def _get_test_executable_file_path(self, filename):
        path = os.popen("echo $PATH").read().strip()
        for p in path.split(":"):
            if os.path.exists(os.path.join(p, filename)):
                return os.path.join(p, filename)
        return ""

    def _get_current_version_number(self):
        with open("version.h", "r") as f:
            lines = f.readlines()
            for line in lines:
                res = re.search("#define BOINC_VERSION_STRING \"([\d]+.[\d]+.[\d]+)\"", line)
                if res is not None:
                    return res[1]
        return ""

    def _get_version_from_string(self, string):
        res = re.search("([\d]+.[\d]+.[\d]+)", string)
        if res is not None:
            return res[1]
        return ""

    def _get_file_version(self, filename):
        return self._get_version_from_string(os.popen(("{app} --version").format(app=self._get_test_executable_file_path(filename))).read().strip())

    def _get_user_exists(self, username):
        return os.popen("id -un {username}".format(username=username)).read().strip() == username

    def _get_group_exists(self, groupname):
        return os.popen("getent group {groupname}".format(groupname=groupname)).read().strip() != ""

    def _get_user_in_group(self, username, groupname):
        return os.popen("id -Gn {username}".format(username=username)).read().strip().find(groupname) != -1

    def _get_key_value_pairs_from_file(self, filename):
        result = {}
        with open(filename, "r") as f:
            lines = f.readlines()
            for line in lines:
                if (line.find("=") != -1):
                    key, value = line.split("=")
                    result[key.strip()] = value.strip()
        return result

    def _get_file_owner(self, filename):
        if os.path.islink(filename):
            path = pathlib.Path(os.readlink(filename))
        else:
            path = pathlib.Path(filename)
        return "{owner}:{group}".format(owner=path.owner(), group=path.group())

    def _get_ca_certificates_file_path(self):
        if os.path.exists("/etc/ssl/certs/ca-certificates.crt"):
            return "/etc/ssl/certs/ca-certificates.crt"
        if os.path.exists("/etc/pki/tls/certs/ca-bundle.crt"):
            return "/etc/pki/tls/certs/ca-bundle.crt"
        if os.path.exists("/etc/ssl/ca-bundle.pem"):
            return "/etc/ssl/ca-bundle.pem"
        return ""

    def test_files_exist(self):
        ts = testset.TestSet("Test files exist")
        ts.expect_equal("/usr/local/bin/boinc", self._get_test_executable_file_path("boinc"), "Test 'boinc' file location")
        ts.expect_equal("/usr/local/bin/boinccmd", self._get_test_executable_file_path("boinccmd"), "Test 'boinccmd' file location")
        ts.expect_equal("/usr/local/bin/boincmgr", self._get_test_executable_file_path("boincmgr"), "Test 'boincmgr' file location")
        ts.expect_true(os.path.exists("/usr/lib/systemd/system/boinc-client.service"), "Test 'boinc-client.service' file exists in '/usr/lib/systemd/system/'")
        ts.expect_true(os.path.exists("/etc/default/boinc-client"), "Test 'boinc-client' file exists in '/etc/default/'")
        ts.expect_true(os.path.exists("/etc/init.d/boinc-client"), "Test 'boinc-client' file exists in '/etc/init.d/'")
        ts.expect_true(os.path.exists("/etc/bash_completion.d/boinc.bash"), "Test 'boinc.bash' file exists in '/etc/bash_completion.d/'")
        ts.expect_true(os.path.exists("/etc/X11/Xsession.d/36x11-common_xhost-boinc"), "Test '36x11-common_xhost-boinc' file exists in '/etc/X11/Xsession.d/'")
        ts.expect_true(os.path.exists("/usr/share/applications/boinc.desktop"), "Test 'boinc.desktop' file exists in '/usr/share/applications/'")
        ts.expect_true(os.path.exists("/var/lib/boinc/cc_config.xml"), "Test 'cc_config.xml' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/etc/boinc-client/cc_config.xml"), "Test '/etc/boinc-client/cc_config.xml' file is a symbolic link")
        ts.expect_equal("/var/lib/boinc/cc_config.xml", os.readlink("/etc/boinc-client/cc_config.xml"), "Test '/etc/boinc-client/cc_config.xml' file is a symbolic link to '/var/lib/boinc/cc_config.xml'")
        ts.expect_true(os.path.exists("/var/lib/boinc/global_prefs_override.xml"), "Test 'global_prefs_override.xml' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/etc/boinc-client/global_prefs_override.xml"), "Test '/etc/boinc-client/global_prefs_override.xml' file is a symbolic link")
        ts.expect_equal("/var/lib/boinc/global_prefs_override.xml", os.readlink("/etc/boinc-client/global_prefs_override.xml"), "Test '/etc/boinc-client/global_prefs_override.xml' file is a symbolic link to '/var/lib/boinc/global_prefs_override.xml'")
        ts.expect_true(os.path.exists("/var/lib/boinc/remote_hosts.cfg"), "Test 'remote_hosts.cfg' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/etc/boinc-client/remote_hosts.cfg"), "Test '/etc/boinc-client/remote_hosts.cfg' file is a symbolic link")
        ts.expect_equal("/var/lib/boinc/remote_hosts.cfg", os.readlink("/etc/boinc-client/remote_hosts.cfg"), "Test '/etc/boinc-client/remote_hosts.cfg' file is a symbolic link to '/var/lib/boinc/remote_hosts.cfg'")
        ts.expect_true(os.path.exists("/var/lib/boinc/gui_rpc_auth.cfg"), "Test 'gui_rpc_auth.cfg' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/etc/boinc-client/gui_rpc_auth.cfg"), "Test '/etc/boinc-client/gui_rpc_auth.cfg' file is a symbolic link")
        ts.expect_equal("/var/lib/boinc/gui_rpc_auth.cfg", os.readlink("/etc/boinc-client/gui_rpc_auth.cfg"), "Test '/etc/boinc-client/gui_rpc_auth.cfg' file is a symbolic link to '/var/lib/boinc/gui_rpc_auth.cfg'")
        ts.expect_not_equal("", self._get_ca_certificates_file_path(), "Test system 'ca-certificates.crt' file exists")
        ts.expect_true(os.path.exists("/var/lib/boinc/ca-bundle.crt"), "Test 'ca-bundle.crt' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/var/lib/boinc/ca-bundle.crt"), "Test '/var/lib/boinc/ca-bundle.crt' file is a symbolic link")
        ts.expect_equal(self._get_ca_certificates_file_path(), os.readlink("/var/lib/boinc/ca-bundle.crt"), "Test '/var/lib/boinc/ca-bundle.crt' file is a symbolic link to the system 'ca-certificates.crt' file")
        return ts.result()

    def test_version(self):
        ts = testset.TestSet("Test version is correct")
        current_version = self._get_current_version_number()
        ts.expect_not_equal("", current_version, "Test current version could be read from the 'version.h' file")
        ts.expect_equal(current_version, self._get_file_version("boinc"), "Test 'boinc' version is correctly set")
        ts.expect_equal(current_version, self._get_file_version("boinccmd"), "Test 'boinccmd' version is correctly set")
        return ts.result()

    def test_user(self):
        ts = testset.TestSet("Test 'boinc' user and 'boinc' group exist")
        ts.expect_true(self._get_user_exists("boinc"), "Test 'boinc' user exists")
        ts.expect_true(self._get_group_exists("boinc"), "Test 'boinc' group exists")
        ts.expect_true(self._get_user_in_group("boinc", "boinc"), "Test 'boinc' user is in 'boinc' group")
        if (self._get_group_exists("video")):
            ts.expect_true(self._get_user_in_group("boinc", "video"), "Test 'boinc' user is in 'video' group")
        if (self._get_group_exists("render")):
            ts.expect_true(self._get_user_in_group("boinc", "render"), "Test 'boinc' user is in 'render' group")
        return ts.result()

    def test_selected_values_from_boinc_client_service_file(self):
        ts = testset.TestSet("Test selected values from the '/usr/lib/systemd/system/boinc-client.service' file")
        data = self._get_key_value_pairs_from_file("/usr/lib/systemd/system/boinc-client.service")
        ts.expect_equal(data["ReadWritePaths"], "-/var/lib/boinc -/etc/boinc-client", "Test 'ReadWritePaths' is correctly set")
        ts.expect_equal(data["User"], "boinc", "Test 'User' is correctly set")
        ts.expect_equal(data["WorkingDirectory"], "/var/lib/boinc", "Test 'WorkingDirectory' is correctly set")
        ts.expect_equal(data["ExecStart"], "/usr/local/bin/boinc", "Test 'ExecStart' is correctly set")
        ts.expect_equal(data["ExecStop"], "/usr/local/bin/boinccmd --quit", "Test 'ExecStop' is correctly set")
        ts.expect_equal(data["ExecReload"], "/usr/local/bin/boinccmd --read_cc_config", "Test 'ExecReload' is correctly set")
        ts.expect_equal(data["ExecStopPost"], "/bin/rm -f lockfile", "Test 'ExecStopPost' is correctly set")
        return ts.result()

    def test_files_permissions(self):
        ts = testset.TestSet("Test files permissions")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/cc_config.xml"), "Test '/var/lib/boinc/cc_config.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/global_prefs_override.xml"), "Test '/var/lib/boinc/global_prefs_override.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/remote_hosts.cfg"), "Test '/var/lib/boinc/remote_hosts.cfg' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/gui_rpc_auth.cfg"), "Test '/var/lib/boinc/gui_rpc_auth.cfg' file owner")

if __name__ == "__main__":
    if not IntegrationTests().result:
        sys.exit(1)
    sys.exit(0)
