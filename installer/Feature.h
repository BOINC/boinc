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

#include <nlohmann/json.hpp>

#include "Record.h"
#include "InstallerStrings.h"

class Feature : public Record {
public:
    explicit Feature(const nlohmann::json& json, const std::string& parent,
        InstallerStrings& installerStrings);
    ~Feature() = default;
    MSIHANDLE getRecord() const override;
    std::vector<Feature> getFeatures() const;
private:
    std::string feature{};
    std::string feature_parent{};
    std::string title{};
    std::string description{};
    int display = MSI_NULL_INTEGER;
    int level = MSI_NULL_INTEGER;
    std::string directory{};
    int attributes = MSI_NULL_INTEGER;
    std::vector<Feature> features{};
};
