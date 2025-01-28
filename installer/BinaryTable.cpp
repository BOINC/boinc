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

#include "BinaryTable.h"

BinaryTable::BinaryTable(const nlohmann::json& json,
    const std::filesystem::path& path, const std::string& platform,
    const std::string& configuration,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading BinaryTable..." << std::endl;

    for (const auto& element : json) {
        binaries.emplace_back(element, path, platform, configuration);
    }

    const auto tableName = std::string("Binary");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/binary-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Name",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("A unique key that identifies the particular "
                "binary data.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Data",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryBinary,
            "",
            DescriptionWithUrl("The unformatted binary data.", url)
        ));
    }
}

bool BinaryTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating BinaryTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Binary` (`Name` CHAR(72) NOT NULL, "
        "`Data` OBJECT NOT NULL PRIMARY KEY `Name`)";
    const auto sql_insert = "INSERT INTO `Binary` (`Name`, `Data`) "
        "VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, binaries);
}
