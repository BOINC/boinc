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

#include <filesystem>

#include "Installer.h"

int main(int, char**) {
    const std::string configuration =
#ifdef _DEBUG
        "Debug";
#else
        "Release";
#endif
    const std::string platform =
#ifdef _ARM64_
        "ARM64";
#else
        "x64";
#endif
    const auto output_path =
        std::filesystem::current_path() / "Build" / platform / configuration;
    Installer installer(output_path);
    if (!installer.load(
        std::filesystem::current_path() / "../installer/boinc.json")) {
        return 1;
    }
    return installer.create_msi(output_path / "boinc.msi") ? 0 : 1;
}
