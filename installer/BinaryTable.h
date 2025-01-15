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

#pragma once

#include <filesystem>

#include "Generator.h"
#include "Binary.h"
#include "ValidationTable.h"

class BinaryTable : public Generator<Binary> {
public:
    explicit BinaryTable(const nlohmann::json& json,
        const std::filesystem::path& path, const std::string& platform,
        const std::string& configuration,
        std::shared_ptr<ValidationTable> validationTable);
    ~BinaryTable() = default;
    bool generate(MSIHANDLE hDatabase) override;
private:
    std::vector<Binary> binaries{};
};
