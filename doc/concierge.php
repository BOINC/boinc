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

// This script gets (via POST)
//  - the URL of a project or AM
//  - optional credentials
// It
// - looks up the URL in a list of vetted entities
// - send cookies to the user's browser containing project/account info,
// - optionally starts a download of the appropriate client installer.

require_once("../inc/translation.inc");
require_once("versions.inc");
require_once("projects.inc");
require_once("concierge.inc");

function get_download_url($pname, $need_vbox) {
    global $platforms;
    global $url_base;

    $p = $platforms[$pname];
    $v = latest_version($p);
    $file = $v['file'];
    if ($need_vbox && array_key_exists('vbox_file', $v)) {
        $file = $v['vbox_file'];
    }
    $url = $url_base.$file;
    return $url;
}

// check if URL is in vetted list
//
$master_url = get_post("master_url");
$master_url = urldecode($master_url);
$vp = vps_lookup($master_url);
if (!$vp) {
    echo "URL not found";
    exit;
}

// could also check HTTP_REFERER here, but that can be forged

$auth = get_post("auth", true);
if ($auth) $auth = urldecode($auth);

$user_name = get_post("user_name", true);
if ($user_name) $user_name = urldecode($user_name);

$download = post_str('download', true);

if ($download) {
    $client_info = $_SERVER['HTTP_USER_AGENT'];
    $platform = client_infO_to_platform($client_info);
    $download_url = get_download_url($platform);
    if (!$download_url) {
        echo "no file to download";
        exit;
    }
}

setrawcookie('attach_master_url', rawurlencode($master_url), $expire);
setrawcookie('attach_project_name', rawurlencode($vp->name), $expire);
if ($auth && $user_name) {
    setrawcookie('attach_auth', rawurlencode($auth), $expire);
    setrawcookie('attach_user_name', rawurlencode($user_name), $expire);
}
setrawcookie('attach_project_desc', rawurlencode($vp->description), $expire);
setrawcookie('attach_project_inst', rawurlencode($vp->institution), $expire);

if ($download) {
Header("Location: ".$download_url);
}

?>
