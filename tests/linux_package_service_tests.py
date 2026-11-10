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

"""Regression tests for the package test's service health check."""

import subprocess
import unittest
from unittest.mock import patch

from linux_package_integration_tests import IntegrationTests


ACTIVE = "ActiveState=active\nMainPID=123\nInvocationID=current\n"
INACTIVE = "ActiveState=failed\nMainPID=0\nInvocationID=current\n"
INITIALIZED = "Initialization completed\n"


class ServiceHealthTests(unittest.TestCase):
    def setUp(self):
        self.service = IntegrationTests.__new__(IntegrationTests)
        patches = [
            patch.object(self.service, "_get_test_executable_file_path", return_value="/usr/bin/boinc"),
            patch("linux_package_integration_tests.os.path.exists", return_value=True),
            patch("linux_package_integration_tests.os.path.getctime", return_value=1000),
            patch("linux_package_integration_tests.time.sleep"),
        ]
        for item in patches:
            item.start()
            self.addCleanup(item.stop)

    def run_check(self, responses):
        with patch("linux_package_integration_tests.subprocess.run") as run:
            run.side_effect = lambda command, **kwargs: subprocess.CompletedProcess(
                command, 0, stdout=responses(command), stderr=""
            )
            return self.service._is_service_initialized_after_install()

    def test_active_current_invocation_passes(self):
        def response(command):
            if command[0] == "journalctl":
                self.assertIn("_SYSTEMD_INVOCATION_ID=current", command)
                self.assertEqual(command[command.index("--since") + 1], "@1000")
                return INITIALIZED
            return ACTIVE
        self.assertTrue(self.run_check(response))

    def test_inactive_service_with_old_log_fails(self):
        self.assertFalse(self.run_check(
            lambda command: INACTIVE if command[0] == "systemctl" else INITIALIZED
        ))

    def test_current_invocation_without_initialization_fails(self):
        self.assertFalse(self.run_check(
            lambda command: ACTIVE if command[0] == "systemctl" else ""
        ))

    def test_crash_during_journal_read_fails(self):
        def response(command):
            if command[0] == "journalctl":
                return INITIALIZED
            return next(states, INACTIVE)
        states = iter([ACTIVE, INACTIVE])
        self.assertFalse(self.run_check(response))

    def test_restart_during_journal_read_requires_new_initialization(self):
        restarted = ACTIVE.replace("current", "new").replace("123", "456")
        states = iter([ACTIVE, restarted])
        def response(command):
            if command[0] == "systemctl":
                return next(states, restarted)
            return INITIALIZED if "_SYSTEMD_INVOCATION_ID=current" in command else ""
        self.assertFalse(self.run_check(response))

    def test_systemctl_failure_fails(self):
        with patch("linux_package_integration_tests.subprocess.run", side_effect=subprocess.CalledProcessError(1, "systemctl")):
            self.assertFalse(self.service._is_service_initialized_after_install())


if __name__ == "__main__":
    unittest.main()
