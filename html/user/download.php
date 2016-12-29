<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// 2nd phase of the "join" process.
// The user has created an account.
// Now see if they need to download BOINC.

require_once("../inc/util.inc");

// return the URL of a script on the BOINC web site
// that will send the needed cookies
// and optionally download the appropriate installer,
//
function concierge_url($user, $download) {
    global $master_url;
    $project_name = parse_config(get_config(), "<long_name>");
    $project_desc = parse_config(get_config(), "<project_desc>");
    $project_inst = parse_config(get_config(), "<project_inst>");
    $user = get_logged_in_user(false);
    $x =  "http://boinc.berkeley.edu/concierge.php";
    $x .= "?master_url=".urlencode($master_url);
    $x .= "&project_name=".urlencode($project_name);
    if ($project_desc) {
        $x .= "&project_desc=".urlencode($project_desc);
    }
    if ($project_inst) {
        $x .= "&project_inst=".urlencode($project_inst);
    }
    if ($user) {
        $x .= "&auth=".urlencode($user->authenticator);
        $x .= "&user_name=".urlencode($user->name);
    }
    if ($download) {
        $x .= "&download=1";
        if (parse_bool(get_config(), "need_vbox")) {
            $x .= "&need_vbox=1";
        }
    }
    return $x;
}

function show_download_page() {
    page_head("Download required software");

    $config = get_config();
    $need_vbox = parse_bool($config, "need_vbox");
    $mcv = parse_config($config, "<min_core_client_version>");
    $dlv = "BOINC";
    $dl = "BOINC";
    if ($mcv) {
        $dlv .= " version " . version_string_maj_min_rel($mcv). " or later";
    }

    $verb = "this is";
    if ($need_vbox) {
        $dl .= " and VirtualBox";
        $dlv .= " and VirtualBox";
        $verb = "these are";
    }
    echo "To participate in ".PROJECT.", $dlv must be installed.
        <p>
        If $verb already installed, <a href=download.php?action=installed>click here</a>.  Otherwise
        <p>
    ";
    show_button("download.php?action=download", "Download $dl");

    echo "<p>
        When the download is finished,
        open the downloaded file to install BOINC.
    ";

    page_tail();
}

// User says needed software is installed.
// Send cookies and tell user to run manager
//
function show_installed_page($user) {
    $url = concierge_url($user, false);
    page_head("Add project");
    echo "
        <iframe width=0 height=0 frameborder=0 src=$url></iframe>
        To start running ".PROJECT." on this computer:
        <ul>
        <li> Open the BOINC Manager.
        <li> Select <b>Add Project</b>.
        <li> You should see a welcome message; click OK.
        </ul>
    ";
    page_tail();
}

function download($user) {
    $url = concierge_url($user, true);
    Header("Location: $url");
}

$user = get_logged_in_user();
$action = get_str("action", true);
if ($action == "installed") {
    show_installed_page($user);
    exit;
}
if ($action == "download") {
    download($user);
}

show_download_page();

?>
