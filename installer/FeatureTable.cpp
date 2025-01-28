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

#include "FeatureTable.h"

FeatureTable::FeatureTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading FeatureTable..." << std::endl;
    for (const auto& feature : json) {
        features.emplace_back(feature, "", installerStrings);
    }

    const auto tableName = std::string("Feature");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/feature-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Feature",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The primary key that is used to identify a "
                "specific feature record.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Feature_Parent",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Feature",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("An optional key of a parent record in the "
                "same table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Title",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("A short string of text that identifies a "
                "feature.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Description",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("A longer string of text that describes a "
                "feature.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Display",
            true,
            0,
            3276,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The number in this field specifies the order "
                "in which the feature is to be displayed in the user "
                "interface.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Level",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The initial installation level of this "
                "feature.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Directory_",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Directory",
            1,
            ValidationCategoryUpperCase,
            "",
            DescriptionWithUrl("The Directory_ column specifies the name of a "
                "directory that can be configured by a Selection Dialog.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Attributes",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            "",
            "0;1;2;4;5;6;8;9;10;16;17;18;20;21;22;24;25;26;32;33;34;36;37;38;"
            "48;49;50;52;53;54",
            DescriptionWithUrl("The remote execution option for features that "
                "are not installed and for which no feature state request is "
                "made by using any of the following properties.", url)
        ));
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
