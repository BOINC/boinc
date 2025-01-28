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

#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <filesystem>

#include "nlohmann/json.hpp"

class InstallerStrings {
public:
    InstallerStrings() noexcept = default;
    ~InstallerStrings();

    const std::string& get(const std::string& key);
    bool load(const nlohmann::json& json, const std::filesystem::path& path);
private:
    std::map<std::string, std::string> strings{};
    std::unordered_set<std::string> keys_used{};

    bool load_from_json(const nlohmann::json& json);
};

