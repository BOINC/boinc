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

// concierge is a mechanism for downloading an installer
// pre-configured for a particular project or AM account
//
// This script gets (via POST)
//  - info about a project or AM
//  - optional account info
//  - the name of an installer file
// Action:
// - look up the URL in a list of vetted entities
// - start a download of the requested installer,
//   with a filename that encodes the entity and account info
//
// The installer looks at its own filename and creates init files accordingly

require_once("../inc/util.inc");
require_once("versions.inc");
require_once("projects.inc");
require_once("concierge.inc");

$master_url = urldecode(post_str("master_url"));

// check if URL is in vetted list
//
$vp = vps_lookup($master_url);
if (!$vp) {
    echo "URL not found";
    exit;
}

// could also check HTTP_REFERER here, but that can be forged

$auth = post_str("auth", true);
if ($auth) $auth = urldecode($auth);

$user_name = post_str("user_name", true);
if ($user_name) $user_name = urldecode($user_name);

$download_filename = post_str("download_filename");

$download_url = "https://boinc.berkeley.edu/dl/$filename";

// add info to filename.
// We should keep the filename as short as possible;
// long filenames are unexpected and might scare people off.
// So add only the basics:
// - entity URL
// - whether it's an AM
// - account user name and account token
//
// Would be nice to include other stuff (entity institution/description)
// to show on the welcome screen.
// Can get that by other means (e.g. get_config RPC)

$info = sprintf("pu=%s&am=%d",
    $master_url,
    $vp->is_am?1:0
);

if ($auth && $user_name) {
    $info .= sprintf("&ut=%s&un=%s",
        $auth,
        $user_name
    );
}

$filename .= '__'.base64_encode($info);
header("Location: ".$download_url);
header(sprintf('Content-Disposition: attachment; filename="%s"', $filename));

?>
