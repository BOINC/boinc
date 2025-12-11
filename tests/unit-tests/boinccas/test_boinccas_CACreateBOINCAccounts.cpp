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

        EXPECT_FALSE(userExists(masterAccountName));
        EXPECT_FALSE(userExists(projectAccountName));
        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(masterAccountName));
        EXPECT_TRUE(userExists(projectAccountName));

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_TRUE(value._Starts_with(
                masterAccountName + std::string("_")));
        }
        else {
            EXPECT_EQ(masterAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_TRUE(value._Starts_with(
                projectAccountName + std::string("_")));
        }
        else {
            EXPECT_EQ(projectAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_TRUE(value._Starts_with(
                std::string(".\\") + masterAccountName + std::string("_")));
        }
        else {
            EXPECT_EQ(std::string(".\\") + masterAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_TRUE(value._Starts_with(
                std::string(".\\") + projectAccountName + std::string("_")));
        }
        else {
            EXPECT_EQ(std::string(".\\") + projectAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE("", value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE("", value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);
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

        EXPECT_FALSE(userExists(masterAccountName));
        EXPECT_FALSE(userExists(projectAccountName));
        ASSERT_TRUE(userCreate(masterAccountName, masterAccountPassword));
        ASSERT_TRUE(userCreate(projectAccountName, projectAccountPassword));
        ASSERT_TRUE(userExists(masterAccountName));
        ASSERT_TRUE(userExists(projectAccountName));
        msiHelper.setProperty(
            hMsi, "BOINC_MASTER_USERNAME", masterAccountName);
        msiHelper.setProperty(
            hMsi, "BOINC_PROJECT_USERNAME", projectAccountName);

        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(masterAccountName));
        EXPECT_TRUE(userExists(projectAccountName));

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_EQ(masterAccountName + std::string("_") + testPCName,
                value);
        }
        else {
            EXPECT_EQ(masterAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_EQ(projectAccountName + std::string("_") + testPCName,
                value);
        }
        else {
            EXPECT_EQ(projectAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_EQ(std::string(".\\") + masterAccountName +
                std::string("_") + testPCName, value);
        }
        else {
            EXPECT_EQ(std::string(".\\") + masterAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        if (GetParam() == "2") {
            EXPECT_EQ(std::string(".\\") + projectAccountName +
                std::string("_") + testPCName, value);
        }
        else {
            EXPECT_EQ(std::string(".\\") + projectAccountName, value);
        }

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_MASTER_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(masterAccountPassword, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_NE(projectAccountPassword, value);

        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi, "RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("", value);

        EXPECT_FALSE(MsiGetMode(hMsi, MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(hMsi, MSIRUNMODE_REBOOTATEND, FALSE);
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
        ASSERT_TRUE(userCreate(testMasterAccountName, masterAccountPassword));
        ASSERT_TRUE(userCreate(testProjectAccountName, projectAccountPassword));
        ASSERT_TRUE(userExists(testMasterAccountName));
        ASSERT_TRUE(userExists(testProjectAccountName));
        msiHelper.setProperty(hMsi, "BOINC_MASTER_USERNAME", testMasterAccountName);
        msiHelper.setProperty(hMsi, "BOINC_MASTER_PASSWORD", masterAccountPassword);
        msiHelper.setProperty(hMsi, "BOINC_PROJECT_USERNAME", testProjectAccountName);
        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", projectAccountPassword);

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
    }
#endif
}
