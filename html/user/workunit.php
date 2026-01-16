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
require_once("../inc/keywords.inc");

check_get_args(array("wuid"));

function keyword_string($kwds) {
    global $job_keywords;

    $ks = explode(" ", $kwds);
    $first = true;
    $x = [];
    foreach ($ks as $k) {
        $x[] = $job_keywords[$k]->name;
    }
    return implode('<br>', $x);
}

function show_wu($wu) {
    page_head(tra("Workunit %1", $wu->id));
    $app = BoincApp::lookup_id($wu->appid);

    $x = "<in>".$wu->xml_doc."</in>";
    $xml_doc = simplexml_load_string($x);

    start_table();
    row2(tra("name"), $wu->name);
    row2(tra("application"), $app->user_friendly_name);
    if ($wu->batch) {
        row2('batch',
            "<a href=submit.php?action=query_batch&batch_id=$wu->batch>$wu->batch</a>"
        );
    }
    row2(tra("created"), time_str($wu->create_time));
    if (isset($wu->keywords) && $wu->keywords) {
        row2(tra("keywords"), keyword_string($wu->keywords));
    }
    if ($wu->canonical_resultid) {
        row2(tra("canonical result"),
            "<a href=result.php?resultid=$wu->canonical_resultid>$wu->canonical_resultid</a>"
        );
        row2(tra("granted credit"), format_credit($wu->canonical_credit));
    }

    // if app is using adaptive replication and no canonical result yet,
    // don't show anything more
    // (so that bad guys can't tell if they have an unreplicated job)

    $config = get_config();
    if ($app->target_nresults>0 && !$wu->canonical_resultid && !$wu->error_mask && !parse_bool($config, "dont_suppress_pending")) {
        row2(tra("Tasks in progress"), tra("suppressed pending completion"));
        end_table();
    } else {
        row2(tra("minimum quorum"), $wu->min_quorum);
        row2(tra("initial replication"), $wu->target_nresults);
        row2(tra("max # of error/total/success tasks"),
            "$wu->max_error_results, $wu->max_total_results, $wu->max_success_results"
        );
        if ($wu->error_mask) {
            row2(tra("errors"), wu_error_mask_str($wu->error_mask));
        }
        if ($wu->need_validate) {
            row2(tra("validation"), tra("Pending"));
        }
        row2('Command line', $xml_doc->workunit->command_line);
        if (function_exists('project_workunit')) {
            project_workunit($wu);
        }
        row2('Priority', $wu->priority);
        end_table();

        echo "<h2>Job instances</h2>\n";
        result_table_start(false, true, null);
        $results = BoincResult::enum("workunitid=$wu->id");
        foreach ($results as $result) {
            show_result_row($result, false, true, false);
        }
        echo "</table>\n";
    }

    // show input files
    //
    echo "<h2>Input files</h2>\n";
    start_table('table-striped');
    table_header("Name<br><small>(click to view)</small>", "Size (bytes)");
    foreach ($xml_doc->workunit->file_ref as $fr) {
        $pname = (string)$fr->file_name;
        $lname = (string)$fr->open_name;
        foreach ($xml_doc->file_info as $fi) {
            if ((string)$fi->name == $pname) {
                table_row(
                    "<a href=$fi->url>$lname</a>",
                    $fi->nbytes
                );
                break;
            }
        }
    }

    end_table();
    page_tail();
}

$wuid = get_int("wuid");
$wu = BoincWorkunit::lookup_id($wuid);
if (!$wu) {
    error_page(tra("can't find workunit"));
}
show_wu($wu);

?>
