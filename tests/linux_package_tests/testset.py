class TestSet:
    def __init__(self, name):
        self.name = name
        self.passed = 0
        self.failed = 0
        self._result_requested = False
        print(("Running test set [{name}]...").format(name=self.name))

    def __del__(self):
        print(("Test set [{name}]: passed [{passed}], failed [{failed}].").format(name=self.name, passed=self.passed, failed=self.failed))
        if not self._result_requested and self.failed > 0:
            raise AssertionError

    def result(self):
        self._result_requested = True
        return self.failed == 0

    def expect_true(self, condition, test_name):
        if not condition:
            self.failed += 1
            print(("Failed [{test_name}]: expected True, got False").format(test_name=test_name))
            return False
        self._print_success_report(test_name)
        return True

    def expect_false(self, condition, test_name):
        if condition:
            self.failed += 1
            print(("Failed [{test_name}]: expected False, got True").format(test_name=test_name))
            return False
        self._print_success_report(test_name)
        return True

    def expect_equal(self, expected, actual, test_name):
        if expected != actual:
            self.failed += 1
            print(("Failed [{test_name}]: expected [{expected}], got [{actual}]").format(test_name=test_name, expected=expected, actual=actual))
            return False
        self._print_success_report(test_name)
        return True

    def expect_not_equal(self, a, b, test_name):
        if a == b:
            self.failed += 1
            print(("Failed [{test_name}]: expected {a} != {b}").format(test_name=test_name, a=a, b=b))
            return False
        self._print_success_report(test_name)

    def _print_success_report(self, test_name):
        self.passed += 1
        print(("Passed [{test_name}]").format(test_name=test_name))
