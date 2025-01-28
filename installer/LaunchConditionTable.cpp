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

#include "LaunchConditionTable.h"

LaunchConditionTable::LaunchConditionTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading LaunchConditionTable..." << std::endl;

    for (const auto& launchCondition : json) {
        launchConditions.emplace_back(launchCondition, installerStrings);
    }

    const auto tableName = std::string("LaunchCondition");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/launchcondition-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Condition",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCondition,
            "",
            DescriptionWithUrl("Expression that must evaluate to True for "
                "installation to begin.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Description",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("Localizable text to display when the "
                "condition fails and the installation must be terminated.",
                url)
        ));
    }
}

bool LaunchConditionTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating LaunchConditionTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `LaunchCondition` "
        "(`Condition` CHAR(255) NOT NULL, "
        "`Description` CHAR(255) NOT NULL LOCALIZABLE "
        "PRIMARY KEY `Condition`)";
    const auto sql_insert = "INSERT INTO `LaunchCondition` "
        "(`Condition`, `Description`) VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert,
        launchConditions);
}
