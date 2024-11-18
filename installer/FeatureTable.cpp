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

#include "FeatureTable.h"

FeatureTable::FeatureTable(const nlohmann::json& json,
    const InstallerStrings& installerStrings) {
    std::cout << "Loading FeatureTable..." << std::endl;
    for (const auto& feature : json) {
        features.emplace_back(feature, "", installerStrings);
    }
}

bool FeatureTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating FeatureTable..." << std::endl;

    std::vector<Feature> all;
    for (const auto& feature : features) {
        all.push_back(feature);
        for (const auto& component : feature.getFeatures()) {
            all.push_back(component);
        }
    }

    const auto sql_create = "CREATE TABLE `Feature` "
        "(`Feature` CHAR(38) NOT NULL, `Feature_Parent` CHAR(38), "
        "`Title` CHAR(64) LOCALIZABLE, `Description` CHAR(255) LOCALIZABLE, "
        "`Display` SHORT, `Level` SHORT NOT NULL, `Directory_` CHAR(72), "
        "`Attributes` SHORT NOT NULL PRIMARY KEY `Feature`)";
    const auto sql_insert = "INSERT INTO `Feature` (`Feature`, "
        "`Feature_Parent`, `Title`, `Description`, `Display`, `Level`, "
        "`Directory_`, `Attributes`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, all);
}
