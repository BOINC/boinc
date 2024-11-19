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
    const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            auto [result, record] = component.getCreateFolder();
            if (result) {
                createFolders.emplace_back(record);
            }
        }
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
