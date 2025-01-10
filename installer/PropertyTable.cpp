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

#include "PropertyTable.h"
#include "GuidHelper.h"

PropertyTable::PropertyTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading PropertyTable..." << std::endl;
    for (const auto& item : json) {
        properties.emplace_back(item, installerStrings);
    }
    std::cout << "Generating new ProductCode..." << std::endl;
    properties.emplace_back("ProductCode", GuidHelper::generate_guid());

    const auto tableName = std::string("Property");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/property-table";
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
            DescriptionWithUrl("The name of the property.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Value",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("A localizable string value for the property.",
                url)
        ));
    }
}

bool PropertyTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating PropertyTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Property` "
        "(`Property` CHAR(72) NOT NULL, `Value` LONGCHAR NOT NULL LOCALIZABLE "
        "PRIMARY KEY `Property`)";
    const auto sql_insert = "INSERT INTO `Property` (`Property`, `Value`) "
        "VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, properties);
}
