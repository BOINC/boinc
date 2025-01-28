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

#include "RemoveFile.h"
#include "JsonHelper.h"
#include "MsiHelper.h"

RemoveFile::RemoveFile(const nlohmann::json& json,
    const std::string& component) : component(component) {
    JsonHelper::get(json, "FileKey", fileKey);
    JsonHelper::get(json, "FileName", fileName);
    JsonHelper::get(json, "DirProperty", dirProperty);
    JsonHelper::get(json, "InstallMode", installMode);
}

MSIHANDLE RemoveFile::getRecord() const {
    return MsiHelper::MsiRecordSet({ fileKey, component, fileName, dirProperty,
        installMode });
}
