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

#ifdef _WIN64

#include "boinc_win.h"

#include "str_replace.h"
#include "client_msgs.h"
#include "hostinfo.h"

bool get_available_wsls(
    std::vector<std::pair<std::string,
    DWORD>>& wsls, std::string& default_wsl
) {
    const std::string lxss_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Lxss";

    HKEY hKey;

    default_wsl.clear();

    LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
        lxss_path.c_str(),
        0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey
    );
    if (lRet != ERROR_SUCCESS) {
        return false;
    }

    const int buf_len = 256;
    char default_wsl_guid[buf_len];
    DWORD default_wsl_guid_len = sizeof(default_wsl_guid);

    lRet = RegQueryValueEx(hKey, "DefaultDistribution", NULL, NULL,
        (LPBYTE)default_wsl_guid, &default_wsl_guid_len
    );
    if ((lRet != ERROR_SUCCESS) || (default_wsl_guid_len > buf_len)) {
        return false;
    }

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
            sub_key.c_str(),
            0, KEY_QUERY_VALUE, &hSubKey
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
            wsls.push_back(std::make_pair(wsl_name, wsl_version));
            if (std::string(wsl_guid) == std::string(default_wsl_guid)) {
                default_wsl = wsl_name;
            }
            RegCloseKey(hSubKey);
        }
    }

    RegCloseKey(hKey);

    return !default_wsl.empty();
}

typedef HRESULT(WINAPI *PWslLaunch)(PCWSTR, PCWSTR, BOOL, HANDLE, HANDLE, HANDLE, HANDLE*);

struct WSL_RESOURCE_MANAGER {
    HINSTANCE wsl_lib = NULL;
    HANDLE in_read = NULL;
    HANDLE in_write = NULL;
    HANDLE out_read = NULL;
    HANDLE out_write = NULL;
    PWslLaunch pWslLaunch = NULL;

    ~WSL_RESOURCE_MANAGER() {
        close_handle(in_read);
        close_handle(in_write);
        close_handle(out_read);
        close_handle(out_write);

        if (wsl_lib) {
            FreeLibrary(wsl_lib);
        }
    }

    // prepare resources
    int prepare_resources() {
        wsl_lib = NULL;
        in_read = NULL;
        in_write = NULL;
        out_read = NULL;
        out_write = NULL;
        pWslLaunch = NULL;

        wsl_lib = LoadLibrary("wslapi.dll");
        if (!wsl_lib) {
            return 1;
        }

        pWslLaunch = (PWslLaunch)GetProcAddress(wsl_lib, "WslLaunch");

        if (!pWslLaunch) {
            return 1;
        }

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&out_read, &out_write, &sa, 0)) {
            return 1;
        }
        if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)) {
            return 1;
        }
        if (!CreatePipe(&in_read, &in_write, &sa, 0)) {
            return 1;
        }
        if (!SetHandleInformation(in_write, HANDLE_FLAG_INHERIT, 0)) {
            return 1;
        }

        return 0;
    }

private:
    inline void close_handle(HANDLE handle) {
        if (handle) {
            CloseHandle(handle);
        }
    }
};

//convert std::string to PCWSTR
//taken from https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
std::wstring s2ws(const std::string& s) {
    const int slength = (int)s.length() + 1;
    const int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

bool create_wsl_process(
    const WSL_RESOURCE_MANAGER& rs, const std::string& wsl_distro_name,
    const std::string& command, HANDLE* handle,
    bool use_current_work_dir = false
) {
    HRESULT ret = rs.pWslLaunch(
        s2ws(wsl_distro_name).c_str(), s2ws(command).c_str(),
        use_current_work_dir, rs.in_read, rs.out_write, rs.out_write,
        handle
    );
    return (ret == S_OK);
}

bool CreateWslProcess(
    const HANDLE& out_write, const std::string& wsl_app,
    const std::string& command, HANDLE& handle
) {
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));

    si.cb = sizeof(STARTUPINFO);
    si.hStdError = out_write;
    si.hStdOutput = out_write;
    si.hStdInput = NULL;
    si.dwFlags |= STARTF_USESTDHANDLES;

    const DWORD dwFlags = CREATE_NO_WINDOW;

    const std::string cmd = wsl_app + " " + command;

    const bool res = (CreateProcess(
        NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, dwFlags,
        NULL, NULL, &si, &pi
    ) != FALSE);

    if (res) {
        handle = pi.hProcess;
        CloseHandle(pi.hThread);
    }

    return res;
}

std::string read_from_pipe(const HANDLE& handle, const HANDLE& out_read) {
    DWORD avail, read, exitcode;
    const int bufsize = 256;
    char buf[bufsize];
    std::string res = "";

    for (;;) {
        PeekNamedPipe(out_read, NULL, 0, NULL, &avail, NULL);

        if (avail) {
            if (!ReadFile(out_read, buf, bufsize - 1, &read, NULL) || read == 0) {
                break;
            }

            buf[read] = '\0';
            res += buf;
        } else {
            if (!GetExitCodeProcess(handle, &exitcode) || exitcode != STILL_ACTIVE) {
                break;
            }
            Sleep(200);
        }
    }

    return res;
}

void parse_sysctl_output(
    const std::vector<std::string>& lines, std::string& ostype,
    std::string& osrelease
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

// Returns the OS name and version for WSL when enabled
//
bool get_wsl_information(
    std::vector<std::string> allowed_wsls, bool& wsl_available,
    WSLS& wsls, bool detect_docker, bool& docker_available,
    bool& docker_compose_available
) {
    std::vector<std::pair<std::string, DWORD>> distros;
    std::string default_distro;

    if (!get_available_wsls(distros, default_distro)) {
        return false;
    }

    WSL_RESOURCE_MANAGER rs;

    if (rs.prepare_resources()) {
        return false;
    }

    wsl_available = false;
    docker_available = false;
    docker_compose_available = false;

    HANDLE handle;

    for (size_t i = 0; i < distros.size(); ++i) {
        char wsl_dist_name[256];
        char wsl_dist_version[256];

        const std::string& distro = distros[i].first;
        // skip 'docker-desktop-data'
        // Ref: https://stackoverflow.com/a/61431088/4210508
        if (distro == "docker-desktop-data"){
            continue;
        }
        // skip distros that are not allowed except for 'docker-desktop'
        if (distro != "docker-desktop" && std::find(allowed_wsls.begin(), allowed_wsls.end(), distro) == allowed_wsls.end()) {
            msg_printf(0, MSG_INFO, "WSL distro '%s' detected but is not allowed", distro.c_str());
            continue;
        }

        WSL wsl;
        wsl.distro_name = distro;
        if (distro == default_distro) {
            wsl.is_default = true;
        } else {
            wsl.is_default = false;
        }
        wsl.wsl_version = std::to_string(distros[i].second);

        // lsbrelease
        if (!create_wsl_process(rs, distro, command_lsbrelease, &handle)) {
            continue;
        }
        wsl_available = HOST_INFO::parse_linux_os_info(
            read_from_pipe(handle, rs.out_read), lsbrelease,
            wsl_dist_name, sizeof(wsl_dist_name), wsl_dist_version,
            sizeof(wsl_dist_version)
        );
        CloseHandle(handle);

        if (!wsl_available) {
            //osrelease
            const std::string command_osrelease = "cat " + std::string(file_osrelease);
            if (!create_wsl_process(rs, distro, command_osrelease, &handle)) {
                continue;
            }
            wsl_available = HOST_INFO::parse_linux_os_info(
                read_from_pipe(handle, rs.out_read),
                osrelease,
                wsl_dist_name, sizeof(wsl_dist_name),
                wsl_dist_version, sizeof(wsl_dist_version)
            );
            CloseHandle(handle);
        }

        //redhatrelease
        if (!wsl_available) {
            const std::string command_redhatrelease = "cat " + std::string(file_redhatrelease);
            if (!create_wsl_process(rs, distro, command_redhatrelease, &handle)) {
                continue;
            }
            wsl_available = HOST_INFO::parse_linux_os_info(
                read_from_pipe(handle, rs.out_read),
                redhatrelease, wsl_dist_name, sizeof(wsl_dist_name),
                wsl_dist_version, sizeof(wsl_dist_version)
            );
            CloseHandle(handle);
        }

        if (!wsl_available) {
            continue;
        }

        std::string os_name = "";
        std::string os_version_extra = "";

        // sysctl -a
        const std::string command_sysctl = "sysctl -a";
        if (create_wsl_process(rs, distro, command_sysctl, &handle)) {
            parse_sysctl_output(
                split(read_from_pipe(handle, rs.out_read), '\n'),
                os_name, os_version_extra
            );
            CloseHandle(handle);
        }

        // uname -s
        if (os_name.empty()) {
            const std::string command_uname_s = "uname -s";
            if (create_wsl_process(rs, distro, command_uname_s, &handle)) {
                os_name = read_from_pipe(handle, rs.out_read);
                strip_whitespace(os_name);
                CloseHandle(handle);
            }
        }

        // uname -r
        if (os_version_extra.empty()) {
            const std::string command_uname_r = "uname -r";
            if (create_wsl_process(rs, distro, command_uname_r ,&handle)) {
                os_version_extra = read_from_pipe(handle, rs.out_read);
                strip_whitespace(os_version_extra);
                CloseHandle(handle);
            }
        }

        if (!os_name.empty()) {
            wsl.os_name = os_name + " " + wsl_dist_name;
        } else {
            wsl.os_name = wsl_dist_name;
        }
        if (!os_version_extra.empty()) {
            wsl.os_version = std::string(wsl_dist_version) + " [" + os_version_extra + "]";
        }
        else {
            wsl.os_version = wsl_dist_version;
        }

        if (detect_docker) {
            if (create_wsl_process(
                rs, distro, command_get_docker_version, &handle
            )) {
                std::string raw = read_from_pipe(handle, rs.out_read);
                std::string version;
                wsl.is_docker_available = HOST_INFO::get_docker_version_string(raw, version);
                if (wsl.is_docker_available) {
                    docker_available = true;
                    wsl.docker_version = version;
                }
                CloseHandle(handle);
            }
            if (create_wsl_process(
                rs, distro, command_get_docker_compose_version, &handle
            )) {
                std::string raw = read_from_pipe(handle, rs.out_read);
                std::string version;
                wsl.is_docker_compose_available = HOST_INFO::get_docker_compose_version_string(raw, version);
                if (wsl.is_docker_compose_available) {
                    docker_compose_available = true;
                    wsl.docker_compose_version = version;
                }
                CloseHandle(handle);
            }
        }

        wsls.wsls.push_back(wsl);
    }

    return true;
}

#endif // _WIN64
