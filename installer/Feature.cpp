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

#include "Feature.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

Feature::Feature(const nlohmann::json& json, const std::string& parent,
    InstallerStrings& installerStrings) : feature_parent(parent) {
    JsonHelper::get(json, "Feature", feature);
    JsonHelper::get(json, "Title", title, installerStrings);
    JsonHelper::get(json, "Description", description, installerStrings);
    JsonHelper::get(json, "Display", display);
    JsonHelper::get(json, "Level", level);
    JsonHelper::get(json, "Directory_", directory);
    JsonHelper::get(json, "Attributes", attributes);
    JsonHelper::handle(json, "Features", [&](const auto& f) {
        features.emplace_back(f, feature, installerStrings);
        });
}

MSIHANDLE Feature::getRecord() const {
    return MsiHelper::MsiRecordSet({ feature, feature_parent, title,
        description, display, level, directory, attributes });
}

std::vector<Feature> Feature::getFeatures() const {
    std::vector<Feature> result;
    for (const auto& f : features) {
        result.push_back(f);
        auto subFeatures = f.getFeatures();
        result.insert(result.end(), subFeatures.begin(), subFeatures.end());
    }
    return result;
}
