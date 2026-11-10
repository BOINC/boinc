// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "boinccas_helper.h"

namespace test_boinccas {
    class test_boinccas_CACheckWSLSupport :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CACheckWSLSupport() :
            test_boinccas_TestBase("CheckWSLSupport") {}
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACheckWSLSupport,
        CheckDefaultValues) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("INSTALLWSLIMAGEINSTALLER");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("1", returnValue);

        std::tie(errorcode, returnValue) =
            getMsiProperty("LAUNCHWSLIMAGEINSTALLER");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("1", returnValue);
    }

    TEST_F(test_boinccas_CACheckWSLSupport,
        CheckValuesSet) {
        insertMsiProperties({
            {"INSTALLWSLIMAGEINSTALLER", "0"},
            {"LAUNCHWSLIMAGEINSTALLER", "0"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("INSTALLWSLIMAGEINSTALLER");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("1", returnValue);

        std::tie(errorcode, returnValue) =
            getMsiProperty("LAUNCHWSLIMAGEINSTALLER");
        ASSERT_EQ(0u, errorcode);
        EXPECT_TRUE(returnValue.empty());
    }
#endif
}
