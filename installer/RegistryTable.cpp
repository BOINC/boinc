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

#include "RegistryTable.h"

RegistryTable::RegistryTable(const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& registry : component.getRegistries()) {
                registries.push_back(registry);
            }
        }
    }

    const auto tableName = std::string("Registry");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/registry-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Registry",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Primary key used to identify a registry "
                "record.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Root",
            false,
            -1,
            3,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("he predefined root key for the registry "
                "value.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Key",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryRegPath,
            "",
            DescriptionWithUrl("The localizable key for the registry value.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Name",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column contains the registry value name "
                "(localizable).", url)
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
            DescriptionWithUrl("This column is the localizable registry "
                "value.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Component_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Component",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("External key into the first column of the "
                "Component table referencing the component that controls the "
                "installation of the registry value.", url)
        ));
    }
}

bool RegistryTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating RegistryTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Registry` "
        "(`Registry` CHAR(72) NOT NULL, `Root` SHORT NOT NULL, "
        "`Key` CHAR(255) NOT NULL LOCALIZABLE, `Name` CHAR(255) LOCALIZABLE, "
        "`Value` LONGCHAR LOCALIZABLE, `Component_` CHAR(72) NOT NULL "
        "PRIMARY KEY `Registry`)";
    const auto sql_insert = "INSERT INTO `Registry` (`Registry`, `Root`, "
        "`Key`, `Name`, `Value`, `Component_`) VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, registries);
}
