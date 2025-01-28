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

#include "ComponentTable.h"

ComponentTable::ComponentTable(const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            components.push_back(component);
        }
    }

    const auto tableName = std::string("Component");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/component-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Component",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Identifies the component record.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ComponentId",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryGuid,
            "",
            DescriptionWithUrl("A string GUID unique to this component, "
                "version, and language.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Directory_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Directory",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("External key of an entry in the Directory "
                "table.", url)
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
            "",
            DescriptionWithUrl("This column contains a bit flag that "
                "specifies options for remote execution.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Condition",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCondition,
            "",
            DescriptionWithUrl("This column contains a conditional statement "
                "that can control whether a component is installed.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "KeyPath",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "File;Registry",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This value points to a file or folder "
                "belonging to the component that the installer uses to detect "
                "the component.", url)
        ));
    }
}

bool ComponentTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ComponentTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Component` "
        "(`Component` CHAR(72) NOT NULL, `ComponentId` CHAR(38), "
        "`Directory_` CHAR(72) NOT NULL, "
        "`Attributes` SHORT NOT NULL, `Condition` CHAR(255), KeyPath CHAR(72) "
        "PRIMARY KEY `Component`)";
    const auto sql_insert = "INSERT INTO `Component` (`Component`, "
        "`ComponentId`, `Directory_`, `Attributes`, `Condition`, `KeyPath`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, components);
}
