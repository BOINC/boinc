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

namespace test_boinccas_CACreateClientAuthFile {

    using CreateClientAuthFileFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CACreateClientAuthFile : public ::testing::Test {
    protected:
        test_boinccas_CACreateClientAuthFile() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<CreateClientAuthFileFn>(
                    "CreateClientAuthFile");
        }

        CreateClientAuthFileFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;
    };

    struct AccountData {
        std::string username;
        std::string password;
    };

    std::pair<bool, AccountData> parseAccountXml(const std::string& filename)
    {
        AccountData result;

        tinyxml2::XMLDocument doc;
        const auto err = doc.LoadFile(filename.c_str());
        if (err != tinyxml2::XML_SUCCESS) {
            return { false, result };
        }

        const auto* root = doc.FirstChildElement("client_authorization");
        if (!root) {
            return { false, result };
        }

        const auto* project = root->FirstChildElement("boinc_project");
        if (!project) {
            return { false, result };
        }

        const auto* const usernameElem =
            project->FirstChildElement("username");
        if (usernameElem && usernameElem->GetText()) {
            result.username = usernameElem->GetText();
        }

        const auto* const passwordElem =
            project->FirstChildElement("password");
        if (passwordElem && passwordElem->GetText()) {
            result.password = passwordElem->GetText();
        }

        return { true, result };
    }

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateClientAuthFile,
        Empty_DATADIR_Property) {
        PMSIHANDLE hMsi;
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, hFunc(hMsi));
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        DATADIR_Doesnt_Exist) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, hFunc(hMsi));
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionDisabled_No_XML_File_Created) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "0");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        EXPECT_EQ(0u, hFunc(hMsi));
        const auto authFile = dir / "client_auth.xml";
        EXPECT_FALSE(std::filesystem::exists(authFile));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionDisabled_Existing_XML_File_Removed) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        const auto authFile = dir / "client_auth.xml";
        std::filesystem::create_directory(dir);
        {
            std::ofstream ofs(authFile);
            ofs << "<test>data</test>";
        }
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "0");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_FALSE(std::filesystem::exists(authFile));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionDisabled_Existing_Opened_XML_File_Failed_To_Remove) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        const auto authFile = dir / "client_auth.xml";
        std::filesystem::create_directory(dir);
        std::ofstream ofs(authFile);
        ofs << "<test>data</test>";
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "0");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        EXPECT_NE(0u, hFunc(hMsi));
        EXPECT_TRUE(std::filesystem::exists(authFile));
        ofs.close();
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_No_Username_And_Password) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        EXPECT_NE(0u, hFunc(hMsi));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_Empty_Username) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_ISUSERNAME", "");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_NE(0u, hFunc(hMsi));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_Empty_Username_Valid_Password) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_ISUSERNAME", "");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", "password");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("password", value);

        EXPECT_NE(0u, hFunc(hMsi));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_Empty_Password) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", "");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_NE(0u, hFunc(hMsi));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_Empty_Password_ValidUsername) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_ISUSERNAME", "username");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("username", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", "");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_NE(0u, hFunc(hMsi));
        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_XML_File_Created) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directory(dir);
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_ISUSERNAME", "test_user");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test_user", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", "test_password");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test_password", value);

        EXPECT_EQ(0u, hFunc(hMsi));

        const auto authFile = dir / "client_auth.xml";
        ASSERT_TRUE(std::filesystem::exists(authFile));
        auto [parsed, accountData] = parseAccountXml(authFile.string());
        ASSERT_TRUE(parsed);
        EXPECT_EQ("test_user", accountData.username);
        EXPECT_EQ("dGVzdF9wYXNzd29yZA==", accountData.password);

        std::filesystem::remove_all(dir);
    }

    TEST_F(test_boinccas_CACreateClientAuthFile,
        ProtectionEnabled_XML_File_Overwritten) {
        PMSIHANDLE hMsi;
        const auto dir = std::filesystem::current_path() /= "test_data";
        const auto authFile = dir / "client_auth.xml";
        std::filesystem::create_directory(dir);
        {
            std::ofstream ofs(authFile);
            ofs << "<test>data</test>";
        }
        msiHelper.insertProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = MsiOpenPackage(msiHelper.getMsiHandle().c_str(),
            &hMsi);
        ASSERT_EQ(0u, result);

        msiHelper.setProperty(hMsi, "ENABLEPROTECTEDAPPLICATIONEXECUTION3",
            "1");
        auto [errorcode, value] =
            msiHelper.getProperty(hMsi,
                "ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_ISUSERNAME", "test_user");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_ISUSERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test_user", value);

        msiHelper.setProperty(hMsi, "BOINC_PROJECT_PASSWORD", "test_password");
        std::tie(errorcode, value) =
            msiHelper.getProperty(hMsi,
                "BOINC_PROJECT_PASSWORD");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test_password", value);

        EXPECT_EQ(0u, hFunc(hMsi));

        ASSERT_TRUE(std::filesystem::exists(authFile));
        auto [parsed, accountData] = parseAccountXml(authFile.string());
        ASSERT_TRUE(parsed);
        EXPECT_EQ("test_user", accountData.username);
        EXPECT_EQ("dGVzdF9wYXNzd29yZA==", accountData.password);

        std::filesystem::remove_all(dir);
    }

#endif
}
