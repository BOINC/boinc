// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// Detection of Intel GPUs
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#include "coproc.h"
#include "util.h"

#include "client_msgs.h"
#include "gpu_detect.h"

#include <openvino/openvino.hpp>
#include <iostream>
#include <vector>
#include <string>

int check_openvino() {
    // Step 1. Initialize OpenVINO Runtime Core
    ov::Core core;

    // Step 2. Get available devices
    std::vector<std::string> available_devices = core.get_available_devices();

    std::cout << "Available devices:" << std::endl;

    // Step 3. Query device properties
    for (const std::string& device : available_devices) {
        std::cout << "Device: " << device << std::endl;

        // Get the full device name
        auto full_device_name = core.get_property(device, ov::device::full_name);
        std::cout << "\tFull Name: " << full_device_name << std::endl;

        // Get the supported properties for the device
        std::vector<std::string> supported_properties = core.get_property(device, ov::supported_properties);
        std::cout << "\tSupported Properties:" << std::endl;
        for (const std::string& prop : supported_properties) {
            // Avoid querying 'SUPPORTED_PROPERTIES' again to prevent recursion
            if (prop != ov::supported_properties) {
                try {
                    // Query the value of each property
                    auto prop_value = core.get_property(device, prop);
                    // Depending on the property type, you may need different ways to print it.
                    // This is a basic example and might need type handling for complex properties.
                    // std::cout << "\t\t" << prop << ": " << prop_value << std::endl; 
                }
                catch (const ov::Exception& ex) {
                    std::cerr << "\t\t" << prop << ": UNSUPPORTED VALUE TYPE" << std::endl;
                }
            }
        }
    }

    return 0;
}