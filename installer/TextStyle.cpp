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

#include "TextStyle.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

TextStyle::TextStyle(const nlohmann::json& json) {
    JsonHelper::get(json, "TextStyle", textstyle);
    JsonHelper::get(json, "FaceName", facename);
    JsonHelper::get(json, "Size", size);
    JsonHelper::get(json, "Color", color);
    JsonHelper::get(json, "StyleBits", stylebits);
}

MSIHANDLE TextStyle::getRecord() const {
    return MsiHelper::MsiRecordSet({ textstyle, facename, size, color,
        stylebits });
}
