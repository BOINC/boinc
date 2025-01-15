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

#include <filesystem>

#include "File.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

File::File(const nlohmann::json& json, const std::string& component,
    const std::string& directory) :
    component(component), directory(directory) {
    JsonHelper::get(json, "FilePath", filepath);
    JsonHelper::get(json, "IsFont", isFont);
}

MSIHANDLE File::getRecord() const {
    return MsiHelper::MsiRecordSet({ getFileId(), component,
        filename_short + "|" + filename_long, filesize, version, language,
        attributes, sequence });
}

std::filesystem::path File::getFilepath() const {
    return filepath;
}

std::string File::getFileId() const {
    auto result = component + "_" + filepath.filename().string();
    std::transform(result.begin(), result.end(), result.begin(), [](auto c) {
        if (c == '-') {
            return '_';
        }
        return c;
        });
    return result;
}

bool File::isFontFile() const noexcept {
    return isFont;
}

void File::setFilesize(int size) noexcept {
    filesize = size;
}

void File::setVersion(const std::string& v) {
    version = v;
}

void File::setLanguage(const std::string& l) {
    language = l;
}

void File::setAttributes(int a) noexcept {
    attributes = a;
}

void File::setSequence(int s) noexcept {
    sequence = s;
}

void File::setFilepath(const std::filesystem::path& p) {
    filepath = p;
}

void File::setShortFileName(const std::string& n) {
    filename_short = n;
}

void File::setLongFileName(const std::string& n) {
    filename_long = n;
}

std::string File::getShortFileName() const {
    return filename_short;
}

std::string File::getLongFileName() const {
    return filename_long;
}

bool File::isVersioned() const noexcept {
    return !version.empty();
}

std::string File::getDirectory() const {
    return directory;
}
