// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "common_defs.h"
#include "client_msgs.h"
#include "cs_proxy.h"

PROXY_INFO gui_proxy_info;
PROXY_INFO env_var_proxy_info;
PROXY_INFO config_proxy_info;
PROXY_INFO working_proxy_info;

static void show_proxy_info(PROXY_INFO& p) {
    if (p.use_http_proxy) {
        if (p.http_server_port) {
            msg_printf(NULL, MSG_INFO, "Using HTTP proxy %s:%d",
                p.http_server_name, p.http_server_port
            );
        } else {
            msg_printf(NULL, MSG_INFO, "Using HTTP proxy %s",
                p.http_server_name
            );
        }
    }
    if (p.use_socks_proxy) {
        if (p.socks_server_port) {
            msg_printf(NULL, MSG_INFO, "Using SOCKS proxy %s:%d",
                p.socks_server_name, p.socks_server_port
            );
        } else {
            msg_printf(NULL, MSG_INFO, "Using SOCKS proxy %s",
                p.socks_server_name
            );
        }
    }
#if 0
    if (!p.use_http_proxy && !p.use_socks_proxy) {
        msg_printf(NULL, MSG_INFO, "Not using a proxy");
    }
#endif
}

void select_proxy_info() {
    if (gui_proxy_info.present) {
        working_proxy_info = gui_proxy_info;
        msg_printf(0, MSG_INFO, "Using proxy info from GUI");
        if (env_var_proxy_info.present) {
            msg_printf(0, MSG_INFO, "Proxy info env vars overridden by GUI");
        }
        if (config_proxy_info.present) {
            msg_printf(0, MSG_INFO, "Config file proxy info overridden by GUI");
        }
    } else if (config_proxy_info.present) {
        working_proxy_info = config_proxy_info;
        msg_printf(0, MSG_INFO, "Using proxy info from cc_config.xml");
        if (env_var_proxy_info.present) {
            msg_printf(0, MSG_INFO, "Proxy info env vars overridden by cc_config.xml");
        }
    } else if (env_var_proxy_info.present) {
        working_proxy_info = env_var_proxy_info;
        msg_printf(0, MSG_INFO, "Using proxy info from environment variables");
    }
    show_proxy_info(working_proxy_info);
}

void proxy_info_startup() {
    select_proxy_info();
    working_proxy_info.need_autodetect_proxy_settings = true;
    working_proxy_info.have_autodetect_proxy_settings = false;
}
