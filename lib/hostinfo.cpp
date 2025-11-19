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

// Write and parse HOST_INFO structures.
// Used by client and GUI

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "util.h"
#include "str_replace.h"
#include "filesys.h"

#include "hostinfo.h"

using std::string;

HOST_INFO::HOST_INFO() {
    clear_host_info();
}

// this must NOT clear coprocs
// (initialization logic assumes that)
//
void HOST_INFO::clear_host_info() {
    timezone = 0;
    safe_strcpy(domain_name, "");
    safe_strcpy(ip_addr, "");
    safe_strcpy(host_cpid, "");

    p_ncpus = 0;
    safe_strcpy(p_vendor, "");
    safe_strcpy(p_model, "");
    safe_strcpy(p_features, "");
    p_fpops = 0;
    p_iops = 0;
    p_membw = 0;
    p_calculated = 0;
    p_vm_extensions_disabled = false;

    m_nbytes = 0;
    m_cache = 0;
    m_swap = 0;

    d_total = 0;
    d_free = 0;

    safe_strcpy(os_name, "");
    safe_strcpy(os_version, "");

#ifdef _WIN32
    wsl_distros.clear();
#else
    safe_strcpy(docker_version, "");
    docker_type = NONE;
    safe_strcpy(docker_compose_version, "");
    docker_compose_type = NONE;
#endif

    safe_strcpy(product_name, "");
    safe_strcpy(mac_address, "");

    safe_strcpy(virtualbox_version, "");
    num_opencl_cpu_platforms = 0;
}

int HOST_INFO::parse(XML_PARSER& xp, bool static_items_only) {
    clear_host_info();
    while (!xp.get_tag()) {
        if (xp.match_tag("/host_info")) return 0;
        if (xp.parse_double("p_fpops", p_fpops)) {
            // fix foolishness that could result in negative value here
            //
            if (p_fpops < 0) p_fpops = -p_fpops;
            continue;
        }
        if (xp.parse_double("p_iops", p_iops)) {
            if (p_iops < 0) p_iops = -p_iops;
            continue;
        }
        if (xp.parse_double("p_membw", p_membw)) {
            if (p_membw < 0) p_membw = -p_membw;
            continue;
        }
        if (xp.parse_double("p_calculated", p_calculated)) continue;
        if (xp.parse_bool("p_vm_extensions_disabled", p_vm_extensions_disabled)) continue;
        if (xp.parse_str("host_cpid", host_cpid, sizeof(host_cpid))) continue;
#ifdef ANDROID
        if (xp.parse_str("product_name", product_name, sizeof(product_name))) continue;
        if (xp.parse_str("mac_address", mac_address, sizeof(mac_address))) continue;
        if (xp.parse_str("domain_name", domain_name, sizeof(domain_name))) continue;
#endif

        if (static_items_only) continue;

        // Items after here are determined dynamically at startup,
        // so don't parse them from the state file

        if (xp.parse_int("timezone", timezone)) continue;
        if (xp.parse_str("domain_name", domain_name, sizeof(domain_name))) continue;
        if (xp.parse_str("ip_addr", ip_addr, sizeof(ip_addr))) continue;
        if (xp.parse_int("p_ncpus", p_ncpus)) continue;
        if (xp.parse_str("p_vendor", p_vendor, sizeof(p_vendor))) continue;
        if (xp.parse_str("p_model", p_model, sizeof(p_model))) continue;
        if (xp.parse_str("p_features", p_features, sizeof(p_features))) {
            downcase_string(p_features);
            continue;
        }
        if (xp.parse_double("m_nbytes", m_nbytes)) continue;
        if (xp.parse_double("m_cache", m_cache)) continue;
        if (xp.parse_double("m_swap", m_swap)) continue;
        if (xp.parse_double("d_total", d_total)) continue;
        if (xp.parse_double("d_free", d_free)) continue;
        if (xp.parse_str("os_name", os_name, sizeof(os_name))) continue;
        if (xp.parse_str("os_version", os_version, sizeof(os_version))) continue;
#ifdef _WIN32
        if (xp.match_tag("wsl")) {
            this->wsl_distros.parse(xp);
            continue;
        }
#else
        int i;
        if (xp.parse_str("docker_version", docker_version, sizeof(docker_version))) continue;
        if (xp.parse_int("docker_type", i)) {
            docker_type = (DOCKER_TYPE)i;
            continue;
        }
        if (xp.parse_str("docker_compose_version", docker_compose_version, sizeof(docker_compose_version))) continue;
        if (xp.parse_int("docker_compose_type", i)) {
            docker_compose_type = (DOCKER_TYPE)i;
            continue;
        }
#endif
        if (xp.parse_str("product_name", product_name, sizeof(product_name))) continue;
        if (xp.parse_str("virtualbox_version", virtualbox_version, sizeof(virtualbox_version))) continue;
        if (xp.match_tag("coprocs")) {
            this->coprocs.parse(xp);
        }

        // The same CPU can have a different opencl_cpu_prop
        // for each of multiple OpenCL platforms
        //
        if (xp.match_tag("opencl_cpu_prop")) {
            int retval = opencl_cpu_prop[num_opencl_cpu_platforms].parse(xp);
            if (!retval) num_opencl_cpu_platforms++;
        }
    }
    return ERR_XML_PARSE;
}

// Write the host information to either:
// - client state XML file (net info, coprocs)
// - a GUI RPC reply (net info, coprocs)
// - a scheduler request message
//   (net info unless config says otherwise,
//   no coprocs - we write them separately)
// - account manager request
//   (net info unless config says otherwise, coprocs)
// - app init file (net info, coprocs)
//
int HOST_INFO::write(
    MIOFILE& out, bool include_net_info, bool include_coprocs
) {
    char pv[265], pm[256], pf[P_FEATURES_SIZE], osn[256], osv[256], pn[256];
    out.printf(
        "<host_info>\n"
        "    <timezone>%d</timezone>\n",
        timezone
    );
    if (include_net_info) {
        out.printf(
            "    <domain_name>%s</domain_name>\n"
            "    <ip_addr>%s</ip_addr>\n",
            domain_name,
            ip_addr
        );
    }
    xml_escape(p_vendor, pv, sizeof(pv));
    xml_escape(p_model, pm, sizeof(pm));
    xml_escape(p_features, pf, sizeof(pf));
    xml_escape(os_name, osn, sizeof(osn));
    xml_escape(os_version, osv, sizeof(osv));
    out.printf(
        "    <host_cpid>%s</host_cpid>\n"
        "    <p_ncpus>%d</p_ncpus>\n"
        "    <p_vendor>%s</p_vendor>\n"
        "    <p_model>%s</p_model>\n"
        "    <p_features>%s</p_features>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <p_vm_extensions_disabled>%d</p_vm_extensions_disabled>\n"
        "    <m_nbytes>%f</m_nbytes>\n"
        "    <m_cache>%f</m_cache>\n"
        "    <m_swap>%f</m_swap>\n"
        "    <d_total>%f</d_total>\n"
        "    <d_free>%f</d_free>\n"
        "    <os_name>%s</os_name>\n"
        "    <os_version>%s</os_version>\n"
        "    <n_usable_coprocs>%d</n_usable_coprocs>\n",
        host_cpid,
        p_ncpus,
        pv,
        pm,
        pf,
        p_fpops,
        p_iops,
        p_membw,
        p_calculated,
        p_vm_extensions_disabled?1:0,
        m_nbytes,
        m_cache,
        m_swap,
        d_total,
        d_free,
        osn,
        osv,
        coprocs.ndevs()
    );
#ifdef _WIN32
    wsl_distros.write_xml(out);
#else
    if (strlen(docker_version)) {
        out.printf(
            "    <docker_version>%s</docker_version>\n"
            "    <docker_type>%d</docker_type>\n",
            docker_version,
            docker_type
        );
    }
    if (strlen(docker_compose_version)) {
        out.printf(
            "    <docker_compose_version>%s</docker_compose_version>\n"
            "    <docker_compose_type>%d</docker_compose_type>\n",
            docker_compose_version,
            docker_compose_type
        );
    }
#endif
    if (strlen(product_name)) {
        xml_escape(product_name, pn, sizeof(pn));
        out.printf(
            "    <product_name>%s</product_name>\n",
            pn
        );
    }
    if (strlen(mac_address)) {
        out.printf(
            "    <mac_address>%s</mac_address>\n",
            mac_address
        );
    }
    if (strlen(virtualbox_version)) {
        char buf[256];
        xml_escape(virtualbox_version, buf, sizeof(buf));
        out.printf(
            "    <virtualbox_version>%s</virtualbox_version>\n",
            buf
        );
    }
    if (include_coprocs) {
        this->coprocs.write_xml(out, false);
    }

    // The same CPU can have a different opencl_cpu_prop
    // for each of multiple OpenCL platforms.
    // We send them all to the project server because:
    // - Different OpenCL platforms report different values
    //   for the same CPU
    // - Some OpenCL CPU apps may work better with certain
    //   OpenCL platforms
    //
    for (int i=0; i<num_opencl_cpu_platforms; i++) {
        opencl_cpu_prop[i].write_xml(out);
    }
    out.printf(
        "</host_info>\n"
    );
    return 0;
}

// CPU benchmarks are run in a separate process,
// which communicates its result via a file.
// The following functions read and write this file.
//
int HOST_INFO::parse_cpu_benchmarks(FILE* in) {
    char buf[256];

    char* p = boinc::fgets(buf, 256, in);
    if (!p) return 0;           // Fixes compiler warning
    while (boinc::fgets(buf, 256, in)) {
        if (match_tag(buf, "<cpu_benchmarks>"));
        else if (match_tag(buf, "</cpu_benchmarks>")) return 0;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
    }
    return 0;
}

int HOST_INFO::write_cpu_benchmarks(FILE* out) {
    boinc::fprintf(out,
        "<cpu_benchmarks>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <m_cache>%f</m_cache>\n"
        "</cpu_benchmarks>\n",
        p_fpops,
        p_iops,
        p_membw,
        p_calculated,
        m_cache
    );
    return 0;
}

#ifdef __APPLE__
    // While the official Podman installer puts the Podman executable at
    // "/opt/podman/bin/podman", other installation methods (e.g. brew) might not
    static void find_podman_path(char *path, size_t len) {
    // Mac executables get a very limited PATH environment variable, so we must get the
    // PATH variable used by Terminal and search there for the path to podman
    FILE *f = popen("a=`/usr/libexec/path_helper`;b=${a%\\\"*}\\\";env ${b} which podman", "r");
    if (f) {
        fgets(path, len, f);
        pclose(f);
        char* p = strstr(path, "\n");
        if (p) *p = '\0'; // Remove the newline character
        }
        if (boinc_file_exists(path)) return;

        // If we couldn't get it from that file, use default when installed using Podman installer
        strlcpy(path, "/opt/podman/bin/podman", len);
        if (boinc_file_exists(path)) return;

        // If we couldn't get it from that file, use default when installed by Homebrew
#ifdef __arm64__
        strlcpy(path, "/opt/homebrew/bin/podman", len);
#else
        strlcpy(path, "/usr/local/bin/podman", len);
#endif
        if (boinc_file_exists(path)) return;
        path[0] = '\0'; // Failed to find path to Podman
        return;
    }
#endif

// name of CLI program
//
const char* docker_cli_prog(DOCKER_TYPE type) {
    switch (type) {
    case DOCKER: return "docker";
#ifdef __APPLE__
        case PODMAN:
        static char docker_cli_string[MAXPATHLEN];
        static bool docker_cli_inited = false;
        char path[MAXPATHLEN];
        if (docker_cli_inited) return docker_cli_string;
        docker_cli_string[0] = '\0';
        find_podman_path(path, sizeof(path));
        if (path[0]) {
            strlcpy(docker_cli_string,
                    "\"/Library/Application Support/BOINC Data/Run_Podman\" ",
                    sizeof(docker_cli_string)
                    );
            strlcat(docker_cli_string,
                    path,
                    sizeof(docker_cli_string)
                    );
            strlcat(docker_cli_string, " ", sizeof(docker_cli_string));
        }
        docker_cli_inited = true;
        return docker_cli_string;
#else
    case PODMAN: return "podman";
#endif
    default: break;
    }
    return "unknown";
}

// display name
//
const char* docker_type_str(DOCKER_TYPE type) {
    switch (type) {
    case DOCKER: return "Docker";
    case PODMAN: return "podman";
    default: break;
    }
    return "unknown";
}

// parse a string like
// Docker version 24.0.7, build 24.0.7-0ubuntu2~22.04.1
// or
// podman version 4.9.3
// ... possibly with a \n at the end.
// Return the version (24.0.7 or 4.9.3)
//
bool HOST_INFO::get_docker_version_string(
    DOCKER_TYPE /*type*/, const char* raw, string &version
) {
    char *p, *q;
    const char *prefix;
    prefix = "version ";
    p = (char*)strstr(raw, prefix);
    if (!p) return false;
    p += strlen(prefix);
    q = (char*)strstr(p, ",");
    if (!q) {
        q = (char*)strstr(p, " ");
    }
    if (!q) {
        q = (char*)strstr(p, "\n");
    }
    if (!q) return false;
    *q = 0;
    version = p;
    return true;
}

bool HOST_INFO::get_docker_compose_version_string(
    DOCKER_TYPE type, const char *raw, string& version
) {
    char *p, *q;
    const char* prefix;
    switch (type) {
    case DOCKER:
        // Docker Compose version v2.17.3
        prefix = "Docker Compose version v";
        p = (char*)strstr(raw, prefix);
        if (!p) return false;
        p += strlen(prefix);
        q = (char*)strstr(p, "\n");
        if (q) *q = 0;
        version = p;
        return true;
    // not sure about podman case
    default: break;
    }
    return false;
}

bool HOST_INFO::have_docker() {
#ifdef _WIN32
    for (WSL_DISTRO &wd: wsl_distros.distros) {
        if (!wd.docker_version.empty()) return true;
    }
    return false;
#else
    return strlen(docker_version)>0;
#endif
}
