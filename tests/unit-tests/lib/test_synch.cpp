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

#ifndef _WIN32

#include "gtest/gtest.h"
#include "synch.h"

#define KEY 0xdeadbeef

using namespace std;

namespace test_synch {
    class test_synch : public ::testing::Test {};

    TEST_F(test_synch, synch) {
        EXPECT_EQ(create_semaphore(KEY), 0);
        EXPECT_EQ(lock_semaphore(KEY), 0);
        EXPECT_EQ(unlock_semaphore(KEY), 0);
        EXPECT_EQ(destroy_semaphore(KEY), 0);
    }

}

#endif
