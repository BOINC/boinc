import sys
import testset

def main():
    test_set = testset.TestSet("TestSet")
    test_set.expect_true(True, "True")
    test_set.expect_false(False, "False")
    test_set.expect_equal(1, 1, "Equal")
    test_set.expect_not_equal(1, 2, "Not Equal")
    return test_set.result()

if __name__ == "__main__":
    if not main():
        sys.exit(1)
    sys.exit(0)
