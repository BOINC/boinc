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

#include "CustomActionTable.h"

CustomActionTable::CustomActionTable(const nlohmann::json& json,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading CustomActionTable..." << std::endl;
    for (const auto& customAction : json) {
        customActions.emplace_back(customAction);
    }

    const auto tableName = std::string("CustomAction");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/customaction-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Action",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Name of the action.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Type",
            false,
            1,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A field of flag bits specifying the basic "
                "type of custom action and options.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Source",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCustomSource,
            "",
            DescriptionWithUrl("A property name or external key into another "
                "table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Target",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("An execution parameter that depends on the "
                "basic type of custom action.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ExtendedType",
            true,
            0,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The extended type of the action.", url)
        ));
    }
}

bool CustomActionTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating CustomActionTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `CustomAction` "
        "(`Action` CHAR(72) NOT NULL, `Type` SHORT NOT NULL, "
        "`Source` CHAR(64), `Target` CHAR(255), `ExtendedType` LONG "
        "PRIMARY KEY `Action`)";
    const auto sql_insert = "INSERT INTO `CustomAction` (`Action`, `Type`, "
        "`Source`, `Target`, `ExtendedType`) VALUES (?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert,
        customActions);
}
