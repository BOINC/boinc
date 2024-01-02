import os
import re
import sys
import testset

def get_test_executable_file_path(filename):
    path = os.popen("echo $PATH").read().strip()
    for p in path.split(":"):
        if os.path.exists(os.path.join(p, filename)):
            return os.path.join(p, filename)
    return ""

def get_current_version_number():
    with open("version.h", "r") as f:
        lines = f.readlines()
        for line in lines:
            res = re.search("#define BOINC_VERSION_STRING \"([\d]+.[\d]+.[\d]+)\"", line)
            if res is not None:
                return res[1]
    return ""

def get_version_from_string(string):
    res = re.search("([\d]+.[\d]+.[\d]+)", string)
    if res is not None:
        return res[1]
    return ""

def get_file_version(filename):
    return get_version_from_string(os.popen(("{app} --version").format(app=get_test_executable_file_path(filename))).read().strip())

def get_user_exists(username):
    return os.popen("id -un {username}".format(username=username)).read().strip() == username

def get_group_exists(groupname):
    return os.popen("getent group {groupname}".format(groupname=groupname)).read().strip() != ""

def get_user_in_group(username, groupname):
    return os.popen("id -Gn {username}".format(username=username)).read().strip().find(groupname) != -1

def test_files_exist():
    ts = testset.TestSet("Test files exist")
    ts.expect_equal("/usr/local/bin/boinc", get_test_executable_file_path("boinc"), "Test 'boinc' file location")
    ts.expect_equal("/usr/local/bin/boinccmd", get_test_executable_file_path("boinccmd"), "Test 'boinccmd' file location")
    ts.expect_equal("/usr/local/bin/boincmgr", get_test_executable_file_path("boincmgr"), "Test 'boincmgr' file location")
    return ts.result()

def test_version():
    ts = testset.TestSet("Test version is correct")
    current_version = get_current_version_number()
    ts.expect_not_equal("", current_version, "Test current version could be read from the 'version.h' file")
    ts.expect_equal(current_version, get_file_version("boinc"), "Test 'boinc' version is correctly set")
    ts.expect_equal(current_version, get_file_version("boinccmd"), "Test 'boinccmd' version is correctly set")
    return ts.result()

def test_user():
    ts = testset.TestSet("Test 'boinc' user and 'boinc' group exist")
    ts.expect_true(get_user_exists("boinc"), "Test 'boinc' user exists")
    ts.expect_true(get_group_exists("boinc"), "Test 'boinc' group exists")
    ts.expect_true(get_user_in_group("boinc", "boinc"), "Test 'boinc' user is in 'boinc' group")
    return ts.result()

def main():
    result = True

    result &= test_files_exist()
    result &= test_version()
    result &= test_user()

    return result

if __name__ == "__main__":
    if not main():
        sys.exit(1)
    sys.exit(0)
