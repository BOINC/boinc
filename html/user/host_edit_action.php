<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");

function fail($msg) {
    echo "Error: $msg";
    page_tail();
    exit();
}

function get_host($hostid, $user) {
    $host = BoincHost::lookup_id($hostid);
    if (!$host || $host->userid != $user->id) {
        fail("We have no record of that computer");
    }
    return $host;
}

$user = get_logged_in_user();

page_head(tra("Merge computer records"));

$nhosts = get_int("nhosts");
$hostid = get_int("id_0");
$latest_host = get_host($hostid, $user);
for ($i=1; $i<$nhosts; $i++) {
    $var = "id_$i";
    $hostid = get_int($var, true);
    if (!$hostid) continue;
    $host = get_host($hostid, $user);
    if ($host->create_time > $latest_host->create_time) {
        $error = merge_hosts($latest_host, $host);
        if ($error) {
            echo "<br>$error\n";
            continue;
        }
        $latest_host = $host;
    } else {
        merge_hosts($host, $latest_host);
    }
    // reread latest_host from database since we just
    // updated its credits
    //
    $latest_host = BoincHost::lookup_id($latest_host->id);
}
echo "
    <p><a href=hosts_user.php>".tra("Return to list of your computers")."</a>
";
page_tail();

//Header("Location: show_host_detail.php?hostid=$latest_host->id");

?>
