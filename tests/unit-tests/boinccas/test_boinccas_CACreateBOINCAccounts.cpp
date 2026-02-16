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

namespace test_boinccas_CACreateBOINCAccounts {
    constexpr auto masterAccountName = "boinc_master";
    constexpr auto projectAccountName = "boinc_project";
    constexpr auto masterAccountPassword = "qwerty123456!@#$%^";
    constexpr auto projectAccountPassword = "ytrewq654321^%$#@!";
    constexpr auto testPCName = "testpc";

    class test_boinccas_CACreateBOINCAccounts :
        public test_boinccas_TestBase_WithParam<std::string_view> {
    protected:
        test_boinccas_CACreateBOINCAccounts() :
            test_boinccas_TestBase_WithParam("CreateBOINCAccounts") {
        }

        void SetUp() override {
            if (userExists(getMasterAccountName())) {
                userDelete(getMasterAccountName());
            }
            if (userExists(getProjectAccountName())) {
                userDelete(getProjectAccountName());
            }
        }

        void TearDown() override {
            if (userExists(getMasterAccountName())) {
                userDelete(getMasterAccountName());
            }
            if (userExists(getProjectAccountName())) {
                userDelete(getProjectAccountName());
            }
        }

        auto getMasterAccountName() const {
            if (GetParam() == "2") {
                return std::string(masterAccountName) + "_"
                    + testPCName;
            }
            return std::string(masterAccountName);
        }

        auto getProjectAccountName() const {
            if (GetParam() == "2") {
                return std::string(projectAccountName) + "_"
                    + testPCName;
            }
            return std::string(projectAccountName);
        }
    };

#ifdef BOINCCAS_TEST
    INSTANTIATE_TEST_SUITE_P(test_boinccas_CACreateBOINCAccountsProductType,
        test_boinccas_CACreateBOINCAccounts,
        testing::Values("1", "2", "3"));

    TEST_P(test_boinccas_CACreateBOINCAccounts, CanCreateAccounts) {
        ASSERT_FALSE(userExists(masterAccountName));
        ASSERT_TRUE(userCreate(masterAccountName, masterAccountPassword));
        ASSERT_TRUE(userExists(masterAccountName));
        ASSERT_TRUE(userDelete(masterAccountName));
        ASSERT_FALSE(userExists(masterAccountName));
    }

    TEST_P(test_boinccas_CACreateBOINCAccounts, CreateDefaultAccounts) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("MsiNTProductType", GetParam().data());
        auto [errorcode, value] = getMsiProperty("MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        setMsiProperty("ComputerName", testPCName);
        std::tie(errorcode, value) = getMsiProperty("ComputerName");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testPCName, value);

        EXPECT_FALSE(userExists(getMasterAccountName()));
        EXPECT_FALSE(userExists(getProjectAccountName()));
        EXPECT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(getMasterAccountName()));
        EXPECT_TRUE(userExists(getProjectAccountName()));

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getMasterAccountName(), value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getProjectAccountName(), value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getMasterAccountName(), value);

        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getProjectAccountName(), value);

        std::string masterPassword;
        std::tie(errorcode, masterPassword) =
            getMsiProperty("BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(33, masterPassword.length());
        EXPECT_NE("", masterPassword);

        std::string projectPassword;
        std::tie(errorcode, projectPassword) =
            getMsiProperty("BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(33, projectPassword.length());
        EXPECT_NE("", projectPassword);

        EXPECT_NE(masterPassword, projectPassword);

        std::tie(errorcode, value) =
            getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);

        TearDown();
    }

    TEST_P(test_boinccas_CACreateBOINCAccounts, ChangePasswords) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("MsiNTProductType", GetParam().data());
        auto [errorcode, value] = getMsiProperty("MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        setMsiProperty("ComputerName", testPCName);
        std::tie(errorcode, value) = getMsiProperty("ComputerName");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testPCName, value);

        EXPECT_FALSE(userExists(getMasterAccountName()));
        EXPECT_FALSE(userExists(getProjectAccountName()));
        ASSERT_TRUE(userCreate(getMasterAccountName(),
            masterAccountPassword));
        ASSERT_TRUE(userCreate(getProjectAccountName(),
            projectAccountPassword));
        ASSERT_TRUE(userExists(getMasterAccountName()));
        ASSERT_TRUE(userExists(getProjectAccountName()));
        setMsiProperty("BOINC_MASTER_USERNAME", getMasterAccountName());
        setMsiProperty("BOINC_PROJECT_USERNAME", getProjectAccountName());

        EXPECT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(getMasterAccountName()));
        EXPECT_TRUE(userExists(getProjectAccountName()));

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getMasterAccountName(), value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getProjectAccountName(), value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getMasterAccountName(), value);

        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getProjectAccountName(), value);

        std::string masterPassword;
        std::tie(errorcode, masterPassword) =
            getMsiProperty("BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(masterAccountPassword, masterPassword);
        EXPECT_EQ(33, masterPassword.length());

        std::string projectPassword;
        std::tie(errorcode, projectPassword) =
            getMsiProperty("BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(projectAccountPassword, projectPassword);
        EXPECT_EQ(33, projectPassword.length());

        EXPECT_NE(masterPassword, projectPassword);

        std::tie(errorcode, value) =
            getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("", value);

        EXPECT_FALSE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);

        TearDown();
    }

    TEST_P(test_boinccas_CACreateBOINCAccounts, DontChangeExistingAccounts) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("MsiNTProductType", GetParam().data());
        auto [errorcode, value] = getMsiProperty("MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        setMsiProperty("ComputerName", testPCName);
        std::tie(errorcode, value) = getMsiProperty("ComputerName");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testPCName, value);

        constexpr auto testMasterAccountName = "test_master";
        constexpr auto testProjectAccountName = "test_project";

        EXPECT_FALSE(userExists(testMasterAccountName));
        EXPECT_FALSE(userExists(testProjectAccountName));
        ASSERT_TRUE(userCreate(testMasterAccountName,
            masterAccountPassword));
        ASSERT_TRUE(userCreate(testProjectAccountName,
            projectAccountPassword));
        ASSERT_TRUE(userExists(testMasterAccountName));
        ASSERT_TRUE(userExists(testProjectAccountName));
        setMsiProperty("BOINC_MASTER_USERNAME", testMasterAccountName);
        setMsiProperty("BOINC_MASTER_PASSWORD", masterAccountPassword);
        setMsiProperty("BOINC_PROJECT_USERNAME", testProjectAccountName);
        setMsiProperty("BOINC_PROJECT_PASSWORD", projectAccountPassword);

        EXPECT_EQ(0u, executeAction());
        EXPECT_TRUE(userExists(testMasterAccountName));
        EXPECT_TRUE(userExists(testProjectAccountName));

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testMasterAccountName, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testProjectAccountName, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testMasterAccountName, value);

        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testProjectAccountName, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(masterAccountPassword, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(projectAccountPassword, value);

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("", value);

        EXPECT_FALSE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);

        if (userExists(testMasterAccountName)) {
            userDelete(testMasterAccountName);
        }
        if (userExists(testProjectAccountName)) {
            userDelete(testProjectAccountName);
        }

        TearDown();
    }
#endif
}
