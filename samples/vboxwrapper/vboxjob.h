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

#ifndef _VBOXJOB_H_
#define _VBOXJOB_H_


#define JOB_FILENAME "vbox_job.xml"


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

    // name of the OS the VM runs
    std::string os_name;

    // the type of disk controller to emulate
    std::string vm_disk_controller_type;

    // the disk controller model to emulate
    std::string vm_disk_controller_model;

    // size of the memory allocation for the VM, in megabytes
    double memory_size_mb;

    // whether to use CERN specific data structures
    bool enable_cern_dataformat;

    // whether to use an iso9660 image to implement VM contextualization (e.g. uCernVM)
    bool enable_isocontextualization;

    // whether to add an extra cache disk for systems like uCernVM
    bool enable_cache_disk;

    // whether to allow network access
    bool enable_network;

    // use bridged mode for network
    bool network_bridged_mode;

    // whether to use shared directory infrastructure
    bool enable_shared_directory;

    // whether to use floppy io infrastructure
    bool enable_floppyio;

    // whether to enable remote desktop functionality
    bool enable_remotedesktop;

    // whether to enable GBAC functionality
    bool enable_gbac;

    // whether to enable graphics support by way of
    // http://boinc.berkeley.edu/trac/wiki/GraphicsApi#File
    bool enable_graphics_support;

    // whether to use savestate instead of poweroff on exit
    bool enable_vm_savestate_usage;

    // whether to disable automatic checkpoint support
    bool disable_automatic_checkpoints;

    // maximum amount of wall-clock time this VM is allowed to run before
    // considering itself done.
    double job_duration;

    // name of file where app will write its fraction done
    std::string fraction_done_filename;

    // if nonzero, do port forwarding for Web GUI
    int pf_guest_port;      
    int pf_host_port;

    std::vector<VBOX_PORT_FORWARD> port_forwards;

    // minimum time between checkpoints
    double minimum_checkpoint_interval;

    // list of files to copy from slot dir to shared/
    std::vector<std::string> copy_to_shared;

    // if find file of this name in shared/, send trickle-up message
    // with variety = filename, contents = file contents
    std::vector<std::string> trickle_trigger_files;

    // if find file of this name in shared/, send specified file
    std::vector<VBOX_INTERMEDIATE_UPLOAD> intermediate_upload_files;

    // if find this file in shared/, task is over.
    // File can optionally contain exit code (first line)
    // File can optionally contain is_notice bool (second line)
    // and stderr text (subsequent lines).
    // Addresses a problem where VM doesn't shut down properly
    std::string completion_trigger_file;

    // if find this file in shared/, task is restarted at a later date.
    // File can optionally contain restart delay (first line)
    // File can optionally contain is_notice bool (second line)
    // and stderr text (subsequent lines).
    // Addresses a problem where VM doesn't shut down properly
    std::string temporary_exit_trigger_file;
};

#endif
