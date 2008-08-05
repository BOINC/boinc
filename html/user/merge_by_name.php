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

require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/boinc_db.inc");

db_init();

function merge_name($list) {
    // find the newest one
    //
    $newest_host = $list[0];
    echo "<br><br>Processing $newest_host->domain_name\n";
    foreach ($list as $host) {
        if ($host->create_time > $newest_host->create_time) {
            $newest_host = $host;
        }
    }
    foreach ($list as $host) {
        if ($host->id == $newest_host->id) {
            continue;
        }
        $error = merge_hosts($host, $newest_host);
        if (!$error) {
            echo "<br>Merged $host->id into $newest_host->id\n";
        } else {
            echo "<br>$error\n";
        }
    }
}

function merge_by_name($userid) {
    $hosts = array();
    $host_list = BoincHost::enum("userid=$userid");
    foreach($host_list as $host) {
        $hosts[$host->domain_name][] = $host;
    }
    foreach($hosts as $hlist) {
        merge_name($hlist);
    }
}

$user = get_logged_in_user();

page_head("Merge computers by name");

if ($_GET['confirmed']) {
    check_tokens($user->authenticator);
    merge_by_name($user->id);
    echo "
        <p><a href=hosts_user.php>
        Return to the list of your computers</a>.
    ";
} else {
    $tokens = url_tokens($user->authenticator);
    echo "
        This operation merges computers based on their domain name.
        <p>
        For each domain name, it will merge all older computers
        having that name with the newest computer having that name.
        Incompatible computers will not be merged.
        <p>
        <a href=merge_by_name.php?confirmed=1&$tokens>Go ahead and do this</a>.
        <p><a href=hosts_user.php>Return to the list of computers</a>.
    ";
}
page_tail();
?>
