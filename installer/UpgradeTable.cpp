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

#include "UpgradeTable.h"

UpgradeTable::UpgradeTable(const nlohmann::json& json,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading UpgradeTable..." << std::endl;

    for (const auto& item : json) {
        values.emplace_back(item);
    }

    const auto tableName = std::string("Upgrade");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/upgrade-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "UpgradeCode",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryGuid,
            "",
            DescriptionWithUrl("The UpgradeCode property in this column "
                "specifies the upgrade code of all products that are to be "
                "detected by the FindRelatedProducts action.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "VersionMin",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("Lower boundary of the range of product "
                "versions detected by FindRelatedProducts.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "VersionMax",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("Upper boundary of the range of product "
                "versions detected by the FindRelatedProducts action.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Language",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The set of languages detected by "
                "FindRelatedProducts.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Attributes",
            false,
            0,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column contains bit flags specifying "
                "attributes of the Upgrade table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Remove",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The installer sets the REMOVE property to "
                "features specified in this column.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ActionProperty",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("When the FindRelatedProducts action detects a "
                "related product installed on the system, it appends the "
                "product code to the property specified in this field.", url)
        ));
    }
}

bool UpgradeTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating UpgradeTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Upgrade` "
        "(`UpgradeCode` CHAR(38) NOT NULL, `VersionMin` CHAR(20), "
        "`VersionMax` CHAR(20), `Language` CHAR(255), "
        "`Attributes` LONG NOT NULL, `Remove` CHAR(255), "
        "`ActionProperty` CHAR(72) NOT NULL PRIMARY KEY `UpgradeCode`, "
        "`VersionMin`, `VersionMax`, `Language`, `Attributes`)";
    const auto sql_insert = "INSERT INTO `Upgrade` (`UpgradeCode`, "
        "`VersionMin`, `VersionMax`, `Language`, `Attributes`, `Remove`, "
        "`ActionProperty`) VALUES (?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
