// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

#include <fstream>
#include <iostream>

#include "JsonHelper.h"
#include "InstallerStrings.h"

const std::string& InstallerStrings::get(const std::string& key) const {
    if (strings.find(key) == strings.end()) {
        return key;
    }
    return strings.at(key);
};
bool InstallerStrings::load(const std::filesystem::path& path) {
    const auto filename = path / "locale/en.json";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << filename << std::endl;
        return false;
    }
    nlohmann::json j;
    file >> j;
    return load_from_json(j);
}
bool InstallerStrings::load_from_json(const nlohmann::json& json) {
    for (const auto& item : json) {
        std::string id{};
        std::string value{};
        JsonHelper::get(item, "Id", id);
        JsonHelper::get(item, "Value", value);
        if (!id.empty()) {
            strings.emplace(id, value);
        }
        else {
            std::cerr << "WARNING: Skipped record with no Id specified."
                << std::endl;
        }
    }
    return true;
}
