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

#include <map>
#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "Generator.h"
#include "InstallerStrings.h"

class Installer {
public:
    explicit Installer(const std::filesystem::path& output_path,
        const std::string& platform, const std::string& configuration);
    ~Installer() = default;
    bool load(const std::filesystem::path& json);
    bool create_msi(const std::filesystem::path& msi);
private:
    bool load_from_json(const nlohmann::json& json,
        const std::filesystem::path& path);
    nlohmann::json load_json(const std::filesystem::path& json);
    void process_json(nlohmann::json& json,
        const std::filesystem::path& parent_file);
    bool forceCodePage(MSIHANDLE hDatabase);

    std::map<std::string, std::shared_ptr<GeneratorTable>> tables{};
    InstallerStrings installer_strings;
    std::filesystem::path output_path{};
    std::string platform;
    std::string configuration;
};
