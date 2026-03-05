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

namespace test_boinccas_CALaunchBOINCManager {
    constexpr auto executableName = "boincmgr.exe";
    class test_boinccas_CALaunchBOINCManager : public test_boinccas_TestBase {
    protected:
        test_boinccas_CALaunchBOINCManager() :
            test_boinccas_TestBase("LaunchBOINCManager") {
        }

        void TearDown() override {
            const auto processId = findProcessByName(executableName);
            if (processId != 0) {
                EXPECT_TRUE(killProcessById(processId));
            }
            for (auto i = 0u; i < 5u; ++i) {
                if (!isProcessRunning(executableName)) {
                    break;
                }
                Sleep(1000);
            }
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        DWORD findProcessByName(const std::string& processName) {
            const auto snapshot =
                CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) {
                return 0;
            }
            wil::unique_handle snapshotHandle(snapshot);

            PROCESSENTRY32 processEntry;
            processEntry.dwSize = sizeof(PROCESSENTRY32);
            if (!Process32First(snapshot, &processEntry)) {
                return 0;
            }
            do {
                if (processName == processEntry.szExeFile) {
                    return processEntry.th32ProcessID;
                }
            } while (Process32Next(snapshot, &processEntry));
            return 0;
        }

        bool isProcessRunning(const std::string& processName) {
            return findProcessByName(processName) != 0;
        }

        bool killProcessById(DWORD processId) {
            const auto processHandle =
                OpenProcess(PROCESS_TERMINATE, FALSE, processId);
            if (!processHandle) {
                return false;
            }
            wil::unique_handle processHandleWrapper(processHandle);
            return TerminateProcess(processHandleWrapper.get(), 0) == TRUE;
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CALaunchBOINCManager,
        Empty_INSTALLDIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCManager,
        NonExistent_INSTALLDIR_Directory) {
        const auto dir =
            std::filesystem::current_path() /= "non_existent_directory";
        insertMsiProperties({
            {"INSTALLDIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCManager,
        Empty_INSTALLDIR_Directory) {
        testDir = std::filesystem::current_path() /= "empty";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCManager,
        NonEmpty_INSTALLDIR_Directory_LaunchBOINCManager) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("C:\\Windows\\System32\\calc.exe",
            testDir / executableName));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            executableFound = std::filesystem::exists(testDir / executableName);
            if (executableFound) {
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());
        auto processFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            if (isProcessRunning(executableName)) {
                processFound = true;
                break;
            }
            Sleep(1000);
        }
        EXPECT_TRUE(processFound);
    }
#endif
}
