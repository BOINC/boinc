<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// projects can show a "Join" button that redirects here,
// passing various info about the project.
// We then send cookies to the user's browser,
// and download the appropriate client installer.
// When the manager eventually runs, it looks for the cookies
// and skips the choose-project page,
// and possibly the credentials page.

require_once("versions.inc");
require_once("projects.inc");

function get_str($name) {
    if (isset($_GET[$name])) {
        return $_GET[$name];
    }
    return null;
}

function get_download_url($pname) {
    global $platforms;
    global $url_base;

    $p = $platforms[$pname];
    $v = latest_version($p);
    $file = $v['file'];
    if (array_key_exists('vbox_file', $v)) {
        $file = $v['vbox_file'];
    }
    $url = $url_base.$file;
    return $url;
}

// based on the agent string, return name of file to download,
// or null if can't figure it out
//
function url_to_download() {
    $client_info = $_SERVER['HTTP_USER_AGENT'];
    if (strstr($client_info, 'Windows')) {
        if (strstr($client_info, 'Win64')||strstr($client_info, 'WOW64')) {
            return get_download_url('winx64');
        } else {
            return get_download_url('win');
        }
    } else if (strstr($client_info, 'Mac')) {
        if (strstr($client_info, 'PPC Mac OS X')) {
            return get_download_url('macppc');
        } else {
            return get_download_url('mac');
        }
    } else if (strstr($client_info, 'Linux') && strstr($client_info, 'Android')) {
        // Check for Android before Linux,
        // since Android contains the Linux kernel and the
        // web browser user agent string list Linux too.
        return get_download_url('androidarm');
    } else if (strstr($client_info, 'Linux')) {
        if (strstr($client_info, 'x86_64')) {
            return get_download_url('linuxx64');
        } else {
            return get_download_url('linux');
        }
    }
    return null;
}

$master_url = get_str("master_url");
$project_name = get_str("project_name");
$auth = get_str("auth");
$project_desc = get_str("project_desc");
$project_inst = get_str("project_inst");

if (!$master_url || !$project_name) {
    echo "missing arg";
    exit;
}

$url = url_to_download();
if (!$url) {
    echo "no file to download";
    exit;
}

// see if this project is in BOINC's list;
// if so, use the info there if the project didn't supply it
//
$p = lookup_project($master_url);
if ($p) {
    setcookie('attach_known', "1");
    if (!$project_inst) {
        $project_inst = $p[2];
    }
    if (!$project_desc) {
        $project_desc = $p[4];
    }
} else {
    setcookie('attach_known', "0");
}

$expire = time() + 24*86400;

setcookie('attach_master_url', $master_url, $expire);
setcookie('attach_project_name', $project_name, $expire);
setcookie('attach_auth', $auth, $expire);
setcookie('attach_project_desc', $project_desc, $expire);
setcookie('attach_project_inst', $project_inst, $expire);

Header("Location: ".$url);

?>
