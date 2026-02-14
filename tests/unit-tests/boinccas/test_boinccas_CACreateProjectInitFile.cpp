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
#include "project_init.h"

namespace test_boinccas_CACreateProjectInitFile {
    class test_boinccas_CACreateProjectInitFile :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CACreateProjectInitFile() :
            test_boinccas_TestBase("CreateProjectInitFile") {
        }

        void TearDown() override {
            const auto temp_file =
                std::filesystem::current_path() /= "login_token.txt";
            if (std::filesystem::exists(temp_file)) {
                std::filesystem::remove(temp_file);
            }
            if (std::filesystem::exists(dir)) {
                std::filesystem::remove_all(dir);
            }
        }

        const std::filesystem::path dir =
            std::filesystem::current_path() /= "test_data";
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateProjectInitFile,
        Empty_DATADIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        DATADIR_Doesnt_Exist) {
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_Empty_No_XML_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        EXPECT_FALSE(std::filesystem::exists(authFile));
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_Not_Empty_And_No_PROJINIT_AUTH_No_XML_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "http://test.com/");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("http://test.com/", value);

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        EXPECT_FALSE(std::filesystem::exists(authFile));
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_Not_Empty_And_PROJINIT_AUTH_Empty_No_XML_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "http://test.com/");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("http://test.com/", value);

        setMsiProperty("PROJINIT_AUTH", "");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_AUTH");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        EXPECT_FALSE(std::filesystem::exists(authFile));
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_And_PROJINIT_AUTH_Not_Empty_And_No_PROJINIT_NAME) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "http://test.com/");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("http://test.com/", value);

        setMsiProperty("PROJINIT_AUTH", "0123abcd");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_AUTH");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0123abcd", value);

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        ASSERT_TRUE(std::filesystem::exists(authFile));

        const auto prev_dir = std::filesystem::current_path();
        std::filesystem::current_path(dir);
        PROJECT_INIT pi;
        pi.init();
        EXPECT_EQ("http://test.com/", std::string(pi.url));
        EXPECT_EQ("0123abcd", std::string(pi.account_key));
        EXPECT_EQ("http://test.com/", std::string(pi.name));
        EXPECT_FALSE(pi.embedded);
        std::filesystem::current_path(prev_dir);
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_And_PROJINIT_AUTH_Not_Empty_PROJINIT_NAME_Empty) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "http://test.com/");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("http://test.com/", value);

        setMsiProperty("PROJINIT_AUTH", "0123abcd");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_AUTH");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0123abcd", value);

        setMsiProperty("PROJINIT_NAME", "");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_NAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        ASSERT_TRUE(std::filesystem::exists(authFile));

        const auto prev_dir = std::filesystem::current_path();
        std::filesystem::current_path(dir);
        PROJECT_INIT pi;
        pi.init();
        EXPECT_EQ("http://test.com/", std::string(pi.url));
        EXPECT_EQ("0123abcd", std::string(pi.account_key));
        EXPECT_EQ("http://test.com/", std::string(pi.name));
        EXPECT_FALSE(pi.embedded);
        std::filesystem::current_path(prev_dir);
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        PROJINIT_URL_And_PROJINIT_AUTH_And_PROJINIT_NAME_Not_Empty) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("PROJINIT_URL", "http://test.com/");
        auto [errorcode, value] = getMsiProperty("PROJINIT_URL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("http://test.com/", value);

        setMsiProperty("PROJINIT_AUTH", "0123abcd");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_AUTH");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0123abcd", value);

        setMsiProperty("PROJINIT_NAME", "Test");
        std::tie(errorcode, value) = getMsiProperty("PROJINIT_NAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("Test", value);

        EXPECT_EQ(0u, executeAction());
        const auto authFile = dir / "project_init.xml";
        ASSERT_TRUE(std::filesystem::exists(authFile));

        const auto prev_dir = std::filesystem::current_path();
        std::filesystem::current_path(dir);
        PROJECT_INIT pi;
        pi.init();
        EXPECT_EQ("http://test.com/", std::string(pi.url));
        EXPECT_EQ("0123abcd", std::string(pi.account_key));
        EXPECT_EQ("Test", std::string(pi.name));
        EXPECT_FALSE(pi.embedded);
        std::filesystem::current_path(prev_dir);
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        SETUPEXENAME_Doesnt_Exist_No_TXT_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
        const auto loginFile = dir / "login_token.txt";
        EXPECT_FALSE(std::filesystem::exists(loginFile));
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        SETUPEXENAME_Empty_No_TXT_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("SETUPEXENAME", "");
        auto [errorcode, value] = getMsiProperty("SETUPEXENAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_TRUE(value.empty());

        EXPECT_EQ(0u, executeAction());
        const auto loginFile = dir / "login_token.txt";
        EXPECT_FALSE(std::filesystem::exists(loginFile));
    }

    TEST_F(test_boinccas_CACreateProjectInitFile,
        SETUPEXENAME_Set_And_TXT_File_Created) {
        std::filesystem::create_directory(dir);
        insertMsiProperties({
        {"DATADIR", dir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("SETUPEXENAME", "setup.exe");
        auto [errorcode, value] = getMsiProperty("SETUPEXENAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("setup.exe", value);

        EXPECT_EQ(0u, executeAction());
        const auto loginFile = dir / "login_token.txt";
        ASSERT_TRUE(std::filesystem::exists(loginFile));

        std::ifstream lf(loginFile);
        ASSERT_TRUE(lf.is_open());
        std::string data;
        ASSERT_TRUE(lf >> data);
        EXPECT_EQ("setup.exe", data);
    }

#endif
}
