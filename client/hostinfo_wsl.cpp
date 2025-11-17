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

// enumerate the WSL distros on this host.
// For each one, see if it contains Podman or Docker, and get the version

#include "boinc_win.h"
#include "win_util.h"

#include "error_numbers.h"
#include "str_replace.h"
#include "client_state.h"
#include "client_msgs.h"
#include "hostinfo.h"
#include "util.h"

using std::vector;
using std::string;

// timeout for commands run in WSL container
// If something goes wrong we don't want client to hang
//
#define CMD_TIMEOUT 10.0

static void get_docker_version(WSL_CMD&, WSL_DISTRO&);
static void get_docker_compose_version(WSL_CMD&, WSL_DISTRO&);

// scan the registry to get the list of all WSL distros on this host.
// See https://patrickwu.space/2020/07/19/wsl-related-registry/
//
int get_all_distros(WSL_DISTROS& distros) {
    const std::string lxss_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Lxss";

    HKEY hKey;

    // look up main entry (Lxss)
    //
    LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
        lxss_path.c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey
    );
    if (lRet == ERROR_FILE_NOT_FOUND) {
      msg_printf(0, MSG_INFO, "WSL: registry key not found; assuming no WSL distros are installed");
      return 0;
    }
    if (lRet != ERROR_SUCCESS) {
        msg_printf(0, MSG_INFO, "WSL: registry open failed (error %ld)", lRet);
        return -1;
    }

    const int buf_len = 256;
    char default_wsl_guid[buf_len];
    DWORD default_wsl_guid_len = sizeof(default_wsl_guid);

    lRet = RegQueryValueEx(hKey, "DefaultDistribution", NULL, NULL,
        (LPBYTE)default_wsl_guid, &default_wsl_guid_len
    );
    if ((lRet != ERROR_SUCCESS) || (default_wsl_guid_len > buf_len)) {
        msg_printf(0, MSG_INFO, "WSL: registry query for DefaultDistribution failed (error %ld)", lRet);
        RegCloseKey(hKey);
        return 0;
    }

    // scan subkeys (one per distro)
    //
    int i = 0;
    while(true) {
        char wsl_guid[buf_len];
        DWORD wsl_guid_len = sizeof(wsl_guid);

        LONG ret = RegEnumKeyEx(
            hKey, i++, wsl_guid, &wsl_guid_len, NULL, NULL, NULL, NULL
        );
        if (ret != ERROR_SUCCESS) {
            break;
        }

        HKEY hSubKey;
        const std::string sub_key = lxss_path + "\\" + wsl_guid;
        ret = RegOpenKeyEx(HKEY_CURRENT_USER,
            sub_key.c_str(), 0, KEY_QUERY_VALUE, &hSubKey
        );
        if (ret != ERROR_SUCCESS) {
            break;
        }

        DWORD wsl_state = 0;
        DWORD wsl_state_len = sizeof(wsl_state);
        ret = RegQueryValueEx(
            hSubKey, "State", NULL, NULL, (LPBYTE)&wsl_state, &wsl_state_len
        );
        if (ret != ERROR_SUCCESS || wsl_state != 1) {
            RegCloseKey(hSubKey);
            continue;
        }

        DWORD wsl_version = 1;
        DWORD wsl_version_len = sizeof(wsl_version);
        // there might be no version key, so we ignore the return value
        RegQueryValueEx(
            hSubKey, "Version", NULL, NULL, (LPBYTE)&wsl_version,
            &wsl_version_len
        );

        char wsl_name[buf_len];
        DWORD wsl_name_len = sizeof(wsl_name);
        ret = RegQueryValueEx(hSubKey, "DistributionName", NULL, NULL,
            (LPBYTE)wsl_name, &wsl_name_len
        );
        if ((ret == ERROR_SUCCESS) && (wsl_name_len < buf_len)) {
            WSL_DISTRO distro;
            distro.distro_name = wsl_name;
            distro.wsl_version = wsl_version;
            if (!strcmp(wsl_guid, default_wsl_guid)) {
                distro.is_default = true;
            }
            distros.distros.push_back(distro);
        }
        RegCloseKey(hSubKey);
    }

    RegCloseKey(hKey);

    // if boinc-buda-runner is present, ignore others
    //
    for (WSL_DISTRO &wd: distros.distros) {
        if (wd.distro_name == BOINC_WSL_DISTRO_NAME) {
            WSL_DISTRO distro = wd;
            distros.distros.clear();
            distros.distros.push_back(distro);
            break;
        }
    }

    return 0;
}

// parse the output of 'sysctl -a' to get OS name and version
//
void parse_sysctl_output(
    const std::vector<std::string>& lines,
    std::string& ostype, std::string& osrelease
) {
    char buf[256], ostype_found[256], osrelease_found[256];
    ostype.clear();
    osrelease.clear();
    for (size_t i = 0; i < lines.size(); ++i) {
        safe_strcpy(buf, lines[i].c_str());
        strip_whitespace(buf);
        if (strstr(buf, "kernel.ostype =")) {
            safe_strcpy(ostype_found, strchr(buf, '=') + 1);
            ostype = ostype_found;
            strip_whitespace(ostype);
            continue;
        }
        if (strstr(buf, "kernel.osrelease =")) {
            safe_strcpy(osrelease_found, strchr(buf, '=') + 1);
            osrelease = osrelease_found;
            strip_whitespace(osrelease);
        }
    }
}

// if either name or version is not already there, add
//
static void update_os(
    WSL_DISTRO &wd, const char* os_name, const char* os_version
) {
    if (wd.os_name.empty() && strlen(os_name)) {
        wd.os_name = os_name;
    }
    if (wd.os_version.empty() && strlen(os_version)) {
        wd.os_version = os_version;
    }
}

// have both OS name and version?
//
static bool got_both(WSL_DISTRO &wd) {
    return !wd.os_name.empty() && !wd.os_version.empty();
}

// Get list of WSL distros usable by BOINC
// For each of them:
//      try to find the OS name and version
//      see if Docker and docker compose are present, get versions
// Return nonzero on error
//
int get_wsl_information(WSL_DISTROS &distros) {
    WSL_DISTROS all_distros;
    distros.distros.clear();
    int retval = get_all_distros(all_distros);
    if (retval) return retval;
    if (all_distros.distros.empty()) {
        return 0;
    }

    string err_msg;
    string reply;
    WSL_CMD rs;

    if (rs.setup(err_msg)) {
        msg_printf(0, MSG_INFO, "WSL unavailable: %s", err_msg.c_str());
        return 0;
    }

    // loop over all WSL distros
    for (WSL_DISTRO &wd: all_distros.distros) {
        // skip 'docker-desktop-data'
        // See: https://stackoverflow.com/a/61431088/4210508
        if (wd.distro_name == "docker-desktop-data"){
            continue;
        }

        char os_name[256];
        char os_version[256];
        strcpy(os_name, "");
        strcpy(os_version, "");

        // Try to get the name and version of the OS in the WSL distro.
        // There are several ways of doing this

        // try running 'lsb_release -a'
        //
        if (!rs.run_program_in_wsl(wd.distro_name, command_lsbrelease)) {
            read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
            HOST_INFO::parse_linux_os_info(
                reply, lsbrelease,
                os_name, sizeof(os_name),
                os_version, sizeof(os_version)
            );
            CloseHandle(rs.proc_handle);
            update_os(wd, os_name, os_version);
        } else {
            // if failure, skip this distro, but try others;
            // might be a problem with this distro
            continue;
        }

        // try reading '/etc/os-relese'
        //
        if (!got_both(wd)) {
            const std::string command_osrelease = "cat " + std::string(file_osrelease);
            if (!rs.run_program_in_wsl( wd.distro_name, command_osrelease)) {
                read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
                HOST_INFO::parse_linux_os_info(
                    reply, osrelease,
                    os_name, sizeof(os_name),
                    os_version, sizeof(os_version)
                );
                CloseHandle(rs.proc_handle);
                update_os(wd, os_name, os_version);
            } else {
                continue;
            }
        }

        // try reading '/etc/redhatrelease'
        //
        if (!got_both(wd)) {
            const std::string command_redhatrelease = "cat " + std::string(file_redhatrelease);
            if (!rs.run_program_in_wsl(wd.distro_name, command_redhatrelease)) {
                read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
                HOST_INFO::parse_linux_os_info(
                    reply, redhatrelease,
                    os_name, sizeof(os_name),
                    os_version, sizeof(os_version)
                );
                CloseHandle(rs.proc_handle);
                update_os(wd, os_name, os_version);
            } else {
                continue;
            }
        }

        std::string os_name_str = "";
        std::string os_version_str = "";

        // try running 'sysctl -a'
        //
        if (!got_both(wd)) {
            const std::string command_sysctl = "sysctl -a";
            if (!rs.run_program_in_wsl(
                wd.distro_name, command_sysctl
            )) {
                read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
                parse_sysctl_output(
                    split(reply, '\n'),
                    os_name_str, os_version_str
                );
                CloseHandle(rs.proc_handle);
                update_os(wd, os_name_str.c_str(), os_version_str.c_str());
            } else {
                continue;
            }
        }

        // try running 'uname -s'
        //
        if (!got_both(wd)) {
            const std::string command_uname_s = "uname -s";
            if (!rs.run_program_in_wsl(
                wd.distro_name, command_uname_s
            )) {
                read_from_pipe(rs.out_read, rs.proc_handle, os_name_str, CMD_TIMEOUT);
                strip_whitespace(os_name_str);
                CloseHandle(rs.proc_handle);
                update_os(wd, os_name_str.c_str(), "");
            } else {
                continue;
            }
        }

        // try running 'uname -r'
        //
        if (!got_both(wd)) {
            const std::string command_uname_r = "uname -r";
            if (!rs.run_program_in_wsl(
                wd.distro_name, command_uname_r
            )) {
                read_from_pipe(rs.out_read, rs.proc_handle, os_version_str, CMD_TIMEOUT);
                strip_whitespace(os_version_str);
                CloseHandle(rs.proc_handle);
                update_os(wd, "", os_version_str.c_str());
            } else {
                continue;
            }
        }

        // in case nothing worked
        update_os(wd, "unknown", "unknown");

        // get the libc version by running 'ldd --version'
        // on most distros this generates something like
        // ldd (Ubuntu GLIBC 2.27-3ubuntu1.6) 2.27
        // ...
        // NOTE: on Alpine this generates
        // musl libc (x86_64)
        // Version 1.2.5
        // ...
        // We currently don't parse this.
        //
        if (!rs.run_program_in_wsl(
            wd.distro_name, "ldd --version"
        )) {
            string buf;
            read_from_pipe(rs.out_read, rs.proc_handle, buf, CMD_TIMEOUT);
            wd.libc_version = parse_ldd_libc(buf.c_str());
        }

        // see if Docker is installed in the distro
        //
        get_docker_version(rs, wd);
        get_docker_compose_version(rs, wd);

        // see if distro is disallowed
        //
        vector<string> &dw = cc_config.disallowed_wsls;
        if (std::find(dw.begin(), dw.end(), wd.distro_name) != dw.end()) {
            wd.disallowed = true;
        }

        // if it's boinc-buda-runner, look for version file
        //
        if (wd.distro_name == BOINC_WSL_DISTRO_NAME) {
            wd.boinc_buda_runner_version = 1;
            if (!rs.run_program_in_wsl(
                wd.distro_name, "cat /home/boinc/version.txt"
            )) {
                string buf;
                char buf2[256];
                read_from_pipe(rs.out_read, rs.proc_handle, buf, CMD_TIMEOUT);
                safe_strcpy(buf2, buf.c_str());
                char *p = strstr(buf2, "version: ");
                if (p) {
                    wd.boinc_buda_runner_version = atoi(p+strlen("version: "));
                }
            }
        }

        distros.distros.push_back(wd);
    }

    return 0;
}

static bool get_docker_version_aux(
    WSL_CMD &rs, WSL_DISTRO &wd, DOCKER_TYPE type
) {
    bool ret = false;
    string reply;
    string cmd = string(docker_cli_prog(type)) + " --version";
    if (!rs.run_program_in_wsl(wd.distro_name, cmd.c_str())) {
        read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
        string version;
        if (HOST_INFO::get_docker_version_string(type, reply.c_str(), version)) {
            wd.docker_version = version;
            wd.docker_type = type;
            ret = true;
            if (version.empty()) {
                msg_printf(0, MSG_INFO,
                    "Docker version parse failed: %s", reply.c_str()
                );
            }
        } else {
            msg_printf(0, MSG_INFO, "Docker detection in %s:",
                wd.distro_name.c_str()
            );
            msg_printf(0, MSG_INFO, "-   cmd: %s", cmd.c_str());
            msg_printf(0, MSG_INFO, "-   output: %s", reply.c_str());
        }
        CloseHandle(rs.proc_handle);
    } else {
        msg_printf(0, MSG_INFO, "Docker detection in %s:",
            wd.distro_name.c_str()
        );
        msg_printf(0, MSG_INFO, "-   cmd failed: %s", cmd.c_str());
    }
    return ret;
}

static void get_docker_version(WSL_CMD &rs, WSL_DISTRO &wd) {
    if (get_docker_version_aux(rs, wd, PODMAN)) return;
    get_docker_version_aux(rs, wd, DOCKER);
}

static bool get_docker_compose_version_aux(
    WSL_CMD &rs, WSL_DISTRO &wd, DOCKER_TYPE type
) {
    bool ret = false;
    string reply;
    string cmd = string(docker_cli_prog(type)) + " compose version";
    if (!rs.run_program_in_wsl(wd.distro_name, cmd.c_str())) {
        read_from_pipe(rs.out_read, rs.proc_handle, reply, CMD_TIMEOUT);
        string version;
        if (HOST_INFO::get_docker_compose_version_string(
            type, reply.c_str(), version
        )) {
            wd.docker_compose_version = version;
            wd.docker_compose_type = type;
            ret = true;
        }
        CloseHandle(rs.proc_handle);
    }
    return false;
}

static void get_docker_compose_version(WSL_CMD& rs, WSL_DISTRO &wd) {
    if (get_docker_compose_version_aux(rs, wd, PODMAN)) return;
    get_docker_compose_version_aux(rs, wd, DOCKER);
}

// called on startup (after scanning distros)
// and after doing an RPC to get latest version info.
//
void show_wsl_messages() {
    int bdv = gstate.host_info.wsl_distros.boinc_distro_version();
    const char *url = "https://github.com/BOINC/boinc/wiki/Installing-Docker-on-Windows";
    if (bdv == 0) {
        msg_printf_notice(0, true, url,
            "Some BOINC projects require Docker.  <a href=%s>Learn how to install it</a>",
            url
        );
    } else if (bdv < gstate.latest_boinc_buda_runner_version) {
        const char *url2 = "https://github.com/BOINC/boinc/wiki/Updating-the-BOINC-WSL-distro";
        msg_printf_notice(0, true, url,
            "A new version of the BOINC WSL distro is available.  <a href=%s>Learn how to upgrade.</a>",
            url2
        );
    }
}
