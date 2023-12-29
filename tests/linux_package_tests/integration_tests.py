import os
import sys
import testset

def get_test_executable_file_path(filename):
    return os.popen("which " + filename).read().strip()

def test_files_exist():
    ts = testset.TestSet("Test Files Exist")
    ts.expect_equal("/usr/local/bin/boinc", get_test_executable_file_path("boinc"), "Test 'boinc' file location")
    ts.expect_equal("/usr/local/bin/boinccmd", get_test_executable_file_path("boinccmd"), "Test 'boinccmd' file location")
    ts.expect_equal("/usr/local/bin/boincmgr", get_test_executable_file_path("boincmgr"), "Test 'boincmgr' file location")
    return ts.result()

def main():
    result = True
    result &= test_files_exist()
    return result

if __name__ == "__main__":
    if not main():
        sys.exit(1)
    sys.exit(0)
