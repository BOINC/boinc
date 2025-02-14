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

#ifdef _WIN32
    #include "boinc_win.h"
#endif

#include "shmem.h"

using namespace std;

namespace test_shmem {
    class test_shmem : public ::testing::Test {};

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
}
