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
#include "common_defs.h"
#include "credit.h"

namespace test_credit {
    class test_credit : public ::testing::Test {};

    TEST_F(test_credit, fpops_to_credit) {
        ASSERT_EQ(fpops_to_credit(1.0), COBBLESTONE_SCALE);
        ASSERT_NE(fpops_to_credit(5.0), COBBLESTONE_SCALE);
        ASSERT_EQ(fpops_to_credit(6000000.0), 6000000.0 * COBBLESTONE_SCALE);
    }

    TEST_F(test_credit, cpu_time_to_credit) {
        ASSERT_EQ(cpu_time_to_credit(1.0, 1.0), COBBLESTONE_SCALE);
    }

}
