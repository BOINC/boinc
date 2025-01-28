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

#include "ShortcutTable.h"

ShortcutTable::ShortcutTable(const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& shortcut : component.getShortcuts()) {
                values.push_back(shortcut);
            }
        }
    }

    const auto tableName = std::string("Shortcut");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/shortcut-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Shortcut",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The key value for this table.", url)
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
            DescriptionWithUrl("The external key into the first column of the "
                "Directory table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Name",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFilename,
            "",
            DescriptionWithUrl("The localizable name of the shortcut to be "
                "created.", url)
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
            DescriptionWithUrl("The external key into the first column of the "
                "Component table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Target",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryShortcut,
            "",
            DescriptionWithUrl("The shortcut target.", url)
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
            DescriptionWithUrl("The command-line arguments for the shortcut.",
                url)
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
            DescriptionWithUrl("The localizable description of the shortcut.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Hotkey",
            true,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The hotkey for the shortcut.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Icon_",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Icon",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The external key to column one of the Icon "
                "table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "IconIndex",
            true,
            -32767,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The icon index for the shortcut.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "ShowCmd",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            "",
            "1;3;7",
            DescriptionWithUrl("The Show command for the application window.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "WkDir",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The name of the property that has the path of "
                "the working directory for the shortcut.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DisplayResourceDll",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This field contains a Formatted string value "
                "for the full path to the language-neutral portable "
                "executable (LN file) that contains the resource "
                "configuration (RC Config) data.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DisplayResourceId",
            true,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The display name index for the shortcut.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DescriptionResourceDll",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("This field contains a Formatted string value "
                "for the full path to the language-neutral portable "
                "executable (LN file) that contains the resource "
                "configuration (RC Config) data.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DescriptionResourceId",
            true,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The description name index for the shortcut.",
                url)
        ));
    }
}

bool ShortcutTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ShortcutTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Shortcut` "
        "(`Shortcut` CHAR(72) NOT NULL, `Directory_` CHAR(72) NOT NULL, "
        "`Name` CHAR(128) NOT NULL LOCALIZABLE, "
        "`Component_` CHAR(72) NOT NULL, `Target` CHAR(255) NOT NULL, "
        "`Arguments` CHAR(255), `Description` CHAR(255) LOCALIZABLE, "
        "`Hotkey` SHORT, `Icon_` CHAR(72), `IconIndex` SHORT, "
        "`ShowCmd` SHORT, `WkDir` CHAR(72) NOT NULL, "
        "`DisplayResourceDll` CHAR(255), `DisplayResourceId` SHORT, "
        "`DescriptionResourceDll` CHAR(255), `DescriptionResourceId` SHORT "
        "PRIMARY KEY `Shortcut`)";

    const auto sql_insert = "INSERT INTO `Shortcut` (`Shortcut`, "
        "`Directory_`, `Name`, `Component_`, `Target`, `Arguments`, "
        "`Description`, `Hotkey`, `Icon_`, `IconIndex`, `ShowCmd`, `WkDir`, "
        "`DisplayResourceDll`, `DisplayResourceId`, `DescriptionResourceDll`, "
        "`DescriptionResourceId`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
