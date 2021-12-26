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
#include "mem_usage.h"

using namespace std;

namespace test_mem_usage {

    // The fixture for testing class Foo.

    class test_mem_usage : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_mem_usage() {
            // You can do set-up work for each test here.
        }

        virtual ~test_mem_usage() {
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

    // Test shmem functions for Windows/Unix/Linux/Mac V5 applications
    TEST_F(test_mem_usage, mem_usage) {
        double vm_usage;
        double resident_set;

        int result = mem_usage(vm_usage, resident_set);
        EXPECT_EQ(result, 0);
        EXPECT_GT(vm_usage, 1024); //assumes at least 1kb of memory usage
        EXPECT_GT(resident_set, 1024); //assumes at least 1kb of memory usage
    }

} // namespace
