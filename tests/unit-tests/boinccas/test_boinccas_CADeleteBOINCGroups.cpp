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

namespace test_boinccas_CADeleteBOINCGroups {

    using DeleteBOINCGroupsFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CADeleteBOINCGroups : public ::testing::Test {
    protected:
        test_boinccas_CADeleteBOINCGroups() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<DeleteBOINCGroupsFn>(
                    "DeleteBOINCGroups");
        }

        void TearDown() override {
            if (localGroupExists("boinc_admins")) {
                deleteLocalGroup("boinc_admins");
            }
            if (localGroupExists("boinc_users")) {
                deleteLocalGroup("boinc_users");
            }
            if (localGroupExists("boinc_projects")) {
                deleteLocalGroup("boinc_projects");
            }
            cleanRegistryKey();
        }

        DeleteBOINCGroupsFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;

    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoData_Expect_Success_With_No_Actions) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_ADMINS_EXISTS) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_USERS_EXISTS) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_users"));
        ASSERT_TRUE(localGroupExists("boinc_users"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_PROJECTS_EXISTS) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_projects"));
        ASSERT_TRUE(localGroupExists("boinc_projects"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_ADMINS_AND_BOINC_USERS_EXIST) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        ASSERT_TRUE(createLocalGroup("boinc_users"));
        ASSERT_TRUE(localGroupExists("boinc_users"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_ADMINS_AND_BOINC_PROJECTS_EXIST) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        ASSERT_TRUE(createLocalGroup("boinc_projects"));
        ASSERT_TRUE(localGroupExists("boinc_projects"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_USERS_AND_BOINC_PROJECTS_EXIST) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_users"));
        ASSERT_TRUE(localGroupExists("boinc_users"));
        ASSERT_TRUE(createLocalGroup("boinc_projects"));
        ASSERT_TRUE(localGroupExists("boinc_projects"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        NoUpgrade_BOINC_ADMINS_AND_BOINC_USERS_AND_BOINC_PROJECTS_EXIST) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        ASSERT_TRUE(createLocalGroup("boinc_users"));
        ASSERT_TRUE(localGroupExists("boinc_users"));
        ASSERT_TRUE(createLocalGroup("boinc_projects"));
        ASSERT_TRUE(localGroupExists("boinc_projects"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_FALSE(localGroupExists("boinc_users"));
        EXPECT_FALSE(localGroupExists("boinc_projects"));
    }

    TEST_F(test_boinccas_CADeleteBOINCGroups,
        DoUpgrade_BOINC_ADMINS_AND_BOINC_USERS_AND_BOINC_PROJECTS_EXIST) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        ASSERT_FALSE(localGroupExists("boinc_admins"));
        ASSERT_FALSE(localGroupExists("boinc_users"));
        ASSERT_FALSE(localGroupExists("boinc_projects"));

        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        ASSERT_TRUE(createLocalGroup("boinc_users"));
        ASSERT_TRUE(localGroupExists("boinc_users"));
        ASSERT_TRUE(createLocalGroup("boinc_projects"));
        ASSERT_TRUE(localGroupExists("boinc_projects"));

        msiHelper.setProperty(hMsi, "ProductVersion", "1.0.0");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        ASSERT_EQ(0u, hFunc(hMsi));

        EXPECT_TRUE(localGroupExists("boinc_admins"));
        EXPECT_TRUE(localGroupExists("boinc_users"));
        EXPECT_TRUE(localGroupExists("boinc_projects"));
    }
#endif
}
