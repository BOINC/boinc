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

// show summary of a workunit

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/result.inc");

BoincDb::get(true);

$wuid = get_int("wuid");
$wu = BoincWorkunit::lookup_id($wuid);
if (!$wu) {
    error_page("can't find workunit");
}

page_head("Workunit $wuid");
$app = BoincApp::lookup_id($wu->appid);

start_table();
row2("name", $wu->name);
row2("application", $app->user_friendly_name);
row2("created", time_str($wu->create_time));
if ($wu->canonical_resultid) {
    row2("canonical result",
        "<a href=result.php?resultid=$wu->canonical_resultid>$wu->canonical_resultid</a>"
    );
    row2("granted credit", format_credit($wu->canonical_credit));
}

// if app is using adaptive replication and no canonical result yet,
// don't show anything more
// (so that bad guys can't tell if they have an unreplicated job)

if ($app->target_nresults>0 && !$wu->canonical_resultid) {
    row2("Tasks in progress", "suppressed pending completion");
    end_table();
} else {
    row2("minimum quorum", $wu->min_quorum);
    row2("initial replication", $wu->target_nresults);
    row2("max # of error/total/success tasks",
        "$wu->max_error_results, $wu->max_total_results, $wu->max_success_results"
    );
    if ($wu->error_mask) {
        row2("errors", wu_error_mask_str($wu->error_mask));
    }
    if ($wu->need_validate) {
        row2("validation", "Pending");
    }
    end_table();
    project_workunit($wu);

    result_table_start(false, true, null);
    $results = BoincResult::enum("workunitid=$wuid");
    $i = 0;
    foreach ($results as $result) {
        show_result_row($result, false, true, false, $i++);
    }
    echo "</table>\n";
}

page_tail();

?>
