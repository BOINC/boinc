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

#include "msi_helper.h"

class test_boinccas_Base {
    using boinccasFn = UINT(WINAPI*)(MSIHANDLE);
public:
    test_boinccas_Base() = delete;
protected:
    ~test_boinccas_Base() = default;
    test_boinccas_Base(std::string_view functionName);

    auto openMsi() {
        return MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
    }

    auto executeAction() {
        return hFunc(hMsi);
    }

    void insertMsiProperties(
        const std::vector<std::pair<std::string, std::string>>& properties) {
        msiHelper.insertProperties(properties);
    }

    auto getMsiProperty(const std::string& propertyName) {
        return msiHelper.getProperty(hMsi, propertyName);
    }

    void setMsiProperty(const std::string& propertyName,
        const std::string& propertyValue) {
        msiHelper.setProperty(hMsi, propertyName, propertyValue);
    }

    auto getMsiHandle() {
        return hMsi;
    }
private:
    wil::unique_hmodule hDll = nullptr;
    boinccasFn hFunc = nullptr;
    MsiHelper msiHelper;
    PMSIHANDLE hMsi;
};

class test_boinccas_TestBase :
    public test_boinccas_Base, public ::testing::Test {
public:
    test_boinccas_TestBase() = delete;
protected:
    ~test_boinccas_TestBase() = default;
    test_boinccas_TestBase(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};

template <typename T>
class test_boinccas_TestBase_WithParam :
    public test_boinccas_Base, public ::testing::TestWithParam<T> {
public:
    test_boinccas_TestBase_WithParam() = delete;
protected:
    ~test_boinccas_TestBase_WithParam() = default;
    test_boinccas_TestBase_WithParam(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};
