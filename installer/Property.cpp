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

#include "Property.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

Property::Property(const nlohmann::json& json,
    InstallerStrings& installerStrings) {
    JsonHelper::get(json, "Property", property);
    JsonHelper::get(json, "Value", value, installerStrings);
}

Property::Property(const std::string& property, const std::string& value) :
    property(property), value(value) {
}

MSIHANDLE Property::getRecord() const {
    return MsiHelper::MsiRecordSet({ property, value });
}
