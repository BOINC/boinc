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

#ifndef BOINC_VBOXJOB_H
#define BOINC_VBOXJOB_H


#define JOB_FILENAME "vbox_job.xml"


#ifndef VBOX_VRAM_MIN
// Default value suggested by VirtualBox v6.1.30
// when a VM is created manually using a Linux 64-bit template.
// VirtualBox does not complain if lower values are used (9-16 MB)
// but certain VMs occasionally hang when they boot (Rosetta).
// They don't hang if vram is at least 16 MB.
#define VBOX_VRAM_MIN 16.0
#endif
#ifndef VBOX_VRAM_MAX
// highest value currently accepted by VirtualBox v6.1.30
#define VBOX_VRAM_MAX 128.0
#endif


// Represents the state of a intermediate upload
class VBOX_INTERMEDIATE_UPLOAD {
public:
    VBOX_INTERMEDIATE_UPLOAD();
    ~VBOX_INTERMEDIATE_UPLOAD();

    void clear();

    std::string file;
    bool reported;
    bool ignore;
};


class VBOX_PORT_FORWARD {
public:
    VBOX_PORT_FORWARD();
    ~VBOX_PORT_FORWARD();

    void clear();
    int parse(XML_PARSER& xp);

    int host_port;          // 0 means assign dynamically
    int guest_port;
    bool is_remote;
    int nports;
};


class VBOX_JOB {
public:
    VBOX_JOB();
    ~VBOX_JOB();

    void clear();
    int parse();

    std::string os_name;
        // name of the OS the VM runs

    std::string vm_disk_controller_type;
        // the type of disk controller to emulate

    std::string vm_disk_controller_model;
        // the disk controller model to emulate

    std::string vm_graphics_controller_type;
        // the graphics controller type to emulate

    double vram_size_mb;
        // size of the video memory allocation for the VM, in megabytes
        // should be between VBOX_VRAM_MIN and VBOX_VRAM_MAX MB
        // default: VBOX_VRAM_MIN MB

    double memory_size_mb;
        // size of the memory allocation for the VM, in megabytes

    bool enable_cern_dataformat;
        // whether to use CERN specific data structures

    bool enable_isocontextualization;
        // whether to use an iso9660 image to implement VM contextualization (e.g. uCernVM)

    bool enable_cache_disk;
        // whether to add an extra cache disk for systems like uCernVM

    bool boot_iso;
        // whether to put the iso as the first boot device

    bool enable_network;
        // whether to allow network access

    bool network_bridged_mode;
        // use bridged mode for network

    bool enable_shared_directory;
        // whether to use shared directory infrastructure

    bool enable_scratch_directory;
        // whether to use scratch directory infrastructure

    bool enable_floppyio;
        // whether to use floppy io infrastructure

    bool enable_remotedesktop;
        // whether to enable remote desktop functionality

    bool enable_gbac;
        // whether to enable GBAC functionality

    bool enable_graphics_support;
        // whether to enable graphics support by way of
        // http://boinc.berkeley.edu/trac/wiki/GraphicsApi#File

    bool enable_screenshots_on_error;
        // capture screen shots during catastrophic events

    bool enable_vm_savestate_usage;
        // whether to use savestate instead of poweroff on exit

    bool disable_automatic_checkpoints;
        // whether to disable automatic checkpoint support

    double job_duration;
        // maximum amount of wall-clock time this VM is allowed to run before
        // considering itself done.

    std::string fraction_done_filename;
        // name of file where app will write its fraction done

    std::string heartbeat_filename;
        // name of the file to check for a heartbeat
        // (i.e. check mod time with stat)

    double minimum_heartbeat_interval;
        // check heartbeat interval

    int pf_guest_port;
    int pf_host_port;
        // if nonzero, do port forwarding for Web GUI

    std::vector<VBOX_PORT_FORWARD> port_forwards;

    double minimum_checkpoint_interval;
        // minimum time between checkpoints

    std::vector<std::string> copy_to_shared;
        // list of files to copy from slot dir to shared/

    bool copy_cmdline_to_shared;
        // copy the cmdline to shared/cmdline

    std::vector<std::string> trickle_trigger_files;
        // if find file of this name in shared/, send trickle-up message
        // with variety = filename, contents = file contents

    std::vector<VBOX_INTERMEDIATE_UPLOAD> intermediate_upload_files;
        // if find file of this name in shared/, send specified file

    std::string completion_trigger_file;
        // if find this file in shared/, task is over.
        // File can optionally contain exit code (first line)
        // File can optionally contain is_notice bool (second line)
        // and stderr text (subsequent lines).
        // Addresses a problem where VM doesn't shut down properly

    std::string temporary_exit_trigger_file;
        // if find this file in shared/, task is restarted at a later date.
        // File can optionally contain restart delay (first line)
        // File can optionally contain is_notice bool (second line)
        // and stderr text (subsequent lines).
        // Addresses a problem where VM doesn't shut down properly

    std::string multiattach_vdi_file;
        // Name of the vdi file (without path) to be attached in multiattach mode.
        // The file is expected to be in the project's base directory.
};

#endif
