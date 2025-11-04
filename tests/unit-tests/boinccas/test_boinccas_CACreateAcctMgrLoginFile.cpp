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
#include "tinyxml2.h"

#include "boinccas_helper.h"

using namespace std;

namespace test_boinccas_CACreateAcctMgrLoginFile {
    using CreateAcctMgrLoginFileFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CACreateAcctMgrLoginFile : public ::testing::Test {
    protected:
        test_boinccas_CACreateAcctMgrLoginFile() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<CreateAcctMgrLoginFileFn>(
                    "CreateAcctMgrLoginFile");
        }

        void TearDown() override {
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        void createDummyFile(const std::filesystem::path& filePath) {
            std::ofstream ofs(filePath);
            ofs << "dummy content";
            ofs.close();
        }

        CreateAcctMgrLoginFileFn hFunc = nullptr;
        MsiHelper msiHelper;
        std::filesystem::path testDir;
    private:
        wil::unique_hmodule hDll = nullptr;
    };

    struct AccountData {
        std::string login;
        std::string passwordHash;
    };

    std::pair<bool, AccountData> parseAccountXml(const std::string& filename)
    {
        AccountData result;

        tinyxml2::XMLDocument doc;
        const auto err = doc.LoadFile(filename.c_str());
        if (err != tinyxml2::XML_SUCCESS) {
            return { false, result };
        }

        auto* root = doc.FirstChildElement("acct_mgr_login");
        if (!root) {
            return { false, result };
        }

        auto* loginElem = root->FirstChildElement("login");
        if (loginElem && loginElem->GetText()) {
            result.login = loginElem->GetText();
        }

        auto* hashElem = root->FirstChildElement("password_hash");
        if (hashElem && hashElem->GetText()) {
            result.passwordHash = hashElem->GetText();
        }

        return { true, result };
    }

#ifndef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_DATADIR_Property) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_DATADIR_Directory) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
    }

    //TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
    //    Empty_DATADIR_Directory_And_ACCTMGR_LOGIN_Property_Set) {
    //    PMSIHANDLE hMsi;
    //    const auto dir = std::filesystem::current_path() /= "test_data";
    //    msiHelper.insertProperties({
    //    {"DATADIR", dir.string().c_str()},
    //    {"ACCTMGR_LOGIN", "testuser"}
    //        });
    //    const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
    //        &hMsi);
    //    ASSERT_EQ(0u, result);

    //    EXPECT_EQ(0u, hFunc(hMsi));
    //}

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_ACCTMGR_LOGIN_Property) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        ACCTMGR_LOGIN_Property_Set_And_Empty_ACCTMGR_PASSWORDHASH_Property) {
        PMSIHANDLE hMsi;
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        msiHelper.insertProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, hFunc(hMsi));
        const auto acctMgrLoginFile = testDir / "acct_mgr_login.xml";
        EXPECT_TRUE(std::filesystem::exists(acctMgrLoginFile));
        auto [parsed, accountData] =
            parseAccountXml(acctMgrLoginFile.string());
        EXPECT_TRUE(parsed);
        EXPECT_EQ("testuser", accountData.login);
        EXPECT_TRUE(accountData.passwordHash.empty());
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        ACCTMGR_LOGIN_And_ACCTMGR_PASSWORDHASH_Properties_Set) {
        PMSIHANDLE hMsi;
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        msiHelper.insertProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"},
        {"ACCTMGR_PASSWORDHASH", "abcd1234hashvalue"}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, hFunc(hMsi));
        const auto acctMgrLoginFile = testDir / "acct_mgr_login.xml";
        EXPECT_TRUE(std::filesystem::exists(acctMgrLoginFile));
        auto [parsed, accountData] =
            parseAccountXml(acctMgrLoginFile.string());
        EXPECT_TRUE(parsed);
        EXPECT_EQ("testuser", accountData.login);
        EXPECT_EQ("abcd1234hashvalue", accountData.passwordHash);
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        acct_mgr_login_xml_overwrite) {
        PMSIHANDLE hMsi;
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        const auto acctMgrLoginFile = testDir / "acct_mgr_login.xml";
        std::ofstream ofs(acctMgrLoginFile);
        ofs << "dummy content";
        ofs.close();
        msiHelper.insertProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"},
        {"ACCTMGR_PASSWORDHASH", "abcd1234hashvalue"}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(std::filesystem::exists(acctMgrLoginFile));
        auto [parsed, accountData] =
            parseAccountXml(acctMgrLoginFile.string());
        EXPECT_TRUE(parsed);
        EXPECT_EQ("testuser", accountData.login);
        EXPECT_EQ("abcd1234hashvalue", accountData.passwordHash);
    }
#endif
}
