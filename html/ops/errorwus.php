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

require_once("../inc/common_defs.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/cache.inc");

// User - configuarble variables
// seconds to cache this page
// this page runs a scan of the two largest tables, so this shouldn't be done more often than necessary
$cache_sec = 1800;
// Number that determines how many client errors are necessary for a WU to show up in this list.
// This number is added to min_quorum of the WU, so a value of 1 means that there must be more than
// (min_quorum + 1) errors for a WU to show up in this list.
$notification_level = 1;

$hide_canceled = get_str("hide_canceled", true);
$refresh_cache = get_int("refresh_cache", true);

admin_page_head("All-error Workunits");

function print_wu($row) {
    echo "<tr>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<input type=\"checkbox\" name=\"WU[]\" value=\"".$row['id']."\">\n";
    echo "<a href=db_action.php?table=workunit&detail=high&id=";
    echo $row['id'];
    echo ">";
    echo $row['id'];
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo $row['name'];
    echo "</td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo $row['quorum'];
    echo "</td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    if ($row['mask']) {
        echo wu_error_mask_str($row['mask']);
    } else {
        echo $row['mask'];
    }
    echo "</td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=3&sort_by=mod_time&detail=low&workunitid=";
    echo $row['id'];
    echo ">";
    echo $row['clerrors'];
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=6&sort_by=mod_time&detail=low&workunitid=";
    echo $row['id'];
    echo ">";
    echo $row['valerrors'];
    echo "</a></td>\n";
    echo "</tr>\n";
}

function get_error_wus() {
    global $notification_level;

    $db = BoincDb::get();
    $dbresult = $db->do_query("
        SELECT workunitid, outcome, workunit.name, min_quorum, error_mask
        FROM result INNER JOIN workunit ON workunit.id = workunitid
        WHERE server_state = 5
        ORDER BY workunitid, outcome DESC
    ;");

    $row_cache = array();
    $previd = -1;
    $prevname = "";
    $prevquorum = 1;
    $prevmask = 0;
    $valerrors = 0;
    $clerrors = 0;

    while ($res = $dbresult->fetch_object()) {
        $id = $res->workunitid;
        if ($id != $previd) {
            if (($clerrors  > $prevquorum + $notification_level) || ($valerrors > $prevquorum + $notification_level)) {
                $row_cache[] = array("id" => $previd, "name" => $prevname, "quorum" => $prevquorum, "clerrors" => $clerrors,
                                     "valerrors" => $valerrors, "mask" => $prevmask);
            }
            $prevmask = $res->error_mask;
            $previd = $id;
            $prevname = $res->name;
            $prevquorum = $res->min_quorum;
            $clerrors = 0;
            $valerrors = 0;
        }
        if ($res->outcome == 3) {
            $clerrors ++;
        }
        if ($res->outcome == 6) {
            $valerrors ++;
        }
        if ($res->outcome == 1) {
            $clerrors = 0;
            $valerrors = 0;
        }
    }
    $dbresult->free();

    if (($clerrors  > $prevquorum + $notification_level) || ($valerrors > $prevquorum + $notification_level)) {
        $row_cache[] = array("id" => $previd, "name" => $prevname, "quorum" => $prevquorum, "clerrors" => $clerrors,
                             "valerrors" => $valerrors, "mask" => $prevmask);
    }
    return $row_cache;
}

$last_update = 0;
$row_array = null;

$cache_data = get_cached_data($cache_sec);
if ($cache_data && !$refresh_cache) {
    $cache_data = unserialize($cache_data);
    $last_update = $cache_data['last_update'];
    $row_array = $cache_data['row_array'];
} else {
    $row_array = get_error_wus();
    $last_update = time();
    $cache_data = array('last_update' => $last_update, 'row_array' => $row_array);
    set_cached_data($cache_sec, serialize($cache_data));
}

echo "<br/>";
echo "<form method=\"get\" action=\"errorwus.php\">\n";
print_checkbox("Hide canceled WUs", "hide_canceled", $hide_canceled);
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"OK\">\n";
echo "</form>\n";
echo "Page last updated ".time_str($last_update);
echo "<form action=\"cancel_workunits_action.php\" method=\"post\">\n";
echo "<br/><table border=\"1\">\n";
echo "<input type=\"hidden\" name=\"back\" value=\"errorwus\"/>";
echo "<tr><th>WU ID</th><th>WU name</th><th>Quorum</th><th>Error mask</th><th>Client Errors</th><th>Validate Errors</th></tr>\n";

$hidden=0;
foreach($row_array as $row) {
    if ($hide_canceled == 'on' && (($row['mask'] & WU_ERROR_CANCELLED) == WU_ERROR_CANCELLED)) {
        $hidden++;
        continue;
    }
    print_wu($row);
}

echo "</table>\n<br>";
echo "<input type=\"hidden\" name=\"cancel\" value=\"1\"/>";
if ($hide_canceled == 'on') {
    echo "<input type=\"hidden\" name=\"hide_canceled\" value=\"on\"/>";
}
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"Cancel checked WUs\">";
echo "</form>\n";
echo count($row_array)." entries (".$hidden." hidden)\n";

admin_page_tail();

?>
