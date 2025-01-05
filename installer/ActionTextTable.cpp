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

#include <iostream>

#include "ActionTextTable.h"

ActionTextTable::ActionTextTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading ActionTextTable..." << std::endl;
    for (const auto& item : json) {
        values.emplace_back(item, installerStrings);
    }
    const auto tableName = std::string("ActionText");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/actiontext-table";
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
            "Description",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("Localized description that is displayed in "
                "the progress dialog box, or written to the log when the "
                "action is executing.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Template",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryTemplate,
            "",
            DescriptionWithUrl("A localized format template that is used to "
                "format action data records to display during action "
            "execution.", url)
        ));
    }
}

bool ActionTextTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ActionTextTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ActionText` "
        "(`Action` CHAR(72) NOT NULL, `Description` LONGCHAR LOCALIZABLE, "
        "`Template` LONGCHAR LOCALIZABLE PRIMARY KEY `Action`)";
    const auto sql_insert = "INSERT INTO `ActionText` "
        "(`Action`, `Description`, `Template`) VALUES (?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
