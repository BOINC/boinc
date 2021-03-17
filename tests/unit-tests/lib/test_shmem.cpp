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

#ifndef _WIN32

#include "gtest/gtest.h"
#include "shmem.h"

#define KEY 0xbeefcafe

using namespace std;

namespace test_shmem {

    // The fixture for testing class Foo.

    class test_shmem : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_shmem() {
            // You can do set-up work for each test here.
        }

        virtual ~test_shmem() {
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

    // Test shmem functions for Unix/Linux/Mac V5 applications
    TEST_F(test_shmem, shmem) {
        void* p;
        EXPECT_EQ(create_shmem(KEY, 100, false, &p), 0);

        EXPECT_EQ(attach_shmem(KEY, &p), 0);

        EXPECT_EQ(destroy_shmem(KEY), 0);
    }

} // namespace

#endif
