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

#include "ServiceInstallTable.h"

ServiceInstallTable::ServiceInstallTable(
    const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& serviceInstall : component.getServiceInstalls()) {
                values.push_back(serviceInstall);
            }
        }
    }

    const auto tableName = std::string("ServiceInstall");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/serviceinstall-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "ServiceInstall",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This is the primary key for the table.", url)
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
            DescriptionWithUrl("This column is the string that gives the "
                "service name to install.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DisplayName",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column is the localizable string that "
                "user interface programs use to identify the service.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ServiceType",
            false,
            -2147483647,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column is a set of bit flags that "
                "specify the type of service.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "StartType",
            false,
            0,
            4,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column is a set of bit flags that "
                "specify when to start the service.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ErrorControl",
            false,
            -2147483647,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column specifies the action taken by the "
                "startup program if the service fails to start during "
                "startup.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "LoadOrderGroup",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column contains the string that names "
                "the load ordering group of which this service is a member.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Dependencies",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column is a list of names of services or "
                "load ordering groups that the system must start before this "
                "service.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "StartName",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The service is logged on as the name given by "
                "the string in this column.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Password",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This string is the password to the account "
                "name specified in the StartName column.", url)
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
            DescriptionWithUrl("This column contains any command line "
                "arguments or properties required to run the service.", url)
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
        validationTable->add(Validation(
            tableName,
            "Description",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This column contains a localizable "
                "description for the service being configured.", url)
        ));
    }
}

bool ServiceInstallTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ServiceInstallTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ServiceInstall` "
        "(`ServiceInstall` CHAR(72) NOT NULL, `Name` CHAR(255) NOT NULL, "
        "`DisplayName` CHAR(255) LOCALIZABLE, `ServiceType` LONG NOT NULL, "
        "`StartType` LONG NOT NULL, `ErrorControl` LONG NOT NULL, "
        "`LoadOrderGroup` CHAR(255), `Dependencies` CHAR(255), "
        "`StartName` CHAR(255), `Password` CHAR(255), `Arguments` CHAR(255), "
        "`Component_` CHAR(72) NOT NULL, `Description` CHAR(255) LOCALIZABLE "
        "PRIMARY KEY `ServiceInstall`)";
    const auto sql_insert = "INSERT INTO `ServiceInstall` (`ServiceInstall`, "
        "`Name`, `DisplayName`, `ServiceType`, `StartType`, `ErrorControl`, "
        "`LoadOrderGroup`, `Dependencies`, `StartName`, `Password`, "
        "`Arguments`, `Component_`, `Description`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
