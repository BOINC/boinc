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

#include "RemoveFileTable.h"

RemoveFileTable::RemoveFileTable(const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& file : component.getRemoveFiles())
                values.emplace_back(file);
        }
    }

    const auto tableName = std::string("RemoveFile");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/removefile-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "FileKey",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Primary key used to identify this particular "
                "table entry.", url)
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
            DescriptionWithUrl("External key the first column of the "
                "Component table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "FileName",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryWildCardFilename,
            "",
            DescriptionWithUrl("This column contains the localizable name of "
                "the file to be removed.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DirProperty",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Name of a property whose value is assumed to "
                "resolve to the full path to the folder of the file to be "
                "removed.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "InstallMode",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            "",
            "1;2;3",
            DescriptionWithUrl("Specifies the mode in which the file is installed.", url)
        ));
    }
}

bool RemoveFileTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating RemoveFileTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `RemoveFile` "
        "(`FileKey` CHAR(72) NOT NULL, `Component_` CHAR(72) NOT NULL, "
        "`FileName` CHAR(255) LOCALIZABLE, `DirProperty` CHAR(72) NOT NULL, "
        "`InstallMode` SHORT NOT NULL PRIMARY KEY `FileKey`)";
    const auto sql_insert = "INSERT INTO `RemoveFile` "
        "(`FileKey`, `Component_`, `FileName`, `DirProperty`, `InstallMode`) "
        "VALUES (?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
