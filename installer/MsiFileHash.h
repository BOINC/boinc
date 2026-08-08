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

#pragma once

#include "Record.h"

class MsiFileHash : public Record {
public:
    explicit MsiFileHash(const std::string& file, int hashPart1, int hashPart2,
        int hashPart3, int hashPart4);
    ~MsiFileHash() = default;
    MSIHANDLE getRecord() const override;
private:
    std::string file{};
    int options = 0;
    int hashPart1 = MSI_NULL_INTEGER;
    int hashPart2 = MSI_NULL_INTEGER;
    int hashPart3 = MSI_NULL_INTEGER;
    int hashPart4 = MSI_NULL_INTEGER;
};
