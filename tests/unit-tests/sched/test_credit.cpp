#include "gtest/gtest.h"
#include "common_defs.h"
#include "credit.h"

namespace test_credit {

    // The fixture for testing class Foo.

    class test_credit : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_credit() {
            // You can do set-up work for each test here.
        }

        virtual ~test_credit() {
            // You can do clean-up work that doesn't throw exceptions here.
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:

        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }

        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }

        // Objects declared here can be used by all tests in the test case for Foo.
    };

    // Tests that Foo does Xyz.

    TEST_F(test_credit, fpops_to_credit) {
        ASSERT_EQ(fpops_to_credit(1.0), COBBLESTONE_SCALE);
        ASSERT_NE(fpops_to_credit(5.0), COBBLESTONE_SCALE);
        ASSERT_EQ(fpops_to_credit(6000000.0), 6000000.0 * COBBLESTONE_SCALE);
    }

    TEST_F(test_credit, cpu_time_to_credit) {
        ASSERT_EQ(cpu_time_to_credit(1.0, 1.0), COBBLESTONE_SCALE);
    }

} // namespace
