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

#include "FontTable.h"

FontTable::FontTable(const std::vector<Directory>& directories,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& file : component.getFiles()) {
                if (file.isFontFile()) {
                    fonts.emplace_back(file.getFileId(), "");
                }
            }
        }
    }

    const auto tableName = std::string("Font");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/font-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "File_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "File",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("External key into the File table entry for "
                "the font file.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "FontTitle",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("Font name.", url)
        ));
    }
}

bool FontTable::generate(MSIHANDLE database) {
    std::cout << "Generating FontTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Font` (`File_` CHAR(72) NOT NULL, "
        "`FontTitle` CHAR(128) PRIMARY KEY `File_`)";
    const auto sql_insert = "INSERT INTO `Font` (`File_`, `FontTitle`) "
        "VALUES (?, ?)";

    return Generator::generate(database, sql_create, sql_insert, fonts);
}
