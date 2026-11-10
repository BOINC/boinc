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

import os
import pathlib
import subprocess
import sys
import time
import testset
import testhelper

class IntegrationTests:
    def __init__(self):
        self.testhelper = testhelper.TestHelper()
        self.result = True
        self.result &= self.test_files_exist()
        self.result &= self.test_version()
        self.result &= self.test_user()
        self.result &= self.test_selected_values_from_boinc_client_service_file()
        self.result &= self.test_files_permissions()
        self.result &= self.test_service_running()

    def _resolve_path(self, path):
        path = os.path.abspath(path)
        components = path.split(os.sep)
        current_path = os.sep if path.startswith(os.sep) else components[0]
        symlinks_found = False
        for part in components[1:] if path.startswith(os.sep) else components[1:]:
            current_path = os.path.join(current_path, part)
            if os.path.islink(current_path):
                symlinks_found = True
        if symlinks_found:
            return os.path.realpath(path)
        else:
            return path

    def _get_test_executable_file_path(self, filename):
        path = os.popen("echo $PATH").read().strip()
        for p in path.split(":"):
            if os.path.exists(os.path.join(p, filename)):
                return self._resolve_path(os.path.join(p, filename))
        return ""

    def _get_file_version(self, filename):
        return self.testhelper.get_version_from_string(os.popen(("{app} --version").format(app=self._get_test_executable_file_path(filename))).read().strip())

    def _get_user_exists(self, username):
        return os.popen("id -un {username}".format(username=username)).read().strip() == username

    def _get_group_exists(self, groupname):
        return os.popen("getent group {groupname}".format(groupname=groupname)).read().strip().split(":")[0] == groupname

    def _get_user_in_group(self, username, groupname):
        return os.popen("id -Gn {username}".format(username=username)).read().strip().find(groupname) != -1

    def _get_user_home_directory(self, username):
        return os.popen("getent passwd {username}".format(username=username)).read().strip().split(":")[5]

    def _get_uid_range(self, username):
        result = os.popen("cat /etc/subuid | grep {username}".format(username=username)).read().strip()
        if (result == ""):
            return ""
        result = result.split(":")
        if (len(result) != 3):
            return ""
        start = int(result[1])
        count = int(result[2])
        return "{start}:{count}".format(start=start, count=count)

    def _get_gid_range(self, groupname):
        result = os.popen("cat /etc/subgid | grep {groupname}".format(groupname=groupname)).read().strip()
        if (result == ""):
            return ""
        result = result.split(":")
        if (len(result) != 3):
            return ""
        start = int(result[1])
        count = int(result[2])
        return "{start}:{count}".format(start=start, count=count)

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

    def _get_ca_certificates_file_paths(self):
        paths = []
        if os.path.exists("/etc/ssl/certs/ca-certificates.crt"):
            paths.append("/etc/ssl/certs/ca-certificates.crt")
        if os.path.exists("/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem"):
            paths.append("/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem")
        if os.path.exists("/etc/ssl/ca-bundle.pem"):
            paths.append("/etc/ssl/ca-bundle.pem")
        return paths

    def test_files_exist(self):
        ts = testset.TestSet("Test files exist")
        ts.expect_equal("/usr/local/bin/boinc", self._get_test_executable_file_path("boinc"), "Test 'boinc' file location")
        ts.expect_equal("/usr/local/bin/boinccmd", self._get_test_executable_file_path("boinccmd"), "Test 'boinccmd' file location")
        ts.expect_equal("/usr/local/bin/boincmgr", self._get_test_executable_file_path("boincmgr"), "Test 'boincmgr' file location")
        ts.expect_true(os.path.exists("/usr/lib/systemd/system/boinc-client.service"), "Test 'boinc-client.service' file exists in '/usr/lib/systemd/system/'")
        ts.expect_true(os.path.exists("/etc/default/boinc-client"), "Test 'boinc-client' file exists in '/etc/default/'")
        ts.expect_false(os.path.exists("/etc/init.d/boinc-client"), "Test 'boinc-client' file does not exist in '/etc/init.d/'")
        ts.expect_true(os.path.exists("/etc/bash_completion.d/boinc.bash"), "Test 'boinc.bash' file exists in '/etc/bash_completion.d/'")
        ts.expect_true(os.path.exists("/etc/X11/Xsession.d/36x11-common_xhost-boinc"), "Test '36x11-common_xhost-boinc' file exists in '/etc/X11/Xsession.d/'")
        ts.expect_true(os.path.exists("/usr/local/share/applications/boinc.desktop"), "Test 'boinc.desktop' file exists in '/usr/local/share/applications/'")
        ts.expect_true(os.path.exists("/usr/local/share/icons/boinc.png"), "Test 'boinc.png' file exists in '/usr/local/share/icons/'")
        ts.expect_true(os.path.exists("/usr/local/share/icons/boinc.svg"), "Test 'boinc.svg' file exists in '/usr/local/share/icons/'")
        ts.expect_true(os.path.exists("/var/lib/boinc/cc_config.xml"), "Test 'cc_config.xml' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.exists("/etc/boinc-client/cc_config.xml"), "Test 'cc_config.xml' file exists in '/etc/boinc-client/'")
        if (os.path.islink("/etc/boinc-client/cc_config.xml")):
            ts.expect_equal("/var/lib/boinc/cc_config.xml", os.readlink("/etc/boinc-client/cc_config.xml"), "Test '/etc/boinc-client/cc_config.xml' file is a symbolic link to '/var/lib/boinc/cc_config.xml'")
        elif(os.path.islink("/var/lib/boinc/cc_config.xml")):
            ts.expect_equal("/etc/boinc-client/cc_config.xml", os.readlink("/var/lib/boinc/cc_config.xml"), "Test '/var/lib/boinc/cc_config.xml' file is a symbolic link to '/etc/boinc-client/cc_config.xml'")
        else:
            ts.expect_true(False, "Test 'cc_config.xml' file is a symbolic link")
        ts.expect_true(os.path.exists("/var/lib/boinc/global_prefs_override.xml"), "Test 'global_prefs_override.xml' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.exists("/etc/boinc-client/global_prefs_override.xml"), "Test 'global_prefs_override.xml' file exists in '/etc/boinc-client/'")
        if (os.path.islink("/etc/boinc-client/global_prefs_override.xml")):
            ts.expect_equal("/var/lib/boinc/global_prefs_override.xml", os.readlink("/etc/boinc-client/global_prefs_override.xml"), "Test '/etc/boinc-client/global_prefs_override.xml' file is a symbolic link to '/var/lib/boinc/global_prefs_override.xml'")
        elif(os.path.islink("/var/lib/boinc/global_prefs_override.xml")):
            ts.expect_equal("/etc/boinc-client/global_prefs_override.xml", os.readlink("/var/lib/boinc/global_prefs_override.xml"), "Test '/var/lib/boinc/global_prefs_override.xml' file is a symbolic link to '/etc/boinc-client/global_prefs_override.xml'")
        else:
            ts.expect_true(False, "Test 'global_prefs_override.xml' file is a symbolic link")
        ts.expect_true(os.path.exists("/var/lib/boinc/remote_hosts.cfg"), "Test 'remote_hosts.cfg' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.exists("/etc/boinc-client/remote_hosts.cfg"), "Test 'remote_hosts.cfg' file exists in '/etc/boinc-client/'")
        if (os.path.islink("/etc/boinc-client/remote_hosts.cfg")):
            ts.expect_equal("/var/lib/boinc/remote_hosts.cfg", os.readlink("/etc/boinc-client/remote_hosts.cfg"), "Test '/etc/boinc-client/remote_hosts.cfg' file is a symbolic link to '/var/lib/boinc/remote_hosts.cfg'")
        elif(os.path.islink("/var/lib/boinc/remote_hosts.cfg")):
            ts.expect_equal("/etc/boinc-client/remote_hosts.cfg", os.readlink("/var/lib/boinc/remote_hosts.cfg"), "Test '/var/lib/boinc/remote_hosts.cfg' file is a symbolic link to '/etc/boinc-client/remote_hosts.cfg'")
        else:
            ts.expect_true(False, "Test 'remote_hosts.cfg' file is a symbolic link")
        ts.expect_true(os.path.exists("/var/lib/boinc/gui_rpc_auth.cfg"), "Test 'gui_rpc_auth.cfg' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.exists("/etc/boinc-client/gui_rpc_auth.cfg"), "Test 'gui_rpc_auth.cfg' file exists in '/etc/boinc-client/'")
        if (os.path.islink("/etc/boinc-client/gui_rpc_auth.cfg")):
            ts.expect_equal("/var/lib/boinc/gui_rpc_auth.cfg", os.readlink("/etc/boinc-client/gui_rpc_auth.cfg"), "Test '/etc/boinc-client/gui_rpc_auth.cfg' file is a symbolic link to '/var/lib/boinc/gui_rpc_auth.cfg'")
        elif(os.path.islink("/var/lib/boinc/gui_rpc_auth.cfg")):
            ts.expect_equal("/etc/boinc-client/gui_rpc_auth.cfg", os.readlink("/var/lib/boinc/gui_rpc_auth.cfg"), "Test '/var/lib/boinc/gui_rpc_auth.cfg' file is a symbolic link to '/etc/boinc-client/gui_rpc_auth.cfg'")
        else:
            ts.expect_true(False, "Test 'gui_rpc_auth.cfg' file is a symbolic link")
        ts.expect_true(len(self._get_ca_certificates_file_paths()) > 0, "Test system 'ca-certificates.crt' file exists")
        ts.expect_true(os.path.exists("/var/lib/boinc/ca-bundle.crt"), "Test 'ca-bundle.crt' file exists in '/var/lib/boinc/'")
        ts.expect_true(os.path.islink("/var/lib/boinc/ca-bundle.crt"), "Test '/var/lib/boinc/ca-bundle.crt' file is a symbolic link")
        ts.expect_true(os.readlink("/var/lib/boinc/ca-bundle.crt") in self._get_ca_certificates_file_paths(), "Test '/var/lib/boinc/ca-bundle.crt' file is a symbolic link to the system 'ca-certificates.crt' file")
        ts.expect_true(os.path.exists("/var/lib/boinc/all_projects_list.xml"), "Test 'all_projects_list.xml' file exists in '/var/lib/boinc/'")
        return ts.result()

    def test_version(self):
        ts = testset.TestSet("Test version is correct")
        current_version = self.testhelper.get_current_version_number()
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
        if (self._get_group_exists("docker")):
            ts.expect_true(self._get_user_in_group("boinc", "docker"), "Test 'boinc' user is in 'docker' group")
        ts.expect_equal("/var/lib/boinc", self._get_user_home_directory("boinc"), "Test 'boinc' user home directory is '/var/lib/boinc'")
        ts.expect_equal("100000:65536", self._get_uid_range("boinc"), "Test 'boinc' user UID range is '100000:65536'")
        ts.expect_equal("100000:65536", self._get_gid_range("boinc"), "Test 'boinc' group GID range is '100000:65536'")
        return ts.result()

    def test_selected_values_from_boinc_client_service_file(self):
        ts = testset.TestSet("Test selected values from the '/usr/lib/systemd/system/boinc-client.service' file")
        data = self._get_key_value_pairs_from_file("/usr/lib/systemd/system/boinc-client.service")
        ts.expect_equal(data["ProtectSystem"], "strict", "Test 'ProtectSystem' is correctly set")
        ts.expect_equal(data["ReadWritePaths"], "-/var/lib/boinc -/etc/boinc-client -/tmp -/var/tmp", "Test 'ReadWritePaths' is correctly set")
        ts.expect_equal(data["User"], "boinc", "Test 'User' is correctly set")
        ts.expect_equal(data["WorkingDirectory"], "/var/lib/boinc", "Test 'WorkingDirectory' is correctly set")
        ts.expect_equal(data["ExecStart"], "/usr/local/bin/boinc", "Test 'ExecStart' is correctly set")
        ts.expect_equal(data["ExecStop"], "/usr/local/bin/boinccmd --quit", "Test 'ExecStop' is correctly set")
        ts.expect_equal(data["ExecReload"], "/usr/local/bin/boinccmd --read_cc_config", "Test 'ExecReload' is correctly set")
        ts.expect_equal(data["ExecStopPost"], "/bin/rm -f lockfile", "Test 'ExecStopPost' is correctly set")
        return ts.result()

    def test_files_permissions(self):
        ts = testset.TestSet("Test files permissions")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/etc/boinc-client/cc_config.xml"), "Test '/etc/boinc-client/cc_config.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/etc/boinc-client/global_prefs_override.xml"), "Test '/etc/boinc-client/global_prefs_override.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/etc/boinc-client/remote_hosts.cfg"), "Test '/etc/boinc-client/remote_hosts.cfg' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/cc_config.xml"), "Test '/var/lib/boinc/cc_config.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/global_prefs_override.xml"), "Test '/var/lib/boinc/global_prefs_override.xml' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/remote_hosts.cfg"), "Test '/var/lib/boinc/remote_hosts.cfg' file owner")
        ts.expect_equal("boinc:boinc", self._get_file_owner("/var/lib/boinc/gui_rpc_auth.cfg"), "Test '/var/lib/boinc/gui_rpc_auth.cfg' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/bin/boinc"), "Test '/usr/local/bin/boinc' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/bin/boinccmd"), "Test '/usr/local/bin/boinccmd' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/bin/boincmgr"), "Test '/usr/local/bin/boincmgr' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/lib/systemd/system/boinc-client.service"), "Test '/usr/lib/systemd/system/boinc-client.service' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/etc/default/boinc-client"), "Test '/etc/default/boinc-client' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/etc/bash_completion.d/boinc.bash"), "Test '/etc/bash_completion.d/boinc.bash' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/etc/X11/Xsession.d/36x11-common_xhost-boinc"), "Test '/etc/X11/Xsession.d/36x11-common_xhost-boinc' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/share/applications/boinc.desktop"), "Test '/usr/local/share/applications/boinc.desktop' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/share/icons/boinc.png"), "Test '/usr/local/share/icons/boinc.png' file owner")
        ts.expect_equal("root:root", self._get_file_owner("/usr/local/share/icons/boinc.svg"), "Test '/usr/local/share/icons/boinc.svg' file owner")
        return ts.result()

    def _is_service_initialized_after_install(self):
        boinc_path = self._get_test_executable_file_path("boinc")
        if not boinc_path or not os.path.exists(boinc_path):
            return False
        install_time = int(os.path.getctime(boinc_path))
        since_arg = "@{ts}".format(ts=install_time)
        state_command = [
            "systemctl", "show", "boinc-client.service",
            "--property=ActiveState,MainPID,InvocationID",
        ]
        for _ in range(30):
            try:
                state = subprocess.run(state_command, check=True, capture_output=True, text=True, timeout=5).stdout
                properties = dict(line.split("=", 1) for line in state.splitlines() if "=" in line)
                invocation = properties.get("InvocationID", "")
                if (properties.get("ActiveState") == "active"
                        and int(properties.get("MainPID", "0")) > 0 and invocation):
                    output = subprocess.run([
                        "journalctl", "-u", "boinc-client.service", "-o", "cat",
                        "--since", since_arg, "--no-pager",
                        "_SYSTEMD_INVOCATION_ID=" + invocation,
                    ], check=True, capture_output=True, text=True, timeout=5).stdout
                    if "Initialization completed" in output:
                        # A crash or restart while reading the journal must not pass.
                        current_state = subprocess.run(state_command, check=True, capture_output=True, text=True, timeout=5).stdout
                        if current_state == state:
                            return True
            except (OSError, subprocess.SubprocessError, ValueError) as error:
                print("Unable to verify boinc-client.service: {error}".format(error=error))
                return False
            time.sleep(1)
        return False

    def test_service_running(self):
        ts = testset.TestSet("Test 'boinc-client.service' started and initialized")
        ts.expect_true(self._is_service_initialized_after_install(), "Test service is active and its current invocation initialized after package installation")
        return ts.result()

if __name__ == "__main__":
    if not IntegrationTests().result:
        sys.exit(1)
    sys.exit(0)
