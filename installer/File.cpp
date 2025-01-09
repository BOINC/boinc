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

File::File(const nlohmann::json& json, const std::string& component) :
    component(component) {
    JsonHelper::get(json, "File", file);
    JsonHelper::get(json, "FilePath", filepath);
    //JsonHelper::handle(json, "FileName", [&](const auto& value) {
    //    //// we don't really support 8.3 file names, but we wan't to be compliant
    //    //// with the MSI standard
    //    //const auto longNameWithExt = value.get<std::string>();
    //    //const auto path = std::filesystem::path(longNameWithExt);
    //    //const auto longName = path.stem().string();
    //    //const auto longExt = path.extension().string();
    //    //auto shortName = longName;
    //    //auto shortExt = longExt;
    //    //if (longName.size() > 8) {
    //    //    shortName = longName.substr(0, 8);
    //    //}
    //    //if (longExt.size() > 4) {
    //    //    shortExt = longExt.substr(0, 4);
    //    //}
    //    //if (shortName != longName || shortExt != longExt) {
    //    //    filename = shortName + shortExt + "|" + longNameWithExt;
    //    //}
    //    //else {
    //    //    filename = longNameWithExt;
    //    //}
    //    char shortName[MAX_PATH];
    //    if (GetShortPathName(filepath.string().c_str(), shortName, MAX_PATH) != 0) {
    //        filename = std::string(shortName) + "|" + value.get<std::string>();
    //    }
    //    else {
    //        filename = value.get<std::string>();
    //    }
    //    });
    JsonHelper::get(json, "IsFont", isFont);
}

MSIHANDLE File::getRecord() const {
    return MsiHelper::MsiRecordSet({ getFileId(), component, filename,
        filesize, version, language, attributes, sequence });
}

std::filesystem::path File::getFilepath() const {
    return filepath;
}

std::string File::getFileId() const {
    auto result = file;
    auto p = result.find("-");
    if (p != std::string::npos) {
        result.replace(p, 1, "_");
    }
    p = result.find("@");
    if (p != std::string::npos) {
        result.replace(p, 1, "_");
    }
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

void File::setFileName(const std::string& n) {
    filename = n;
}

bool File::isVersioned() const noexcept {
    return !version.empty();
}
