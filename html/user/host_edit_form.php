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

check_get_args(array("hostid", "detail"));

$user = get_logged_in_user();

$hostid = get_int("hostid");
$host = BoincHost::lookup_id($hostid);
if (!$host || $host->userid != $user->id) {
    error_page("We have no record of that computer");
}

$detail = get_int('detail', true);

page_head(tra("Merge computers"));

$t = time_str($host->create_time);
echo tra("Sometimes BOINC assigns separate identities to the same computer by mistake. You can correct this by merging old identities with the newest one.")."
    <form name=host_list action=host_edit_action.php>
    <input type=hidden name=id_0 value=$hostid>
    <p>
";

$all_hosts = BoincHost::enum("userid=$user->id");

$nhosts = 1;
$hosts = array();
foreach ($all_hosts as $host2) {
    if ($host->id == $host2->id) continue;
    if (!hosts_compatible($host, $host2, $detail)) continue;
    $hosts[] = $host2;
    $nhosts++;
    if ($nhosts==500) break;
}
if ($nhosts == 1) {
    echo "<br>".tra("No hosts are eligible for merging with this one.");
    if (!$detail) {
        echo "<p><a href=host_edit_form.php?hostid=$hostid&detail=1>".tra("Show details")."</a>
        ";
    }
    page_tail();
    exit();
}
echo "
    <p>"
    .tra("Check the computers that are the same as %1 (created %2, computer ID %3):", $host->domain_name, $t, $host->id)."
    <p>
";
start_table();
row_heading_array(array("", tra("name"), tra("created"), tra("computer ID")));

$i = 1;
foreach ($hosts as $host2) {
    $t = time_str($host2->create_time);
    $x = $host2->domain_name;
    if ($x == "") {
        $x = "[".tra("no hostname")."]";
    }
    row_array(array(
        "<input type=checkbox name=id_$i value=$host2->id>",
        $x,
        "$t",
        "$host2->id"
    ));
    $i++;
}
end_table();
echo "
    <br>
    <script>
        function set_all() {
";
for ($i=1; $i<$nhosts; $i++) {
    echo "document.host_list.id_$i.checked=1;\n";
}
echo "
        }
        function clear_all() {
";
for ($i=1; $i<$nhosts; $i++) {
    echo "document.host_list.id_$i.checked=0;\n";
}
echo "
        }
    </script>
    <p><a href=javascript:set_all()>Select all</a>
    <p><a href=javascript:clear_all()>Unselect all</a>
    <input type=hidden name=nhosts value=$nhosts>
    <p><input class=\"btn btn-default\" type=submit value='".tra("Merge hosts")."'>
    </form>
";

if (!$detail) {
    echo "<p><a href=host_edit_form.php?hostid=$hostid&detail=1>".tra("Show details")."</a>
    ";
}

page_tail();

?>
