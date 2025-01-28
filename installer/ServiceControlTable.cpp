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

#include "ServiceControlTable.h"

ServiceControlTable::ServiceControlTable(
    const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& serviceControl : component.getServiceControls()) {
                values.emplace_back(serviceControl);
            }
        }
    }

    const auto tableName = std::string("ServiceControl");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/servicecontrol-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "ServiceControl",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This is the primary key of this table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Name",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column is the string naming the service.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Event",
            false,
            0,
            187,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column contains the operations to be "
                "performed on the named service.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Arguments",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("A list of arguments for starting services.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Wait",
            true,
            0,
            1,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("Leaving this field null or entering a value "
                "of 1 causes the installer to wait a maximum of 30 seconds "
                "for the service to complete before proceeding.", url)
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
            DescriptionWithUrl("External key to column one of the Component "
                "Table.", url)
        ));
    }
}

bool ServiceControlTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ServiceControlTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ServiceControl` "
        "(`ServiceControl` CHAR(72) NOT NULL, "
        "`Name` CHAR(255) NOT NULL LOCALIZABLE, `Event` SHORT NOT NULL, "
        "`Arguments` CHAR(255) LOCALIZABLE, `Wait` SHORT, "
        "`Component_` CHAR(72) NOT NULL PRIMARY KEY `ServiceControl`)";
    const auto sql_insert = "INSERT INTO `ServiceControl` (`ServiceControl`, "
        "`Name`, `Event`, `Arguments`, `Wait`, `Component_`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
