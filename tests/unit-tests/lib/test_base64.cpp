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
#include "base64.h"

using namespace std;

namespace test_base64 {
    class test_base64 : public ::testing::Test {};

    TEST_F(test_base64, r_base64_encode) {
        EXPECT_EQ(r_base64_encode("Boinc"), "Qm9pbmM=");
        EXPECT_EQ(r_base64_encode("B o i n c"), "QiBvIGkgbiBj");
        EXPECT_EQ(r_base64_encode("Bòíncüñ"), "QsOyw61uY8O8w7E=");
        EXPECT_EQ(r_base64_encode("äöüß"), "w6TDtsO8w58=");
        EXPECT_EQ(r_base64_encode("new\nline"), "bmV3CmxpbmU=");
    }

    TEST_F(test_base64, r_base64_decode) {
        EXPECT_EQ(r_base64_decode("Qm9pbmM="), "Boinc");
        EXPECT_EQ(r_base64_decode("QiBvIGkgbiBj"), "B o i n c");
        EXPECT_EQ(r_base64_decode("QsOyw61uY8O8w7E="), "Bòíncüñ");
        EXPECT_EQ(r_base64_decode("w6TDtsO8w58="), "äöüß");
        EXPECT_EQ(r_base64_decode("bmV3CmxpbmU="), "new\nline");
    }
}
