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

        void SetUp() override {
        }

        void TearDown() override {
        }

        CreateClientAuthFileFn hFunc = nullptr;
        MsiHelper msiHelper;
    private:
        wil::unique_hmodule hDll = nullptr;
    };

#ifndef BOINCCAS_TEST

#endif
}
