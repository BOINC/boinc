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

// Show a page with download links and instructions.
//
// - get platform from user agent string
// - find latest version for that platform (regular and vbox)
// - Create a login token.
// - Show download button(s)
//   The download will be via concierge, using the login token.
//
// By default both regular and vbox buttons will be shown, if available.
// You can suppress one or the other by setting
// <disable_regular_download> or <disable_vbox_download>.
//
// Notes:
// 1) You need to have the client versions file
//    (https://boinc.berkeley.edu/download_all.php?xml=1)
//    saved as "versions.xml" in your html/user dir
// 2) Put your project ID in a constant PROJECT_ID
//    (this all works only for listed projects)

require_once("../inc/util.inc");
require_once("../inc/account.inc");

// take the client info string reported by web browser,
// and return best guess for platform
//
function client_info_to_platform($client_info) {
    if (strstr($client_info, 'Windows')) {
        if (strstr($client_info, 'Win64')||strstr($client_info, 'WOW64')) {
            return 'windows_x86_64';
        } else {
            return 'windows_intelx86';
        }
    } else if (strstr($client_info, 'Mac')) {
        if (strstr($client_info, 'PPC Mac OS X')) {
            return 'powerpc-apple-darwin';
        } else {
            return 'x86_64-apple-darwin';
        }
    } else if (strstr($client_info, 'Android')) {
        // Check for Android before Linux,
        // since Android contains the Linux kernel and the
        // web browser user agent string lists Linux too.
        //
        return 'arm-android-linux-gnu';
    } else if (strstr($client_info, 'Linux')) {
        if (strstr($client_info, 'x86_64')) {
            return 'x86_64-pc-linux-gnu';
        } else {
            return 'i686-pc-linux-gnu';
        }
    } else {
        return null;
    }
}

// find release version for user's platform
//
function get_version($dev) {
    $v = simplexml_load_file("versions.xml");
    $client_info = $_SERVER['HTTP_USER_AGENT'];
    $p = client_info_to_platform($client_info);
    $string = $dev?"Development":"Recommended";
    foreach ($v->version as $i=>$v) {
        if ((string)$v->dbplatform != $p) {
            continue;
        }
        if (!strstr((string)$v->description, $string)) {
            continue;
        }
        return $v;
    }
    return null;
}

function download_button($v, $project_id, $token, $user) {
    return sprintf(
        '<form action="https://boinc.berkeley.edu/concierge.php" method="post">
        <input type=hidden name=project_id value="%d">
        <input type=hidden name=token value="%s">
        <input type=hidden name=user_id value="%d">
        <input type=hidden name=filename value="%s">
        <button class="btn btn-info">
        <font size=2><u>Download BOINC</u></font>
        <br>for %s (%s MB)
        <br><small>BOINC %s</small></button>
        </form>
        ',
        $project_id,
        $token,
        $user->id,
        (string)$v->filename,
        (string)$v->platform,
        (string)$v->size_mb,
        (string)$v->version_num
    );
}

function download_button_vbox($v, $project_id, $token, $user) {
    // if no vbox version exists for platform, don't show vbox button
    if(!$v->vbox_filename) {
        return;
    }
    return sprintf(
        '<form action="https://boinc.berkeley.edu/concierge.php" method="post">
        <input type=hidden name=project_id value="%d">
        <input type=hidden name=token value="%s">
        <input type=hidden name=user_id value="%d">
        <input type=hidden name=filename value="%s">
        <button class="btn btn-success">
        <font size=+1><u>Download BOINC + VirtualBox</u></font>
        <br>for %s (%s MB)
        <br><small>BOINC %s, VirtualBox %s</small></a>
        </form>
        ',
        $project_id,
        $token,
        $user->id,
        (string)$v->vbox_filename,
        (string)$v->platform,
        (string)$v->vbox_size_mb,
        (string)$v->version_num,
        (string)$v->vbox_version
    );
}

function show_download_page($user, $dev) {
    global $config;
    $need_vbox = parse_bool($config, "need_vbox");
    $project_id = parse_config($config, "<project_id>");
    if (!$project_id) {
        error_page("must specify project ID in config.xml");
    }
    $v = get_version($dev);

    // if we can't figure out the user's platform,
    // take them to the download_all page on the BOINC site
    //
    if (!$v) {
        Header("Location: https://boinc.berkeley.edu/download_all.php");
        exit;
    }

    page_head("Download software");

    $phrase = "this is";
    if ($need_vbox) {
        $dlv = "the current versions of BOINC and VirtualBox";
        $phrase = "these are";
        $dl = "BOINC and VirtualBox";
    } else {
        $dlv = "the current version of BOINC";
        $phrase = "this is";
        $dl = "BOINC";
    }
    echo "To participate in ".PROJECT.", $dlv must be installed on your computer.
        <p>
        If $phrase already installed, <a href=download.php?action=installed>click here</a>.  Otherwise
        <p>
    ";
    $token = make_login_token($user);
    echo "<center><table border=0 cellpadding=20>\n";
    if ($v->vbox_filename) {
        table_row(
            "",
            download_button_vbox($v, $project_id, $token, $user),
            "&nbsp;&nbsp;",
            download_button($v, $project_id, $token, $user),
            ""
        );
    } else {
        table_row("", download_button($v, $project_id, $token, $user), "");
    }
    echo "</table></center>\n";
    echo "<p><p>
        When the download is finished,
        open the downloaded file to install $dl.
    ";
    page_tail();
}

// if user already has BOINC installed, tell them how to attach
//
function installed() {
    global $config;
    $am = parse_bool($config, "account_manager");
    if ($am) {
        page_head("Use ".PROJECT);
        echo "To add ".PROJECT." on this computer:
        <ul>
        <li> In the BOINC manager, go to the Tools menu
        <li> Select Use Account Manager
        <li> Select ".PROJECT." from the menu
        <li> Enter your email address and ".PROJECT." password.
        </ul>
        ";
    } else {
        page_head("Add ".PROJECT);
        echo "To add ".PROJECT." on this computer:
        <ul>
        <li> In the BOINC manager, go to the Tools menu
        <li> Select Add Project
        <li> Select ".PROJECT." from the menu
        <li> Enter your email address and ".PROJECT." password.
        </ul>
        ";
    }
    page_tail();
}

$user = get_logged_in_user();
$action = get_str("action", true);
$dev = get_str("dev", true);
if ($action == "installed") {
    installed();
} else {
    show_download_page($user, $dev);
}

?>
