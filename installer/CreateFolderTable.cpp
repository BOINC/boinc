// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

#include "CreateFolderTable.h"

CreateFolderTable::CreateFolderTable(
    const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            auto [result, record] = component.getCreateFolder();
            if (result) {
                createFolders.emplace_back(record);
            }
        }
    }

    const auto tableName = std::string("CreateFolder");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/createfolder-table";
    if (validationTable != nullptr) {
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
            DescriptionWithUrl("External key into the first column of the "
                "Directory table.", url)
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
                "Component table.", url)
        ));
    }
}

bool CreateFolderTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating CreateFolderTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `CreateFolder` "
        "(`Directory_` CHAR(72) NOT NULL, `Component_` CHAR(72) NOT NULL "
        "PRIMARY KEY `Directory_`, `Component_`)";
    const auto sql_insert = "INSERT INTO `CreateFolder` "
        "(`Directory_`, `Component_`) VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert,
        createFolders);
}
