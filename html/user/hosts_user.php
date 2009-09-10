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

// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/cache.inc");

function link_url($sort, $rev, $show_all) {
    global $userid;
    $x = $userid ? "&userid=$userid":"";
    return "hosts_user.php?sort=$sort&rev=$rev&show_all=$show_all$x";
}

function link_url_rev($actual_sort, $sort, $rev, $show_all) {
    if ($actual_sort == $sort) {
        $rev = 1 - $rev;
    }
    return link_url($sort, $rev, $show_all);
}

function more_or_less($sort, $rev, $show_all) {
    echo "<p>";
    if ($show_all) {
        $url = link_url($sort, $rev, 0);
        echo "Show: All computers | <a href=$url>Only computers active in past 30 days</a>";
    } else {
        $url = link_url($sort, $rev, 1);
        echo "Show: <a href=$url>All computers</a> | Only computers active in past 30 days";
    }
    echo "<p>";
}

function user_host_table_start($private, $sort, $rev, $show_all) {
    start_table();
    echo "<tr>";
    $url = link_url_rev($sort, "id", $rev, $show_all);
    echo "<th><a href=$url>Computer ID</a></th>\n";
    if ($private) {
        $url = link_url_rev($sort, "name", $rev, $show_all);
        echo "<th><a href=$url>Name</a></th>\n";
        $url = link_url_rev($sort, "venue", $rev, $show_all);
        echo "<th><a href=$url>Location</th>\n";
    } else {
        echo "<th>Rank</th>";
    }
    $url = link_url_rev($sort, "expavg_credit", $rev, $show_all);
    echo "<th><a href=$url>Avg. credit</a></th>\n";
    $url = link_url_rev($sort, "total_credit", $rev, $show_all);
    echo "<th><a href=$url>Total credit</a></th>\n";
    echo "<th>BOINC<br>version</th>\n";
    $url = link_url_rev($sort, "cpu", $rev, $show_all);
    echo "<th><a href=$url>CPU</a></th>\n";
    echo "<th>GPU</th>\n";
    $url = link_url_rev($sort, "os", $rev, $show_all);
    echo "<th><a href=$url>Operating System</a></th>\n";
    $url = link_url_rev($sort, "rpc_time", $rev, $show_all);
    echo "<th><a href=$url>Last contact</a></th>\n";
}

$show_all = get_int("show_all", true);
if ($show_all != 1) {
    $show_all = 0;
}

$rev = get_int("rev", true);
if ($rev != 1) {
    $rev = 0;
}

$sort = get_str("sort", true);
$desc = false;  // whether the sort order's default is decreasing
switch ($sort) {
case "total_credit": $sort_clause = "total_credit"; $desc = true; break;
case "expavg_credit": $sort_clause = "expavg_credit"; $desc = true; break;
case "name": $sort_clause = "domain_name"; break;
case "id": $sort_clause = "id"; break;
case "cpu": $sort_clause = "p_vendor"; break;
case "gpu": $sort_clause = "serialnum"; break;
case "os": $sort_clause = "os_name"; break;
case "venue": $sort_clause = "venue"; break;
default:
    // default value -- sort by RPC time
    $sort = "rpc_time";
    $sort_clause = "rpc_time"; 
    $desc = true;
}

if ($rev != $desc) {
    $sort_clause .= " desc";
}


$user = get_logged_in_user(false);
$userid = get_int("userid", true);

if ($user && $user->id == $userid) {
    $userid = 0;
}
if ($userid) {
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    $caching = true;

    // At this point, we know that $userid, $show_all and $sort all have
    // valid values.
    //
    $cache_args="userid=$userid&amp;show_all=$show_all&amp;sort=$sort&amp;rev=$rev";
    start_cache(USER_PAGE_TTL, $cache_args);
    if ($user->show_hosts) {
        page_head("Computers belonging to $user->name");
        more_or_less($sort, $rev, $show_all);
        user_host_table_start(false, $sort, $rev, $show_all);
    } else {
        page_head("Computers hidden");
        echo "This user has chosen not to show information about their computers.\n";
        page_tail();
        end_cache(USER_PAGE_TTL, $cache_args);
        exit();
    }
    $private = false;
} else {
    $user = get_logged_in_user();
    $caching = false;
    $userid = $user->id;
    page_head("Your computers");
    more_or_less($sort, $rev, $show_all);
    user_host_table_start(true, $sort, $rev, $show_all);
    $private = true;
}

$now = time();
$old_hosts=0;
$i = 1;
$hosts = BoincHost::enum("userid=$userid order by $sort_clause");
foreach ($hosts as $host) {
    $is_old=false;
    if (($now - $host->rpc_time) > 30*86400) {
        $is_old=true;
        $old_hosts++;
    }
    if (!$show_all && $is_old) continue;
    show_host_row($host, $i, $private, false);
    $i++;
}
echo "</table>\n";

if ($old_hosts>0) {
    more_or_less($sort, $rev, $show_all);
}

if ($private) {
    echo "
        <a href=merge_by_name.php>Merge computers by name</a>
    ";
}

if ($caching) {
    page_tail(true);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    page_tail();
}

?>
