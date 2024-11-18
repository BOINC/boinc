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

#include <sstream>
#include <iomanip>

#include <windows.h>
#include <objbase.h>

#include "GuidHelper.h"

std::string GuidHelper::generate_guid() {
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK) {
        throw std::runtime_error("Failed to create GUID");
    }

    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0')
        << '{'
        << std::setw(8) << guid.Data1 << '-'
        << std::setw(4) << guid.Data2 << '-'
        << std::setw(4) << guid.Data3 << '-'
        << std::setw(2) << static_cast<int>(guid.Data4[0])
        << std::setw(2) << static_cast<int>(guid.Data4[1]) << '-'
        << std::setw(2) << static_cast<int>(guid.Data4[2])
        << std::setw(2) << static_cast<int>(guid.Data4[3])
        << std::setw(2) << static_cast<int>(guid.Data4[4])
        << std::setw(2) << static_cast<int>(guid.Data4[5])
        << std::setw(2) << static_cast<int>(guid.Data4[6])
        << std::setw(2) << static_cast<int>(guid.Data4[7])
        << '}';

    return oss.str();
}
