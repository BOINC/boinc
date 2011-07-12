<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// show details of an app

require_once('../inc/util_ops.inc');

$appid = get_int("appid");

function remove_old($avs) {
    foreach($avs as $av) {
        foreach ($avs as $av2) {
            if ($av->id == $av2->id) continue;
            if ($av->platformid == $av2->platformid && $av->plan_class == $av2->plan_class && $av->version_num > $av2->version_num) {
                $av2->deprecated = 1;
            }
        }
    }
    $x = array();
    foreach($avs as $av) {
        if (!$av->deprecated) $x[] = $av;
    }
    return $x;
}

$app = BoincApp::lookup_id($appid);
if (!$app) admin_error_page("no such app");
admin_page_head("Details for $app->name ($app->user_friendly_name)");
start_table();
row2("Min average efficiency", $app->min_avg_pfc);
end_table();
echo "<h2>App versions</h2>\n";
$avs = BoincAppVersion::enum("appid=$appid");
$avs = remove_old($avs);
foreach ($avs as $av) {
    $platform = BoincPlatform::lookup_id($av->platformid);
    start_table();
    row2("ID", $av->id);
    row2("Platform", $platform->name);
    row2("Plan class", $av->plan_class);
    row2("Version num", $av->version_num);
    row2("Jobs validated", $av->pfc_n);
    row2("Average efficiency", $av->pfc_avg?1/$av->pfc_avg:"---");
    row2("Credit scale factor", $av->pfc_scale);
    row2("Average credit", $av->expavg_credit);
    end_table();
}
admin_page_tail();

?>
