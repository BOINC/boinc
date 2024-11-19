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

#include "MsiFileHashTable.h"

MsiFileHashTable::MsiFileHashTable(const std::vector<File>& files) {
    for (const auto& file : files) {
        MSIFILEHASHINFO hashInfo;
        hashInfo.dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
        if (MsiGetFileHash(file.getFilepath().string().c_str(), 0, &hashInfo)
            != ERROR_SUCCESS) {
            std::cerr << "Failed to get file hash for " << file.getFilepath()
                << std::endl;
            continue;
        }
        msiFileHashes.emplace_back(file.getFileId(), hashInfo.dwData[0],
            hashInfo.dwData[1], hashInfo.dwData[2], hashInfo.dwData[3]);
    }
}

bool MsiFileHashTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating MsiFileHashTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `MsiFileHash` "
        "(`File_` CHAR(72) NOT NULL, `Options` SHORT NOT NULL, "
        "`HashPart1` LONG NOT NULL, `HashPart2` LONG NOT NULL, "
        "`HashPart3` LONG NOT NULL, `HashPart4` LONG NOT NULL "
        "PRIMARY KEY `File_`)";
    const auto sql_insert = "INSERT INTO `MsiFileHash` (`File_`, `Options`, "
        "`HashPart1`, `HashPart2`, `HashPart3`, `HashPart4`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert,
        msiFileHashes);
}
