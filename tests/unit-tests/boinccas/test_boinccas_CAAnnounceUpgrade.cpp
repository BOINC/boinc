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

#include "boinccas_helper.h"

using namespace std;

namespace test_boinccas_CAAnnounceUpgrade {
    using AnnounceUpgradeFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CAAnnounceUpgrade : public ::testing::Test {
    protected:
        test_boinccas_CAAnnounceUpgrade() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<AnnounceUpgradeFn>(
                    "AnnounceUpgrade");
            cleanRegistryKey();
        }
        ~test_boinccas_CAAnnounceUpgrade() override {
            cleanRegistryKey();
        }

        AnnounceUpgradeFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;
    };

    constexpr auto expectedVersion = "1.2.3.4";
#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAAnnounceUpgrade,
        AnnounceUpgrade_Empty_ProductVersion) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, hFunc(hMsi));
        EXPECT_NE(expectedVersion, getRegistryValue("UpgradingTo"));
    }

    TEST_F(test_boinccas_CAAnnounceUpgrade,
        AnnounceUpgrade_With_ProductVersion) {
        msiHelper.insertProperties({
            {"ProductVersion", expectedVersion}
            });
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_EQ(expectedVersion, getRegistryValue("UpgradingTo"));
    }
#endif
}
