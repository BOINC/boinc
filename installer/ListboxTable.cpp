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

#include "ListboxTable.h"

ListboxTable::ListboxTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading ListboxTable..." << std::endl;
    for (const auto& item : json) {
        values.emplace_back(item, installerStrings);
    }

    const auto tableName = std::string("ListBox");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/listbox-table";
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
            "Order",
            false,
            1,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A positive integer used to determine the "
                "ordering of the items that appear in a single list box.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Value",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The value string associated with this item.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Text",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The localizable, visible text to be assigned "
                "to the item.", url)
        ));
    }
}

bool ListboxTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ListboxTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ListBox` "
        "(`Property` CHAR(72) NOT NULL, `Order` SHORT NOT NULL, "
        "`Value` CHAR(64) NOT NULL, `Text` CHAR(64) "
        "PRIMARY KEY `Property`, `Order`)";
    const auto sql_insert = "INSERT INTO `ListBox` "
        "(`Property`, `Order`, `Value`, `Text`) VALUES (?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
