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

#include "IconTable.h"

IconTable::IconTable(const nlohmann::json& json,
    const std::filesystem::path& path, const std::string& platform,
    const std::string& configuration,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading IconTable..." << std::endl;

    for (const auto& item : json) {
        values.emplace_back(item, path, platform, configuration);
    }

    const auto tableName = std::string("Icon");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/icon-table";
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
            DescriptionWithUrl("Name of the icon file.", url)
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
            DescriptionWithUrl("The binary icon data in PE (.dll or .exe) or "
                "icon (.ico) format.", url)
        ));
    }
}

bool IconTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating IconTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Icon` (`Name` CHAR(72) NOT NULL, "
        "`Data` OBJECT PRIMARY KEY `Name`)";
    const auto sql_insert = "INSERT INTO `Icon` (`Name`, `Data`) "
        "VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}
