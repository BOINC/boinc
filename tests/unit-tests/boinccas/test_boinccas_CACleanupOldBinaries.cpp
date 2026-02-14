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

namespace test_boinccas_CACleanupOldBinaries {
    class test_boinccas_CACleanupOldBinaries : public test_boinccas_TestBase {
    protected:
        test_boinccas_CACleanupOldBinaries() :
            test_boinccas_TestBase("CleanupOldBinaries") {
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

        void createTestFilesInDir(const std::filesystem::path& dirPath) {
            createDummyFile(dirPath / "boinc.exe");
            createDummyFile(dirPath / "boincmgr.exe");
            createDummyFile(dirPath / "boinccmd.exe");
            createDummyFile(dirPath / "boinc.dll");
            createDummyFile(dirPath / "libcurl.dll");
            createDummyFile(dirPath / "libeay32.dll");
            createDummyFile(dirPath / "ssleay32.dll");
            createDummyFile(dirPath / "zlib1.dll");
            createDummyFile(dirPath / "dbghelp.dll");
            createDummyFile(dirPath / "dbghelp95.dll");
            createDummyFile(dirPath / "srcsrv.dll");
            createDummyFile(dirPath / "symsrv.dll");
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACleanupOldBinaries,
        Empty_INSTALLDIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CACleanupOldBinaries,
        NonExistent_INSTALLDIR_Directory) {
        const auto dir =
            std::filesystem::current_path() /= "non_existent_directory";
        insertMsiProperties({
            {"INSTALLDIR", dir.string().c_str()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CACleanupOldBinaries,
        Empty_INSTALLDIR_Directory) {
        testDir = std::filesystem::current_path() /= "empty";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
            {"INSTALLDIR", testDir.string().c_str()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CACleanupOldBinaries,
        NonEmpty_INSTALLDIR_Directory_RemoveAllListedFiles) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);
        createTestFilesInDir(testDir);

        insertMsiProperties({
            {"INSTALLDIR", testDir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, executeAction());
        // verify that the directory is now empty
        EXPECT_TRUE(std::filesystem::is_empty(testDir));
    }

    TEST_F(test_boinccas_CACleanupOldBinaries,
        NonEmpty_INSTALLDIR_Directory_KeepUnListedFiles) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);
        createTestFilesInDir(testDir);
        createDummyFile(testDir / "dummy.bin");
        createDummyFile(testDir / "dummy.exe");
        createDummyFile(testDir / "dummy.dll");

        insertMsiProperties({
            {"INSTALLDIR", testDir.string().c_str()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, executeAction());
        // verify that the directory is now empty
        EXPECT_FALSE(std::filesystem::is_empty(testDir));
        EXPECT_TRUE(std::filesystem::exists(testDir / "dummy.bin"));
        EXPECT_TRUE(std::filesystem::exists(testDir / "dummy.exe"));
        EXPECT_TRUE(std::filesystem::exists(testDir / "dummy.dll"));
    }
#endif
}
