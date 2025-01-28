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

#include "CheckboxTable.h"

CheckboxTable::CheckboxTable(const nlohmann::json& json,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading CheckboxTable..." << std::endl;
    for (const auto& item : json) {
        properties.emplace_back(item);
    }

    const auto tableName = std::string("CheckBox");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/checkbox-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Property",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("A named property to be tied to this item.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Value",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The value string associated with this item.",
                url)
        ));
    }
}

bool CheckboxTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating CheckboxTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `CheckBox` "
        "(`Property` CHAR(72) NOT NULL, `Value` CHAR(64) "
        "PRIMARY KEY `Property`)";
    const auto sql_insert = "INSERT INTO `CheckBox` "
        "(`Property`, `Value`) VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, properties);
}
