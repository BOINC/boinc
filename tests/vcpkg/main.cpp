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

#include <iostream>
#include <thread>
#include <chrono>

#include <boinc/boinc_api.h>

int main(int argc, char* argv[]) {
    BOINC_OPTIONS boinc_options;
    boinc_options_defaults(boinc_options);
    if (const int res = boinc_init_options(&boinc_options)) {
        std::cerr << "Error initializing BOINC options: " << res << std::endl;
        return res;
    }

    std::cout << "Running test..." << std::endl;
    boinc_fraction_done(0.0);
    for(int i = 1; i <= 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << i << "/10" << std::endl;
        boinc_fraction_done(i / 10.0);
    }
    boinc_fraction_done(1.0);
    std::cout << "Test completed." << std::endl;
    boinc_finish(0);
    return 0;
}
