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

// we record some properties of hosts (RAM size, CPU info, etc.)
// directly in fields of the host table.
//
// For other stuff (GPUs, Docker info) we use text.
// Old way:
// use strings like [BOINC|8.2.4][CUDA|NVIDIA GeForce RTX 3060 Ti|1|8191MB|57260|300][vbox|7.1.6|1|1]
// and store them in host.serialnum.
// This is bad for several reasons (e.g. we can overflow the 254 chars)
//
// New way:
// store them in JSON in a new field host.misc

// This file has code for both ways.
// At some point we can delete the old way.

#include <string.h>
#include <string>
#include <fmt/format.h>

using std::string;
using fmt::format;

#include "sched_types.h"

#include "sched_host.h"

struct DOCKER_INFO {
    // the following if non-Win
    string docker_version;
    int docker_type;

    // the following if Win
    string wsl_version;
    bool use_wslc;  // have WSL 3+
    // the following if Win and WSL < 3
    string wsl_distro;
    int bbr_version;

    void get_docker_info() {
        docker_version = "";
        docker_type = 0;
        wsl_version = "";
        use_wslc = false;
        wsl_distro = "";
        bbr_version = 0;
        if (strstr(g_request->host.os_name, "Windows")) {
            wsl_version = g_request->host.wsl_version;
            use_wslc = g_request->host.use_wslc;
            if (!g_request->host.use_wslc) {
                WSL_DISTRO *wdp = g_request->host.wsl_distros.find_docker();
                if (wdp) {
                    docker_version = wdp->docker_version;
                    docker_type = wdp->docker_type;
                    wsl_distro = wdp->distro_name;
                    bbr_version = wdp->boinc_buda_runner_version;
                }
            }
        } else {
            docker_version = g_request->host.docker_version;
            docker_type = g_request->host.docker_type;
        }
    }
};

// return misc host info as JSON
//
string host_info_json() {
    string out = format(
        "{{\n"\
"    \"client_version\": \"{}.{}.{}\"",
        g_request->core_client_major_version,
        g_request->core_client_minor_version,
        g_request->core_client_release
    );

    string gpus = g_request->coprocs.summary_string_json();
    if (!gpus.empty()) {
        out += ",\n"\
"    \"gpus\": [\n";
        out += gpus;
        out += "\n    ]";
    }

    DOCKER_INFO di;
    di.get_docker_info();

    if (!di.wsl_version.empty()) {
        out += format(
            ",\n"\
"    \"wsl\": {{\n"\
"        \"version\": \"{}\",\n"\
"        \"use_wslc\": {}\n"\
"    }}",
            di.wsl_version,
            di.use_wslc?"true":"false"
        );
    }
    if (!di.docker_version.empty()) {
        out += format(
            ",\n"\
"    \"docker\": {{\n"\
"        \"version\": \"{}\",\n"\
"        \"type\": {}",
            di.docker_version,
            di.docker_type
        );
        if (!di.wsl_distro.empty()) {
            out += format(
                ",\n        \"wsl_distro\": \"{}\"",
                di.wsl_distro
            );
        }
        if (di.bbr_version) {
            out += format(
                ",\n        \"boinc_buda_runner_version\": {}",
                di.bbr_version
            );
        }
        out += "\n    }}";
    }

    if (strlen(g_request->host.virtualbox_version)) {
        out += format( 
",\n"\
"    \"vbox\": {{\n"\
"        \"version\": \"{}\",\n"\
"        \"hw_accel\": {},\n"\
"        \"hw_accel_enabled\": {}\n"\
"    }}",
            g_request->host.virtualbox_version,
            (strstr(g_request->host.p_features, "vmx") || strstr(g_request->host.p_features, "svm"))?"true":"false",
            g_request->host.p_vm_extensions_disabled?"false":"true"
        );
    }

    if (g_request->dont_use_docker || g_request->dont_use_wsl) {
        out += format(
",\n"\
"    \"config\": {{\n"\
"        \"dont_use_docker\": {},\n"\
"        \"dont_use_wsl\": {}\n"\
"    }}",
            g_request->dont_use_docker?"true":"false",
            g_request->dont_use_wsl?"true":"false"
        );
    }

    out += "\n}";
    return out;
}
