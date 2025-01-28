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

#include <fstream>
#include <iostream>

#include "JsonHelper.h"
#include "InstallerStrings.h"

InstallerStrings::~InstallerStrings() {
    for (const auto& key : strings) {
        if (keys_used.find(key.first) == keys_used.end()) {
            std::cerr << "WARNING: Key " << key.first << " not used."
                << std::endl;
        }
    }
}

const std::string& InstallerStrings::get(const std::string& key) {
    if (!key._Starts_with("IDS_")) {
        return key;
    }
    if (strings.find(key) == strings.end()) {
        std::cerr << "WARNING: Key " << key << " not found." << std::endl;
        return key;
    }
    keys_used.insert(key);
    return strings.at(key);
};
bool InstallerStrings::load(const nlohmann::json& json,
    const std::filesystem::path& path) {
    std::string en_path{};
    for (const auto& item : json) {
        std::string p{};
        std::string language{};
        std::string title{};
        JsonHelper::get(item, "Path", p);
        JsonHelper::get(item, "Language", language);
        JsonHelper::get(item, "Title", title);

        if (language == "en") {
            en_path = p;
        }
        else {
            std::cerr << "WARNING: Unsupported language " << language
                << std::endl;
        }

    }

    if (en_path.empty()) {
        std::cerr << "No English locale specified." << std::endl;
        return false;
    }

    const auto filename = path / en_path;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << filename << std::endl;
        return false;
    }
    nlohmann::json j;
    file >> j;
    const auto result = load_from_json(j);
    file.close();
    return result;
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
