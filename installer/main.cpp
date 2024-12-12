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

int main(int argc, char** argv) {
    std::string configuration =
#ifdef _DEBUG
        "Debug";
#else
        "Release";
#endif
    std::string platform =
#ifdef _ARM64_
        "ARM64";
#else
        "x64";
#endif
    std::string project_path{};

    for (int i = 1; i < argc; i++) {
        auto arg = std::string(argv[i]);
        if (arg == "-c") {
            if (++i >= argc) {
                std::cerr << "Missing configuration" << std::endl;
                return 1;
            }
            arg = argv[i];
            if (arg != "Debug" && arg != "Release") {
                std::cerr << "Invalid configuration: " << arg << std::endl;
                std::cerr << "Valid configurations are Debug and Release"
                    << std::endl;
                return 1;
            }
            configuration = arg;
        }
        else if (arg == "-p") {
            if (++i >= argc) {
                std::cerr << "Missing platform" << std::endl;
                return 1;
            }
            arg = argv[i];
            if (arg != "x64" && arg != "ARM64") {
                std::cerr << "Invalid platform " << arg << std::endl;
                std::cerr << "Valid platforms are x64 and ARM64" << std::endl;
                return 1;
            }
            platform = arg;
        }
        else if (project_path.empty()) {
            project_path = arg;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    const auto output_path =
        std::filesystem::current_path() / "Build" / platform / configuration;
    Installer installer(output_path, platform, configuration);
    if (!installer.load(
        std::filesystem::current_path() / (project_path.empty() ?
            "../installer/boinc.json" : project_path))) {
        return 1;
    }
    return installer.create_msi(output_path / "boinc.msi") ? 0 : 1;
}
