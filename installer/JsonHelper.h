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

#include <type_traits>

#include <nlohmann/json.hpp>

#include "InstallerStrings.h"

class JsonHelper {
public:
    JsonHelper() = delete;
    ~JsonHelper() = delete;
    JsonHelper(const JsonHelper&) = delete;
    JsonHelper& operator=(const JsonHelper&) = delete;
    JsonHelper(JsonHelper&&) = delete;
    JsonHelper& operator=(JsonHelper&&) = delete;

    static bool exists(const nlohmann::json& json, const std::string& key) {
        return json.contains(key) && !json[key].is_null();
    }

    template <class T>
    static void get(const nlohmann::json& json, const std::string& key,
        T& value) {
        if (exists(json, key)) {
            if constexpr (std::is_same_v<T, std::filesystem::path>) {
                value = json[key].get<std::string>();
            }
            else {
                value = json[key];
            }
        }
    }
    template<class T>
    static T get(const nlohmann::json& json, const std::string& key) {
        if (exists(json, key)) {
            return json[key];
        }
        return {};
    }
    template <class T>
    static void get(const nlohmann::json& json, const std::string& key,
        T& value, InstallerStrings& installerStrings) {
        if (exists(json, key)) {
            value = installerStrings.get(json[key]);
        }
    }
    template <class T>
    static T get(const nlohmann::json& json, const std::string& key,
        InstallerStrings& installerStrings) {
        if (exists(json, key)) {
            return installerStrings.get(json[key]);
        }
        return {};
    }
    template <class T>
    static void handle(const nlohmann::json& json, const std::string& key,
        T handler) {
        if (exists(json, key)) {
            if (json[key].is_array()) {
                for (const auto& item : json[key]) {
                    handler(item);
                }
            }
            else {
                handler(json[key]);
            }
        }
    }
};
