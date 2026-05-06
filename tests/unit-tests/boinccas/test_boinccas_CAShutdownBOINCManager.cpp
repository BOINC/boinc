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

namespace test_boinccas_CAShutdownBOINCManager {
    constexpr auto executableName = "boincmgr.exe";
    class test_boinccas_CAShutdownBOINCManager :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAShutdownBOINCManager() :
            test_boinccas_TestBase("ShutdownBOINCManager") {
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
    TEST_F(test_boinccas_CAShutdownBOINCManager, TestShutdown) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy_gui.exe",
            testDir / executableName));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            if (std::filesystem::exists(testDir / executableName)) {
                executableFound = true;
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        STARTUPINFO si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        ASSERT_TRUE(CreateProcess(
            (testDir / executableName).string().c_str(), nullptr, nullptr,
            nullptr, FALSE, 0, nullptr, nullptr, &si, &pi));
        wil::unique_handle processHandle(pi.hProcess);
        wil::unique_handle threadHandle(pi.hThread);

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto processFound = true;
        for (auto i = 0u; i < 5u; ++i) {
            if (!isProcessRunning(executableName)) {
                processFound = false;
                break;
            }
            Sleep(1000);
        }
        EXPECT_FALSE(processFound);
    }
#endif
}
