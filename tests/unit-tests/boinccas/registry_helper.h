// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

template <typename T, typename = std::enable_if_t<
    std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<BYTE>>>>
    T getRegistryValue(HKEY hRootKey, const std::string& keyName,
        const std::string& valueName) {
    HKEY hKey = nullptr;
    const auto openResult = RegOpenKeyEx(hRootKey, keyName.c_str(), 0,
        KEY_READ, &hKey);
    if (openResult != ERROR_SUCCESS) {
        return {};
    }
    wil::unique_hkey autoKey(hKey);

    DWORD type = 0;
    DWORD size = 0;
    auto queryResult = RegQueryValueEx(hKey, valueName.c_str(), nullptr, &type,
        nullptr, &size);
    if (queryResult != ERROR_SUCCESS) {
        return {};
    }

    T value;
    value.resize(size);
    queryResult = RegQueryValueEx(hKey, valueName.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(value.data()), &size);

    if (queryResult != ERROR_SUCCESS) {
        return {};
    }

    if constexpr (std::is_same_v <T, std::string>) {
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            const auto nulPos = value.find('\0');
            if (nulPos != std::string::npos) {
                value.resize(nulPos);
            }
        }
    }

    return value;
}
std::string getRegistryValue(const std::string& valueName);

bool setRegistryValue(HKEY hRootKey, const std::string& keyName,
    const std::string& valueName, const std::string& valueData);
bool setRegistryValue(const std::string& valueName,
    const std::string& valueData);

void cleanRegistryKey(HKEY hRootKey, const std::string& keyName);
void cleanRegistryKey();
