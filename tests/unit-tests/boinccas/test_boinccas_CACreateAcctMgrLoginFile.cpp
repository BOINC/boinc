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

namespace test_boinccas_CACreateAcctMgrLoginFile {
    class test_boinccas_CACreateAcctMgrLoginFile : public test_boinccas_TestBase {
    protected:
        test_boinccas_CACreateAcctMgrLoginFile() :
            test_boinccas_TestBase("CreateAcctMgrLoginFile") {
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

        std::filesystem::path testDir;
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

        const auto* const loginElem = root->FirstChildElement("login");
        if (loginElem && loginElem->GetText()) {
            result.login = loginElem->GetText();
        }

        const auto* const hashElem = root->FirstChildElement("password_hash");
        if (hashElem && hashElem->GetText()) {
            result.passwordHash = hashElem->GetText();
        }

        return { true, result };
    }

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_DATADIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_DATADIR_Directory) {
        const auto dir = std::filesystem::current_path() /= "test_data";
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_DATADIR_Directory_And_ACCTMGR_LOGIN_Property_Set) {
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        Empty_ACCTMGR_LOGIN_Property) {
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
        {"DATADIR", testDir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateAcctMgrLoginFile,
        ACCTMGR_LOGIN_Property_Set_And_Empty_ACCTMGR_PASSWORDHASH_Property) {
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
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
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"},
        {"ACCTMGR_PASSWORDHASH", "abcd1234hashvalue"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, executeAction());
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
        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(testDir);
        const auto acctMgrLoginFile = testDir / "acct_mgr_login.xml";
        std::ofstream ofs(acctMgrLoginFile);
        ofs << "dummy content";
        ofs.close();
        insertMsiProperties({
        {"DATADIR", testDir.string().c_str()},
        {"ACCTMGR_LOGIN", "testuser"},
        {"ACCTMGR_PASSWORDHASH", "abcd1234hashvalue"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, executeAction());
        EXPECT_TRUE(std::filesystem::exists(acctMgrLoginFile));
        auto [parsed, accountData] =
            parseAccountXml(acctMgrLoginFile.string());
        EXPECT_TRUE(parsed);
        EXPECT_EQ("testuser", accountData.login);
        EXPECT_EQ("abcd1234hashvalue", accountData.passwordHash);
    }
#endif
}
