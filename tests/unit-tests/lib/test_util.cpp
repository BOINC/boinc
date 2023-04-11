// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "gtest/gtest.h"
#include "util.h"
#include <math.h>

using namespace std;

namespace test_util {

    // The fixture for testing class Foo.

    class test_util : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_util() {
            // You can do set-up work for each test here.
        }

        virtual ~test_util() {
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

    TEST_F(test_util, dtime) {
        EXPECT_TRUE(dtime() >= 0);
    }

    TEST_F(test_util, dday) {
        EXPECT_TRUE(dday() >= 0);
    }

    TEST_F(test_util, boinc_sleep) {
        double startTime = dtime();
        boinc_sleep(2);
        double endTime = dtime();
        EXPECT_EQ(trunc(endTime - startTime) , 2);
    }

    TEST_F(test_util, push_unique) {
        vector<string> strings;
        push_unique("a", strings);
        EXPECT_EQ(strings.size(), (long unsigned int) 1);

        push_unique("b", strings);
        EXPECT_EQ(strings.size(), (long unsigned int) 2);

        push_unique("b", strings);
        EXPECT_EQ(strings.size(), (long unsigned int) 2);
    }

    TEST_F(test_util, drand) {
        EXPECT_TRUE(drand() > 0);
    }

    TEST_F(test_util, rand_normal) {
        rand_normal();
        rand_normal();
        SUCCEED();
    }

#ifdef _WIN32


#else

    TEST_F(test_util, linux_cpu_time) {
        EXPECT_TRUE(linux_cpu_time(1) > 0);
    }

#endif


} // namespace
