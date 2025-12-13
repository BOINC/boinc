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

#include <MsiQuery.h>

namespace test_boinccas_CACreateBOINCAccounts {
    constexpr auto masterAccountName = "boinc_master";
    constexpr auto projectAccountName = "boinc_project";
    constexpr auto masterAccountPassword = "qwerty123456!@#$%^";
    constexpr auto projectAccountPassword = "ytrewq654321^%$#@!";
    constexpr auto testPCName = "testpc";

    using CreateBOINCAccountsFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CACreateBOINCAccounts :
        public ::testing::TestWithParam<std::string_view> {
    protected:
        test_boinccas_CACreateBOINCAccounts() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<CreateBOINCAccountsFn>(
                    "CreateBOINCAccounts");
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

        CreateBOINCAccountsFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;
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
        PMSIHANDLE hMsi;
        const auto result =
            MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "MsiNTProductType", GetParam().data());
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi, "MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        msiHelper.setProperty(hMsi, "ComputerName", testPCName);
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "ComputerName");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testPCName, value);

        EXPECT_FALSE(userExists(getMasterAccountName()));
        EXPECT_FALSE(userExists(getProjectAccountName()));
        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(getMasterAccountName()));
        EXPECT_TRUE(userExists(getProjectAccountName()));

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getMasterAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getProjectAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getMasterAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getProjectAccountName(), value);

        std::string masterPassword;
        std::tie(errorcode, masterPassword) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(33, masterPassword.length());
        EXPECT_NE("", masterPassword);

        std::string projectPassword;
        std::tie(errorcode, projectPassword) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(33, projectPassword.length());
        EXPECT_NE("", projectPassword);

        EXPECT_NE(masterPassword, projectPassword);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);

        TearDown();
    }

    TEST_P(test_boinccas_CACreateBOINCAccounts, ChangePasswords) {
        PMSIHANDLE hMsi;
        const auto result =
            MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "MsiNTProductType", GetParam().data());
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi, "MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        msiHelper.setProperty(hMsi, "ComputerName", testPCName);
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "ComputerName");
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
        msiHelper.setProperty(
            hMsi, "BOINC_MASTER_USERNAME", getMasterAccountName());
        msiHelper.setProperty(
            hMsi, "BOINC_PROJECT_USERNAME", getProjectAccountName());

        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(getMasterAccountName()));
        EXPECT_TRUE(userExists(getProjectAccountName()));

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getMasterAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getProjectAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getMasterAccountName(), value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(".\\" + getProjectAccountName(), value);

        std::string masterPassword;
        std::tie(errorcode, masterPassword) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(masterAccountPassword, masterPassword);
        EXPECT_EQ(33, masterPassword.length());

        std::string projectPassword;
        std::tie(errorcode, projectPassword) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(projectAccountPassword, projectPassword);
        EXPECT_EQ(33, projectPassword.length());

        EXPECT_NE(masterPassword, projectPassword);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("", value);

        EXPECT_FALSE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);

        TearDown();
    }

    TEST_P(test_boinccas_CACreateBOINCAccounts, DontChangeExistingAccounts) {
        PMSIHANDLE hMsi;
        const auto result =
            MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "MsiNTProductType", GetParam().data());
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi, "MsiNTProductType");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(GetParam(), value);

        msiHelper.setProperty(hMsi, "ComputerName", testPCName);
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "ComputerName");
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
        msiHelper.setProperty(hMsi, "BOINC_MASTER_USERNAME",
            testMasterAccountName);
        msiHelper.setProperty(hMsi, "BOINC_MASTER_PASSWORD",
            masterAccountPassword);
        msiHelper.setProperty(hMsi, "BOINC_PROJECT_USERNAME",
            testProjectAccountName);
        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD",
            projectAccountPassword);

        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(testMasterAccountName));
        EXPECT_TRUE(userExists(testProjectAccountName));

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testMasterAccountName, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testProjectAccountName, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testMasterAccountName, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(testProjectAccountName, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(masterAccountPassword, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(projectAccountPassword, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("", value);

        EXPECT_FALSE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);

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
