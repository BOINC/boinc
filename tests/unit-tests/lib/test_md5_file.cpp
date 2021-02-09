// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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
#include "md5_file.h"

using namespace std;

namespace test_md5_file {

    // The fixture for testing class Foo.

    class test_md5_file : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_md5_file() {
            // You can do set-up work for each test here.
        }

        virtual ~test_md5_file() {
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

    TEST_F(test_md5_file, md5_string) {
        std::string result = md5_string("abcdefghijk");
        EXPECT_EQ(result, "92b9cccc0b98c3a0b8d0df25a421c0e3");
    }

#ifndef _WIN32

    TEST_F(test_md5_file, make_secure_random_string_os) {
        char output[32];
        EXPECT_EQ(make_secure_random_string_os(output), 0);
    }

    TEST_F(test_md5_file, md5_file) {
        char output[33];
        double bytes;
        int result = md5_file("../unit-tests/lib/test_md5_file.txt", output, bytes);
        EXPECT_EQ(result, 0);
        EXPECT_STREQ(output, "3b13c74a05696e71f9aeb4e6f10cbae8");
        EXPECT_EQ(bytes, 737);
    }

#endif

} // namespace


