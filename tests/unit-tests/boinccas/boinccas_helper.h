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

#pragma once

#include <string>
#include "wil/resource.h"
#include <Msi.h>

template <typename F>
std::pair<wil::unique_hmodule, F> load_function_from_boinccas(
    const std::string& function_name) {
    wil::unique_hmodule dll(LoadLibrary("boinccas.dll"));
    if (!dll) {
        throw std::runtime_error("Failed to load boinccas.dll");
    }
    auto func = reinterpret_cast<F>(GetProcAddress(dll.get(),
        function_name.c_str()));
    if (!func) {
        throw std::runtime_error("Failed to load function: " + function_name);
    }
    return { std::move(dll), func };
}

std::string getRegistryValue(const std::string& valueName);
void cleanRegistryKey();

constexpr auto msiName = "test.msi";

class MsiHelper {
public:
    MsiHelper();
    ~MsiHelper();
    void createPropertiesTable();
    void insertProperties(
        const std::vector<std::pair<std::string, std::string>>& properties);
    std::string getMsiHandle() const {
        return "#" + std::to_string(hMsi);
    }

private:
    void cleanup();
    void createTable(const std::string_view& sql_create);
    MSIHANDLE hMsi = 0;
};
