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

#include "AdminExecuteSequenceTable.h"

AdminExecuteSequenceTable::AdminExecuteSequenceTable(
    const nlohmann::json& json,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading AdminExecuteSequenceTable..." << std::endl;

    for (const auto& value : json) {
        actions.emplace_back(value);
    }

    const auto tableName = std::string("AdminExecuteSequence");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/adminexecutesequence-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Action",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Name of the action to execute.", url)
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
            DescriptionWithUrl("Logical expression.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Sequence",
            true,
            -4,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A positive value indicates the "
            "sequence position of the action.", url)
        ));
    }
}

bool AdminExecuteSequenceTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating AdminExecuteSequenceTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `AdminExecuteSequence` "
        "(`Action` CHAR(72) NOT NULL, `Condition` CHAR(255), "
        "`Sequence` SHORT PRIMARY KEY `Action`)";
    const auto sql_insert = "INSERT INTO `AdminExecuteSequence` "
        "(`Action`, `Condition`, `Sequence`) VALUES (?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, actions);
}
