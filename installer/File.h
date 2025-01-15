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

#include <nlohmann/json.hpp>

#include "Record.h"

class File : public Record {
public:
    explicit File(const nlohmann::json& json, const std::string& component,
        const std::string& directory);
    ~File() = default;
    MSIHANDLE getRecord() const override;
    std::filesystem::path getFilepath() const;
    std::string getFileId() const;
    bool isFontFile() const noexcept;
    void setFilesize(int size) noexcept;
    void setVersion(const std::string& v);
    void setLanguage(const std::string& l);
    void setAttributes(int a) noexcept;
    void setSequence(int s) noexcept;
    void setFilepath(const std::filesystem::path& p);
    void setShortFileName(const std::string& n);
    void setLongFileName(const std::string& n);
    std::string getShortFileName() const;
    std::string getLongFileName() const;
    bool isVersioned() const noexcept;
    std::string getDirectory() const;
private:
    std::string component{};
    std::string filename_short{};
    std::string filename_long{};
    int filesize = MSI_NULL_INTEGER;
    std::string version{};
    std::string language{};
    int attributes = MSI_NULL_INTEGER;
    int sequence = MSI_NULL_INTEGER;
    std::filesystem::path filepath{};
    bool isFont = false;
    std::string directory{};
};
