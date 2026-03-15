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

#include "registry_helper.h"

constexpr auto BOINC_SETUP_REGISTRY_KEY =
"SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup";

std::string getRegistryValue(const std::string& valueName) {
    return getRegistryValue<std::string>(HKEY_LOCAL_MACHINE,
        BOINC_SETUP_REGISTRY_KEY, valueName);
}

bool setRegistryValue(const std::string& valueName,
    const std::string& valueData) {
    return setRegistryValue(HKEY_LOCAL_MACHINE,
        BOINC_SETUP_REGISTRY_KEY, valueName, valueData);
}

bool setRegistryValue(HKEY hRootKey, const std::string& keyName,
    const std::string& valueName, const std::string& valueData) {
    HKEY hKey = nullptr;
    const auto createResult = RegCreateKeyEx(hRootKey, keyName.c_str(), 0,
        nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    if (createResult != ERROR_SUCCESS) {
        return false;
    }
    wil::unique_hkey autoKey(hKey);

    const auto setResult = RegSetValueEx(hKey, valueName.c_str(), 0,
        REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()),
        static_cast<DWORD>(valueData.size() + 1));

    return setResult == ERROR_SUCCESS;
}

void cleanRegistryKey() {
    cleanRegistryKey(HKEY_LOCAL_MACHINE, BOINC_SETUP_REGISTRY_KEY);
}

void cleanRegistryKey(HKEY hRootKey, const std::string& keyName) {
    HKEY hKey = nullptr;
    const auto openResult = RegOpenKeyEx(hRootKey, keyName.c_str(), 0, KEY_WRITE,
        &hKey);
    if (openResult != ERROR_SUCCESS) {
        return;
    }
    RegCloseKey(hKey);
    RegDeleteKey(hRootKey, keyName.c_str());
}
