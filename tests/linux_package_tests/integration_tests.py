import os
import re
import sys
import testset

class IntegrationTests:
    def __init__(self):
        self.result = True
        self.result &= self.test_files_exist()
        self.result &= self.test_version()
        self.result &= self.test_user()

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

    def test_files_exist(self):
        ts = testset.TestSet("Test files exist")
        ts.expect_equal("/usr/local/bin/boinc", self._get_test_executable_file_path("boinc"), "Test 'boinc' file location")
        ts.expect_equal("/usr/local/bin/boinccmd", self._get_test_executable_file_path("boinccmd"), "Test 'boinccmd' file location")
        ts.expect_equal("/usr/local/bin/boincmgr", self._get_test_executable_file_path("boincmgr"), "Test 'boincmgr' file location")
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

if __name__ == "__main__":
    if not IntegrationTests().result:
        sys.exit(1)
    sys.exit(0)
