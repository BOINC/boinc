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
// There's a logged-in user.
//
// If no project ID, redirect to BOINC web site
// otherwise...
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
//      run html/ops/get_versions.php
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
    foreach ($v->version as $i=>$v) {
        if ((string)$v->dbplatform != $p) {
            continue;
        }
        if (strstr((string)$v->description, "Recommended")) {
            return $v;
        }
        if ($dev) {
            if (strstr((string)$v->description, "Development")) {
                return $v;
            }
        }
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

// We can't use auto-attach; direct them to the BOINC download page
//
function direct_to_boinc() {
    global $master_url;
    page_head(tra("Download BOINC"));
    text_start();
    echo sprintf(
        '<p>%s
        <p><p>
        %s
        <p>
        <a href=https://boinc.berkeley.edu/download.php>%s</a>
        ',
        tra("To download and install BOINC,
            click on the link below and follow the instructions.
        "),
        tra("When BOINC first runs it will ask you to select a project.
            Select %1 from the list,
            or enter this project's URL: %2",
            PROJECT,
            $master_url
        ),
        tra("Go the BOINC download page.")
    );
    text_end();
    page_tail();
}

function show_download_page($user, $dev) {
    global $config;
    $need_vbox = parse_bool($config, "need_vbox");
    $project_id = parse_config($config, "<project_id>");
    if (!$project_id) {
        direct_to_boinc();
        return;
    }
    $v = get_version($dev);

    // if we can't figure out the user's platform,
    // take them to the download page on the BOINC site
    //
    if (!$v) {
        direct_to_boinc();
        return;
    }

    page_head("Download software");

    $phrase = "";
    if ($need_vbox) {
        $dlv = tra("the current versions of BOINC and VirtualBox");
        $phrase = tra("these versions are");
        $dl = tra("BOINC and VirtualBox");
    } else {
        $dlv = tra("the current version of BOINC");
        $phrase = tra("this version is");
        $dl = "BOINC";
    }
    echo tra("To participate in %1, %2 must be installed on your computer.", PROJECT, $dlv);
    echo"
        <p>
    ";
    echo tra("If %1 already installed, %2click here%3; otherwise:",
        $phrase,
        "<a href=download.php?action=installed>",
        "</a>"
    );
    echo "
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
    echo "<p><p>";
    echo tra("When the download is finished, open the downloaded file to install %1.", $dl);
    echo "<p><p>";
    echo tra("All done? %1Click here to finish%2.", "<a href=welcome.php>", "</a>");
    page_tail();
}

// if user already has BOINC installed, tell them how to attach
//
function installed() {
    global $config;
    $am = parse_bool($config, "account_manager");
    if ($am) {
        page_head(tra("Use %1", PROJECT));
        echo sprintf("%s
            <ul>
            <li> %s
            <li> %s
            <li> %s
            <li> %s
            </ul>
            ",
            tra("To use %1 on this computer:", PROJECT),
            tra("In the BOINC manager, go to the Tools menu"),
            tra("Select Use Account Manager"),
            tra("Select %1 from the list", PROJECT),
            tra("Enter your %1 email address and password.", PROJECT)
        );
    } else {
        page_head(tra("Add %1", PROJECT));
        echo sprintf("%s
            <ul>
            <li> %s
            <li> %s
            <li> %s
            <li> %s
            </ul>
            ",
            tra("To add %1 on this computer:", PROJECT),
            tra("In the BOINC manager, go to the Tools menu"),
            tra("Select Add Project"),
            tra("Select %1 from the list", PROJECT),
            tra("Enter your %1 email address and password.", PROJECT)
        );
    }
    echo "<p><p>";
    echo sprintf('<a href=home.php class="btn btn-success">%s</a>
        ',
        tra('Continue to your home page')
    );
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
