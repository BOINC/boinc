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

#include "Generator.h"
#include "Directory.h"
#include "File.h"
#include "ValidationTable.h"

class FileTable : public Generator<File> {
public:
    explicit FileTable(const std::vector<Directory>& directories,
        const std::filesystem::path& root_path,
        const std::filesystem::path& output_path, const std::string& platform,
        const std::string& configuration,
        std::shared_ptr<ValidationTable> validationTable);
    ~FileTable() = default;
    bool generate(MSIHANDLE hDatabase) override;
private:
    int GetFileLanguage(const std::string& filePath);
    std::string GetFileVersion(const std::string& filePath);
    size_t GetFileSize(const std::string& filePath);
    std::filesystem::path GetAbsolutePath(
        const std::filesystem::path& filePath);
    std::tuple<std::string, std::string> GetFileName(
        const std::filesystem::path& filePath, const std::string& directory);
    std::vector<File> files{};
    std::filesystem::path root_path{};
    std::filesystem::path output_path{};
    std::string platform{};
    std::string configuration{};
    std::shared_ptr<ValidationTable> validationTable;
};
