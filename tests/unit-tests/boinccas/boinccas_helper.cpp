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

#include "boinccas_helper.h"

test_boinccas_Base::test_boinccas_Base(std::string_view functionName) {
    wil::unique_hmodule dll(LoadLibrary("boinccas.dll"));
    if (!dll) {
        throw std::runtime_error("Failed to load boinccas.dll");
    }
    auto func = reinterpret_cast<boinccasFn>(GetProcAddress(dll.get(),
        functionName.data()));
    if (!func) {
        throw std::runtime_error("Failed to load function: " +
            std::string(functionName));
    }
    hDll = std::move(dll);
    hFunc = func;
}
