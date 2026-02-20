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

namespace test_boinccas_CADeleteBOINCAccounts {
    class test_boinccas_CADeleteBOINCAccounts : public test_boinccas_TestBase {
    protected:
        test_boinccas_CADeleteBOINCAccounts() :
            test_boinccas_TestBase("DeleteBOINCAccounts") {
        }

        void TearDown() override {
            if (userExists("boinc_master")) {
                userDelete("boinc_master");
            }
            if (userExists("boinc_project")) {
                userDelete("boinc_project");
            }
            cleanRegistryKey();
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoData_Expect_Success_With_No_Actions) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoUpgrade_BOINC_MASTER_USERNAME_Is_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("BOINC_MASTER_USERNAME", "");
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoUpgrade_BOINC_PROJECT_USERNAME_Is_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("BOINC_PROJECT_USERNAME", "");
        auto [errorcode, value] = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoUpgrade_BOINC_MASTER_USERNAME_Is_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username = "boinc_master";

        setMsiProperty("BOINC_MASTER_USERNAME", username);
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username, value);

        ASSERT_FALSE(userExists(username));
        ASSERT_TRUE(userCreate(username, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username));

        ASSERT_EQ(0u, executeAction());
        auto userDeleted = false;
        for (auto i = 0; i < 10; ++i) {
            if (!userExists(username)) {
                userDeleted = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        EXPECT_TRUE(userDeleted);
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoUpgrade_BOINC_PROJECT_USERNAME_Is_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username = "boinc_project";

        setMsiProperty("BOINC_PROJECT_USERNAME", username);
        auto [errorcode, value] = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username, value);

        ASSERT_FALSE(userExists(username));
        ASSERT_TRUE(userCreate(username, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username));

        ASSERT_EQ(0u, executeAction());
        auto userDeleted = false;
        for (auto i = 0; i < 10; ++i) {
            if (!userExists(username)) {
                userDeleted = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        EXPECT_TRUE(userDeleted);
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        NoUpgrade_BOINC_MASTER_USERNAME_And_BOINC_PROJECT_USERNAME_Are_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username_master = "boinc_master";
        setMsiProperty("BOINC_MASTER_USERNAME", username_master);
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username_master, value);

        ASSERT_FALSE(userExists(username_master));
        ASSERT_TRUE(userCreate(username_master, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username_master));

        constexpr auto username_project = "boinc_project";
        setMsiProperty("BOINC_PROJECT_USERNAME", username_project);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username_project, value);

        ASSERT_FALSE(userExists(username_project));
        ASSERT_TRUE(userCreate(username_project, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username_project));

        ASSERT_EQ(0u, executeAction());
        auto masterUserDeleted = false;
        for (auto i = 0; i < 10; ++i) {
            if (!userExists(username_master)) {
                masterUserDeleted = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        EXPECT_TRUE(masterUserDeleted);

        auto projectUserDeleted = false;
        for (auto i = 0; i < 10; ++i) {
            if (!userExists(username_project)) {
                projectUserDeleted = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        EXPECT_TRUE(projectUserDeleted);
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        DoUpgrade_BOINC_MASTER_USERNAME_Is_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("BOINC_MASTER_USERNAME", "");
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        setMsiProperty("ProductVersion", "1.0.0");
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        DoUpgrade_BOINC_PROJECT_USERNAME_Is_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("BOINC_PROJECT_USERNAME", "");
        auto [errorcode, value] = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        setMsiProperty("ProductVersion", "1.0.0");
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        DoUpgrade_BOINC_MASTER_USERNAME_Is_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username = "boinc_master";

        setMsiProperty("BOINC_MASTER_USERNAME", username);
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username, value);

        setMsiProperty("ProductVersion", "1.0.0");
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        ASSERT_FALSE(userExists(username));
        ASSERT_TRUE(userCreate(username, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username));

        ASSERT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(username));
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        DoUpgrade_BOINC_PROJECT_USERNAME_Is_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username = "boinc_project";

        setMsiProperty("BOINC_PROJECT_USERNAME", username);
        auto [errorcode, value] = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username, value);

        ASSERT_FALSE(userExists(username));
        ASSERT_TRUE(userCreate(username, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username));

        setMsiProperty("ProductVersion", "1.0.0");
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        ASSERT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(username));
    }

    TEST_F(test_boinccas_CADeleteBOINCAccounts,
        DoUpgrade_BOINC_MASTER_USERNAME_And_BOINC_PROJECT_USERNAME_Are_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        constexpr auto username_master = "boinc_master";
        setMsiProperty("BOINC_MASTER_USERNAME", username_master);
        auto [errorcode, value] = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username_master, value);

        ASSERT_FALSE(userExists(username_master));
        ASSERT_TRUE(userCreate(username_master, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username_master));

        constexpr auto username_project = "boinc_project";
        setMsiProperty("BOINC_PROJECT_USERNAME",
            username_project);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(username_project, value);

        ASSERT_FALSE(userExists(username_project));
        ASSERT_TRUE(userCreate(username_project, "qwerty123456!@#$%^"));
        ASSERT_TRUE(userExists(username_project));

        setMsiProperty("ProductVersion", "1.0.0");
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1.0.0", value);

        ASSERT_TRUE(setRegistryValue("UpgradingTo", "2.0.0"));

        ASSERT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(username_master));
        EXPECT_TRUE(userExists(username_project));
    }
#endif
}
