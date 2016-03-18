<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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
$notification_level = get_int("level", true);
if (!$notification_level) {
    $notification_level = 1;
}
$appid_filter = "";
$appid_title = "";
$appid = get_int("appid", true);
if ($appid) {
    $appid_filter = " appid = $appid AND ";
    $app = BoincApp::lookup_id($appid);
    $appid_title = " for ".$app->name;
}

// the following variables are using the cache that is created by the variables above

// hide already canceled WUs
$hide_canceled = get_str("hide_canceled", true);
// hide WU that have only download errors
$hide_dlerr = get_str("hide_dlerr", true);
// refresh cache from DB
$refresh_cache = get_int("refresh_cache", true);

admin_page_head("All-error Workunits".$appid_title);

function print_wu($row) {
    echo "<tr>\n";

    echo "<td align=\"left\" valign=\"top\">";
    if (!in_rops()) {
        echo "<input type=\"checkbox\" name=\"WU[]\" value=\"".$row->id."\">\n";
    }
    echo "<a href=db_action.php?table=workunit&detail=high&id=";
    echo $row->id;
    echo ">";
    echo $row->id;
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">".$row->name."</td>\n";
    echo "<td align=\"left\" valign=\"top\">".$row->appid."</td>\n";
    echo "<td align=\"left\" valign=\"top\">".$row->min_quorum."</td>\n";
    echo "<td align=\"left\" valign=\"top\">".$row->unsent."</td>\n";
    echo "<td align=\"left\" valign=\"top\">".$row->in_progress."</td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=1&detail=low&workunitid=".$row->id.">";
    echo $row->successes;
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=3&client_state=1&detail=low&workunitid=".$row->id.">";
    echo $row->download_errors;
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=3&client_state=3&sort_by=mod_time&detail=low&workunitid=".$row->id.">";
    echo $row->compute_errors;
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    echo "<a href=db_action.php?table=result&query=&outcome=6&sort_by=mod_time&detail=low&workunitid=".$row->id.">";
    echo $row->validate_errors;
    echo "</a></td>\n";

    echo "<td align=\"left\" valign=\"top\">";
    if ($row->error_mask) {
        echo wu_error_mask_str($row->error_mask);
    } else {
        echo "&nbsp;";
    }
    echo "</td>\n";

    echo "</tr>\n";
}

function get_error_wus() {
    global $notification_level;
    global $appid_filter;

    // this query is obviously expensive for big projects but if there is a replica this does not impact the project
    $db = BoincDb::get(true);
    $dbresult = $db->do_query("
        SELECT id, name, appid, unsent, in_progress, successes, compute_errors,
               download_errors, validate_errors, error_mask, min_quorum,
               (compute_errors + download_errors + validate_errors) as total_errors
        FROM (
            SELECT
                workunitid,
                SUM(IF(outcome=1,1,0)) AS successes,
                SUM(IF((outcome=3 AND client_state=1),1,0)) AS download_errors,
                SUM(IF((outcome=3 AND client_state=3),1,0)) AS compute_errors,
                SUM(IF(outcome=6,1,0)) AS validate_errors,
                SUM(IF(server_state=2,1,0)) AS unsent,
                SUM(IF(server_state=4,1,0)) AS in_progress
            FROM result
            WHERE server_state IN (2,4,5)
            GROUP BY workunitid
            ) AS t1
            JOIN workunit ON workunit.id = workunitid
        WHERE canonical_resultid=0 AND $appid_filter
        GREATEST(download_errors, compute_errors, validate_errors) > min_quorum + $notification_level
        ORDER BY name
    ;");

    $row_cache = array();
    while ($row = $dbresult->fetch_object()) {
        $row_cache[] = $row;
    }
    $dbresult->free();

    return $row_cache;
}

$last_update = 0;
$row_array = null;

$cache_args = "level=$notification_level";
if ($appid) $cache_args .= "&appid=$appid";

$cache_data = get_cached_data($cache_sec, $cache_args);
if ($cache_data && !$refresh_cache) {
    $cache_data = unserialize($cache_data);
    $last_update = $cache_data['last_update'];
    $row_array = $cache_data['row_array'];
} else {
    $row_array = get_error_wus();
    $last_update = time();
    $cache_data = array('last_update' => $last_update, 'row_array' => $row_array);
    set_cached_data($cache_sec, serialize($cache_data), $cache_args);
}

echo "<br/>";
echo "<form method=\"get\" action=\"errorwus.php\">\n";
print_checkbox("Hide canceled WUs", "hide_canceled", $hide_canceled);
print_checkbox("Hide WUs with only d/l errors", "hide_dlerr", $hide_dlerr);
if ($appid) {
    echo "<input type=\"hidden\" name=\"appid\" value=\"$appid\"/>";
}
echo "<input type=\"hidden\" name=\"level\" value=\"$notification_level\"/>";
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"OK\">\n";
echo "</form>\n";
echo "Page last updated ".time_str($last_update);
if (!in_rops()) {
    echo "<form action=\"cancel_workunits_action.php\" method=\"post\">\n";
    echo "<input type=\"hidden\" name=\"back\" value=\"errorwus\"/>";
}
echo "<br/><table border=\"1\">\n";
echo "<tr><th>WU ID</th><th>WU name</th><th>App ID</th><th>Quorum</th><th>Unsent</th><th>In Progress</th><th>Success</th>";
echo "<th>Download Errors</th><th>Compute Errors</th><th>Validate Errors</th><th>Error mask</th></tr>\n";

$hidden=0;
foreach($row_array as $row) {
    if ($hide_canceled == 'on' && (($row->error_mask & WU_ERROR_CANCELLED) == WU_ERROR_CANCELLED)) {
        $hidden++;
        continue;
    }
    if ($hide_dlerr == 'on' && $row->download_errors > 0 && $row->compute_errors == 0 && $row->validate_errors == 0) {
        $hidden++;
        continue;
    }
    print_wu($row);
}

echo "</table>\n<br>";
if (!in_rops()) {
    echo "<input type=\"hidden\" name=\"cancel\" value=\"1\"/>";
    echo "<input type=\"hidden\" name=\"hide_canceled\" value=\"$hide_canceled\"/>";
    echo "<input type=\"hidden\" name=\"hide_dlerr\" value=\"$hide_dlerr\"/>";
    if ($appid) {
        echo "<input type=\"hidden\" name=\"appid\" value=\"$appid\"/>";
    }
    echo "<input class=\"btn btn-default\" type=\"submit\" value=\"Cancel checked WUs\">";
    echo "</form>\n";
}
echo count($row_array)." entries (".$hidden." hidden)\n";

admin_page_tail();

?>
