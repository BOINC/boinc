// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
    class test_util : public ::testing::Test {};

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

#if !defined(_WIN32) && !defined(__APPLE__)
    TEST_F(test_util, linux_cpu_time) {
        EXPECT_TRUE(linux_cpu_time(1) > 0);
    }

#endif
}
