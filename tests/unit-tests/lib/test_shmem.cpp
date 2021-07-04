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

#ifdef _WIN32
    #include "boinc_win.h"
#endif

#include "shmem.h"

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

    // Test shmem functions for Windows/Unix/Linux/Mac V5 applications
    TEST_F(test_shmem, shmem) {
        void* p;

#ifdef _WIN32
        const std::string KEY("0xbeefcafe");

        HANDLE handle = create_shmem(reinterpret_cast<LPCTSTR>(KEY.c_str()), 100, &p);
        EXPECT_NE(handle, INVALID_HANDLE_VALUE);
        HANDLE handle2 = attach_shmem(reinterpret_cast<LPCTSTR>(KEY.c_str()), &p);
        EXPECT_NE(handle2, INVALID_HANDLE_VALUE);
        EXPECT_EQ(detach_shmem(handle, &p), 0);
        EXPECT_EQ(detach_shmem(handle2, &p), 0);
#else
#define KEY 0xbeefcafe

        EXPECT_EQ(create_shmem(KEY, 100, false, &p), 0);
        EXPECT_EQ(attach_shmem(KEY, &p), 0);
        EXPECT_EQ(destroy_shmem(KEY), 0);
#endif
    }

} // namespace
