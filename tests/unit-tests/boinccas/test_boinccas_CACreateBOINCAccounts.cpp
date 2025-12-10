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

#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"

#include "boinccas_helper.h"

#include <MsiQuery.h>

namespace test_boinccas_CACreateBOINCAccounts {
    constexpr auto masterAccountName = "boinc_master";
    constexpr auto projectAccountName = "boinc_project";
    constexpr auto masterAccountPassword = "qwerty123456!@#$%^";
    constexpr auto projectAccountPassword = "ytrewq654321^%$#@!";

    using CreateBOINCAccountsFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CACreateBOINCAccounts : public ::testing::Test {
    protected:
        test_boinccas_CACreateBOINCAccounts() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<CreateBOINCAccountsFn>(
                    "CreateBOINCAccounts");
        }

        void TearDown() override {
            if (userExists(masterAccountName)) {
                userDelete(masterAccountName);
            }
            if (userExists(projectAccountName)) {
                userDelete(projectAccountName);
            }
        }

        CreateBOINCAccountsFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateBOINCAccounts, CanCreateAccounts) {
        ASSERT_FALSE(userExists(masterAccountName));
        ASSERT_TRUE(userCreate(masterAccountName, masterAccountPassword));
        ASSERT_TRUE(userExists(masterAccountName));
        ASSERT_TRUE(userDelete(masterAccountName));
        ASSERT_FALSE(userExists(masterAccountName));
    }

    TEST_F(test_boinccas_CACreateBOINCAccounts, CreateAccounts) {
        PMSIHANDLE hMsi;
        const auto result =
            MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_FALSE(userExists(masterAccountName));
        EXPECT_FALSE(userExists(projectAccountName));
        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(masterAccountName));
        EXPECT_TRUE(userExists(projectAccountName));
        EXPECT_EQ(masterAccountName,
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME"));
        EXPECT_EQ(projectAccountName,
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME"));
        EXPECT_EQ(std::string(".\\") + masterAccountName,
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME"));
        EXPECT_EQ(std::string(".\\") + projectAccountName,
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME"));
        EXPECT_NE("", msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD"));
        EXPECT_NE("", msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD"));
        EXPECT_EQ("1", msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED"));
        EXPECT_TRUE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);
    }
#endif
}
