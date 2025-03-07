// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#endif

#include "parse.h"
#include "filesys.h"
#include "boinc_api.h"
#include "vboxlogging.h"
#include "vboxjob.h"


VBOX_INTERMEDIATE_UPLOAD::VBOX_INTERMEDIATE_UPLOAD() {
    clear();
}

VBOX_INTERMEDIATE_UPLOAD::~VBOX_INTERMEDIATE_UPLOAD() {
    clear();
}

void VBOX_INTERMEDIATE_UPLOAD::clear() {
    file = "";
    reported = false;
    ignore = false;
}


VBOX_PORT_FORWARD::VBOX_PORT_FORWARD() {
    clear();
}

VBOX_PORT_FORWARD::~VBOX_PORT_FORWARD() {
    clear();
}

void VBOX_PORT_FORWARD::clear() {
    host_port = 0;
    guest_port = 0;
    is_remote = false;
    nports = 0;
}

int VBOX_PORT_FORWARD::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/port_forward")) {
            if (!host_port) {
                vboxlog_msg("VBOX_PORT_FORWARD::parse(): unspecified host port");
                return ERR_XML_PARSE;
            }
            if (!guest_port) {
                vboxlog_msg("VBOX_PORT_FORWARD::parse(): unspecified guest port");
                return ERR_XML_PARSE;
            }
            return 0;
        }
        else if (xp.parse_bool("is_remote", is_remote)) continue;
        else if (xp.parse_int("host_port", host_port)) continue;
        else if (xp.parse_int("guest_port", guest_port)) continue;
        else if (xp.parse_int("nports", nports)) continue;
        else {
            vboxlog_msg("VBOX_PORT_FORWARD::parse(): unexpected text %s", xp.parsed_tag);
        }
    }
    return ERR_XML_PARSE;
}


VBOX_JOB::VBOX_JOB() {
    clear();
}

VBOX_JOB::~VBOX_JOB() {
    clear();
}

void VBOX_JOB::clear() {
    os_name.clear();
    memory_size_mb = 0.0;
    job_duration = 0.0;
    minimum_checkpoint_interval = 600.0;
    minimum_heartbeat_interval = 600.0;
    fraction_done_filename.clear();
    heartbeat_filename.clear();
    completion_trigger_file.clear();
    temporary_exit_trigger_file.clear();
    multiattach_vdi_file.clear();
    enable_cern_dataformat = false;
    enable_shared_directory = false;
    enable_scratch_directory = false;
    share_slot_dir = false;
    share_project_dir = false;
    enable_floppyio = false;
    enable_cache_disk = false;
    enable_isocontextualization = false;
    vm_network_driver.clear();
    enable_network = false;
    network_bridged_mode = false;
    enable_nat_dns_host_resolver = false;
    enable_remotedesktop = false;
    enable_screenshots_on_error = false;
    enable_graphics_support = false;
    enable_vm_savestate_usage = false;
    disable_automatic_checkpoints = false;
    boot_iso = false;
    pf_guest_port = 0;
    pf_host_port = 0;
    port_forwards.clear();
    intermediate_upload_files.clear();
    copy_cmdline_to_shared = false;

    // Initialize default values
    vm_disk_controller_type = "sata";
    vm_disk_controller_model = "IntelAHCI";
    vm_graphics_controller_type = "VBoxVGA";
    vram_size_mb = VBOX_VRAM_MIN;

}

int VBOX_JOB::parse() {
    MIOFILE mf;
    std::string str;
    char buf[1024];

    boinc_resolve_filename(JOB_FILENAME, buf, sizeof(buf));
    FILE* f = boinc_fopen(buf, "r");
    if (!f) {
        vboxlog_msg("VBOX_JOB::parse(): can't open job file %s", buf);
        return ERR_FOPEN;
    }
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("vbox_job")) return ERR_XML_PARSE;
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            vboxlog_msg("VBOX_JOB::parse(): unexpected text %s", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/vbox_job")) {
            fclose(f);
            return 0;
        }
        else if (xp.parse_string("vm_disk_controller_type", vm_disk_controller_type)) continue;
        else if (xp.parse_string("vm_disk_controller_model", vm_disk_controller_model)) continue;
        else if (xp.parse_string("os_name", os_name)) continue;
        else if (xp.parse_double("vram_size_mb", vram_size_mb)) {
            // keep it within the valid range
            if (vram_size_mb < VBOX_VRAM_MIN) {
                vram_size_mb = VBOX_VRAM_MIN;
            } else if (vram_size_mb > VBOX_VRAM_MAX) {
                vram_size_mb = VBOX_VRAM_MAX;
            }
            continue;
        }
        else if (xp.parse_double("memory_size_mb", memory_size_mb)) continue;
        else if (xp.parse_double("job_duration", job_duration)) continue;
        else if (xp.parse_double("minimum_checkpoint_interval", minimum_checkpoint_interval)) continue;
        else if (xp.parse_double("minimum_heartbeat_interval", minimum_heartbeat_interval)) continue;
        else if (xp.parse_string("fraction_done_filename", fraction_done_filename)) continue;
        else if (xp.parse_string("heartbeat_filename", heartbeat_filename)) continue;
        else if (xp.parse_string("completion_trigger_file", completion_trigger_file)) continue;
        else if (xp.parse_string("temporary_exit_trigger_file", temporary_exit_trigger_file)) continue;
        else if (xp.parse_string("multiattach_vdi_file", multiattach_vdi_file)) continue;
        else if (xp.parse_bool("enable_cern_dataformat", enable_cern_dataformat)) continue;
        else if (xp.parse_string("vm_network_driver", vm_network_driver)) continue;
        else if (xp.parse_bool("enable_network", enable_network)) continue;
        else if (xp.parse_bool("network_bridged_mode", network_bridged_mode)) continue;
        else if (xp.parse_bool("enable_nat_dns_host_resolver", enable_nat_dns_host_resolver)) continue;
        else if (xp.parse_bool("enable_shared_directory", enable_shared_directory)) continue;
        else if (xp.parse_bool("enable_scratch_directory", enable_scratch_directory)) continue;
        else if (xp.parse_bool("share_slot_dir", share_slot_dir)) continue;
        else if (xp.parse_bool("share_project_dir", share_project_dir)) continue;
        else if (xp.parse_bool("enable_floppyio", enable_floppyio)) continue;
        else if (xp.parse_bool("enable_cache_disk", enable_cache_disk)) continue;
        else if (xp.parse_bool("enable_isocontextualization", enable_isocontextualization)) continue;
        else if (xp.parse_bool("enable_remotedesktop", enable_remotedesktop)) continue;
        else if (xp.parse_bool("enable_screenshots_on_error", enable_screenshots_on_error)) continue;
        else if (xp.parse_bool("enable_graphics_support", enable_graphics_support)) continue;
        else if (xp.parse_bool("enable_vm_savestate_usage", enable_vm_savestate_usage)) continue;
        else if (xp.parse_bool("disable_automatic_checkpoints", disable_automatic_checkpoints)) continue;
        else if (xp.parse_bool("boot_iso", boot_iso)) continue;
        else if (xp.parse_int("pf_guest_port", pf_guest_port)) continue;
        else if (xp.parse_int("pf_host_port", pf_host_port)) continue;
        else if (xp.parse_string("copy_to_shared", str)) {
            copy_to_shared.push_back(str);
            continue;
        }
        else if (xp.parse_bool("copy_cmdline_to_shared", copy_cmdline_to_shared)) continue;
        else if (xp.parse_string("trickle_trigger_file", str)) {
            trickle_trigger_files.push_back(str);
            continue;
        }
        else if (xp.parse_string("intermediate_upload_file", str)) {
            VBOX_INTERMEDIATE_UPLOAD iu;
            iu.clear();
            iu.file = str;
            intermediate_upload_files.push_back(iu);
            continue;
        }
        else if (xp.match_tag("port_forward")) {
            VBOX_PORT_FORWARD pf;
            pf.clear();
            pf.parse(xp);
            if (pf.nports) {
                VBOX_PORT_FORWARD pf_range;
                pf_range = pf;
                pf_range.nports = 0;
                for (int i = 0; i < pf.nports; ++i) {
                    port_forwards.push_back(pf_range);
                    pf_range.host_port++;
                    pf_range.guest_port++;
                }
            } else {
                port_forwards.push_back(pf);
            }
        }
        vboxlog_msg("VBOX_JOB::parse(): unexpected text %s", xp.parsed_tag);
    }
    fclose(f);
    return ERR_XML_PARSE;
}

