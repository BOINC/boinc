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

namespace test_boinccas_CACreateBOINCAccounts {
    enum class MsiNTProductType {
        Workstation = 1,
        DomainController = 2,
        Server = 3
    };

    using CreateBOINCAccountsFn = UINT(WINAPI*)(MSIHANDLE);

    class test_boinccas_CACreateBOINCAccounts :
        public ::testing::TestWithParam<MsiNTProductType> {
    protected:
        test_boinccas_CACreateBOINCAccounts() {
            std::tie(hDll, hFunc) =
                load_function_from_boinccas<CreateBOINCAccountsFn>(
                    "CreateBOINCAccounts");
        }

        void SetUp() override {
            const auto result =
                MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
            ASSERT_EQ(0u, result);

            msiHelper.insertProperties(hMsi,
                {
                    {"MsiNTProductType", productTypeString()}
                });

            ASSERT_EQ(productTypeString(),
                msiHelper.getProperty(hMsi, "MsiNTProductType"));
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

        std::string productTypeString() const {
            switch (GetParam()) {
            case MsiNTProductType::Workstation:
                return "1";
            case MsiNTProductType::DomainController:
                return "2";
            case MsiNTProductType::Server:
                return "3";
            default:
                throw std::runtime_error("Unknown MsiNTProductType");
            }
        }

        std::string getExpectedMasterAccountName() const {
            switch (GetParam()) {
            case MsiNTProductType::Workstation:
            case MsiNTProductType::Server:
                return "boinc_master";
            case MsiNTProductType::DomainController:
                return "boinc_master_test";
            default:
                throw std::runtime_error("Unknown MsiNTProductType");
            }
        }

        std::string getExpectedProjectAccountName() const {
            switch (GetParam()) {
            case MsiNTProductType::Workstation:
            case MsiNTProductType::Server:
                return "boinc_project";
            case MsiNTProductType::DomainController:
                return "boinc_project_test";
            default:
                throw std::runtime_error("Unknown MsiNTProductType");
            }
        }

        std::string masterAccountName;
        std::string projectAccountName;
        PMSIHANDLE hMsi;

    private:
        wil::unique_hmodule hDll = nullptr;
    };

#ifdef BOINCCAS_TEST
    INSTANTIATE_TEST_SUITE_P(MsiNTProductTypeTests,
        test_boinccas_CACreateBOINCAccounts,
        ::testing::Values(
            MsiNTProductType::Workstation,
            MsiNTProductType::DomainController,
            MsiNTProductType::Server
        )
    );

    TEST_P(test_boinccas_CACreateBOINCAccounts, CreateAccounts) {
        masterAccountName = getExpectedMasterAccountName();
        projectAccountName = getExpectedProjectAccountName();
        EXPECT_FALSE(userExists(masterAccountName));
        EXPECT_FALSE(userExists(projectAccountName));
        EXPECT_EQ(0u, hFunc(hMsi));
        EXPECT_TRUE(userExists(masterAccountName));
        EXPECT_TRUE(userExists(projectAccountName));
    }
#endif
}
